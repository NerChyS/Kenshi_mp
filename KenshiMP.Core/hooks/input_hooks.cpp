#include "input_hooks.h"
#include "../core.h"
#include "kmp/hook_manager.h"
#include "kmp/memory.h"
#include <spdlog/spdlog.h>
#include <Windows.h>
#include <cstring>
#include <chrono>
#include <deque>

namespace kmp::input_hooks {

// Input for our own UI comes through the WndProc hook in render_hooks.cpp.
// Kenshi itself reads the keyboard through OIS/DirectInput, so swallowing a
// WM_KEYDOWN does NOT stop the game from seeing the key. This hook sits on
// InputHandler::keyDownEvent(OIS::KeyCode) and drops:
//   - F1 (our multiplayer menu key — Kenshi would open its Tutorials window), and
//   - every key while one of our text fields (chat / menu edit box) has focus,
//     so typing doesn't trigger game hotkeys.
//
// Steam 1.0.68: InputHandler::keyDownEvent = RVA 0x360B30
// (KenshiLib GOG RVA 0x360680 + 0x4B0; verified: it special-cases KC_LSHIFT 0x2A / KC_RSHIFT 0x36).

static constexpr uintptr_t RVA_KEYDOWN_EVENT = 0x360B30;
static const uint8_t kKeyDownPrologue[] = {
    0x40, 0x53, 0x48, 0x83, 0xEC, 0x30, 0x44, 0x8B, 0xC2, 0x48, 0x8B, 0xD9, 0x83, 0xFA, 0x2A
};
static constexpr int KC_F1 = 0x3B;

using KeyDownFn = void(__fastcall*)(void* self, int key);
static KeyDownFn s_origKeyDown = nullptr;
static bool s_installed = false;

// Translate an OIS/DirectInput scan code into a printable character (0 if none).
static wchar_t ScanCodeToChar(int sc) {
    UINT vk = MapVirtualKeyW(static_cast<UINT>(sc), MAPVK_VSC_TO_VK_EX);
    if (!vk) return 0;
    BYTE state[256] = {};
    if (GetKeyState(VK_SHIFT) & 0x8000)   { state[VK_SHIFT] = 0x80; state[VK_LSHIFT] = 0x80; }
    if (GetKeyState(VK_CAPITAL) & 0x0001) state[VK_CAPITAL] = 0x01;
    if ((GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_MENU) & 0x8000)) {  // AltGr
        state[VK_CONTROL] = 0x80; state[VK_MENU] = 0x80;
    }
    WCHAR buf[4] = {};
    // flags 0x4: do not change the keyboard (dead-key) state
    int n = ToUnicodeEx(vk, static_cast<UINT>(sc), state, buf, 4, 0x4, GetKeyboardLayout(0));
    return (n == 1) ? buf[0] : 0;
}

// ── Character de-duplication between the two input paths ──
// A key press can reach us twice: via OIS (this hook) and via WM_CHAR (WndProc).
// Either path alone may miss keys, so each character is typed by whichever path
// sees it first, and the matching event from the other path is dropped.
struct PendingChar { wchar_t ch; std::chrono::steady_clock::time_point t; };
static std::deque<PendingChar> s_fromHook, s_fromWnd;

static void TypeChar(wchar_t ch) {
    auto& core = Core::Get();
    if (core.GetNativeHud().IsChatInputActive()) core.GetNativeHud().OnChatChar(ch);
    else core.GetOverlay().GetNativeMenu().OnChar(ch);
}

static void Deliver(wchar_t ch, std::deque<PendingChar>& mine, std::deque<PendingChar>& other) {
    auto now = std::chrono::steady_clock::now();
    auto expired = [&](const PendingChar& p) { return now - p.t > std::chrono::milliseconds(400); };
    while (!other.empty() && expired(other.front())) other.pop_front();
    while (!mine.empty() && expired(mine.front())) mine.pop_front();
    for (auto it = other.begin(); it != other.end(); ++it) {
        if (it->ch == ch) { other.erase(it); return; }   // already typed by the other path
    }
    mine.push_back({ch, now});
    if (mine.size() > 64) mine.pop_front();
    TypeChar(ch);
}

void OnWndChar(wchar_t ch) {
    if (ch < 32 || ch >= 127) return;
    Deliver(ch, s_fromWnd, s_fromHook);
}

static void __fastcall Hook_KeyDownEvent(void* self, int key) {
    if (key == KC_F1) return;  // F1 belongs to Kenshi Online
    auto& core = Core::Get();
    bool chat = core.GetNativeHud().IsChatInputActive();
    auto& menu = core.GetOverlay().GetNativeMenu();
    if (chat || menu.HasActiveEditBox()) {
        // Typing into our UI: the game must not see the key. Printable characters are
        // taken from here (OIS sees every key press; WM_CHAR can lose fast keystrokes).
        wchar_t ch = ScanCodeToChar(key);
        if (ch >= 32 && ch < 127) Deliver(ch, s_fromHook, s_fromWnd);
        return;
    }
    s_origKeyDown(self, key);
}



static bool SEH_PrologueMatches(uintptr_t addr) {
    __try {
        return memcmp(reinterpret_cast<const void*>(addr), kKeyDownPrologue, sizeof(kKeyDownPrologue)) == 0;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool Install() {
    if (s_installed) return true;
    uintptr_t base = Memory::GetModuleBase();
    if (!base) return false;
    uintptr_t addr = base + RVA_KEYDOWN_EVENT;
    if (!SEH_PrologueMatches(addr)) {
        spdlog::warn("input_hooks: keyDownEvent prologue mismatch — not Steam 1.0.68? F1 not swallowed");
        return false;
    }
    if (!HookManager::Get().InstallAt("InputKeyDown", addr, &Hook_KeyDownEvent, &s_origKeyDown)) {
        spdlog::warn("input_hooks: failed to hook InputHandler::keyDownEvent");
        return false;
    }
    s_installed = true;
    spdlog::info("input_hooks: InputHandler::keyDownEvent hooked (F1 + typing swallowed)");
    return true;
}

void Uninstall() {
    if (!s_installed) return;
    HookManager::Get().Remove("InputKeyDown");
    s_installed = false;
}

} // namespace kmp::input_hooks
