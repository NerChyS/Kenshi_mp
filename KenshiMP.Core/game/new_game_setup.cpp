#include "new_game_setup.h"
#include "game_types.h"
#include "../core.h"
#include "kmp/memory.h"
#include <spdlog/spdlog.h>
#include <Windows.h>
#include <chrono>
#include <cstring>
#include <set>

namespace kmp::new_game_setup {

// ── Steam 1.0.68 layout (KenshiLib GOG headers + Steam RVAs verified by disassembly) ──
// ActivePlatoon::setName(const std::string&)  GOG 0x4BDD00 → Steam 0x4BE480:
//     mov rcx,[rcx+78h]; or r9,-1; xor r8d,r8d; add rcx,18h; jmp std::string::assign
//     i.e. this->me->displayName.assign(name, 0, npos)
// ActivePlatoon::getName() const              GOG 0x411980 → Steam 0x412100:
//     mov rax,[rcx+78h]; add rax,18h; ret
static constexpr uintptr_t RVA_ACTIVEPLATOON_SETNAME = 0x4BE480;
static const uint8_t kSetNameBytes[] = {
    0x48, 0x8B, 0x49, 0x78, 0x49, 0x83, 0xC9, 0xFF, 0x45, 0x33, 0xC0, 0x48, 0x83, 0xC1, 0x18, 0xE9
};
static constexpr uintptr_t CHAR_PLATOON        = 0x658;  // Character::platoon (ActivePlatoon*)
static constexpr uintptr_t AP_ME               = 0x78;   // ActivePlatoon::me (Platoon*)
static constexpr uintptr_t PLATOON_DISPLAYNAME = 0x18;   // RootObjectBase::displayName (std::string)
// Character::medical (MedicalSystem, embedded at +0x458) → MedicalSystem::hunger at +0x60.
// Max hunger is 300 (Character::eatItem computes 300.0 - hunger). Character::getMedical
// (Steam 0xCD380) is "lea rax,[rcx+458h]; ret".
static constexpr uintptr_t CHAR_HUNGER         = 0x458 + 0x60;
static constexpr float     HUNGER_FULL         = 300.0f;

using SetNameFn = void(__fastcall*)(void* activePlatoon, const std::string* name);

static MenuButton s_lastButton = MenuButton::None;
static bool s_freshGame = false;
static bool s_loaded = false;
static std::chrono::steady_clock::time_point s_loadedAt{};
static std::set<uintptr_t> s_renamed;
static std::set<uintptr_t> s_fed;

static bool ValidPtr(uintptr_t p) { return p >= 0x10000 && p < 0x00007FFFFFFFFFFF && (p & 7) == 0; }

void OnMainMenuButtonClicked(MenuButton b) {
    s_lastButton = b;
    spdlog::info("new_game_setup: main-menu button {}", static_cast<int>(b));
}

void OnLoadStarted(bool fromMainMenu) {
    s_freshGame = fromMainMenu && s_lastButton == MenuButton::NewGame;
    s_loaded = false;
    s_renamed.clear();
    s_fed.clear();
    spdlog::info("new_game_setup: load started (fromMainMenu={}, freshGame={})", fromMainMenu, s_freshGame);
}

void OnGameLoaded() {
    s_loaded = true;
    s_loadedAt = std::chrono::steady_clock::now();
}

static bool SEH_ReadPtr(uintptr_t addr, uintptr_t& out) {
    __try { out = *reinterpret_cast<const uintptr_t*>(addr); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

// Copies a game std::string (MSVC x64 layout) into buf. Returns false on anything odd.
static bool SEH_ReadGameString(uintptr_t str, char* buf, size_t bufSize) {
    __try {
        size_t size = *reinterpret_cast<const size_t*>(str + 16);
        size_t cap  = *reinterpret_cast<const size_t*>(str + 24);
        if (size >= bufSize || size > cap || cap > 0x10000) return false;
        const char* chars = (cap >= 16) ? *reinterpret_cast<const char* const*>(str)
                                        : reinterpret_cast<const char*>(str);
        if (!ValidPtr(reinterpret_cast<uintptr_t>(chars)) && cap >= 16) return false;
        memcpy(buf, chars, size);
        buf[size] = '\0';
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static bool SEH_SetName(uintptr_t fn, uintptr_t platoon, const std::string* name) {
    __try { reinterpret_cast<SetNameFn>(fn)(reinterpret_cast<void*>(platoon), name); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static bool SEH_FillHunger(uintptr_t ch, float& before) {
    __try {
        float* h = reinterpret_cast<float*>(ch + CHAR_HUNGER);
        before = *h;
        if (!(before >= 0.0f && before <= HUNGER_FULL + 1.0f)) return false;  // layout sanity
        if (before < HUNGER_FULL) *h = HUNGER_FULL;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static bool SEH_BytesMatch(uintptr_t addr) {
    __try { return memcmp(reinterpret_cast<const void*>(addr), kSetNameBytes, sizeof(kSetNameBytes)) == 0; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

void Tick() {
    if (!s_loaded) return;
    auto& core = Core::Get();
    if (!core.IsGameLoaded()) return;

    auto now = std::chrono::steady_clock::now();
    static auto s_last = std::chrono::steady_clock::time_point{};
    if (now - s_last < std::chrono::seconds(3)) return;
    s_last = now;
    auto sinceLoad = now - s_loadedAt;
    // Rename during the first 10 minutes (covers the character editor), fill hunger
    // during the first 5 minutes of a freshly started game only.
    if (sinceLoad > std::chrono::minutes(10)) return;
    bool doHunger = s_freshGame && sinceLoad < std::chrono::minutes(5);

    // Steam 1.0.68 layout check (GameWorld vtable) — never touch memory on other builds
    if (!game::GetKnownGameWorldObject()) return;
    uintptr_t base = Memory::GetModuleBase();
    static int s_setNameOk = -1;
    if (s_setNameOk < 0) {
        s_setNameOk = SEH_BytesMatch(base + RVA_ACTIVEPLATOON_SETNAME) ? 1 : 0;
        if (!s_setNameOk) spdlog::warn("new_game_setup: ActivePlatoon::setName bytes mismatch — squad rename disabled");
    }

    uintptr_t arr = 0; int count = 0;
    if (!game::ReadKnownPlayerCharacters(arr, count) || count <= 0 || count > 256) return;

    auto& registry = core.GetEntityRegistry();
    std::string wanted = core.GetConfig().playerName;
    if (wanted.empty()) wanted = "Player";
    if (wanted.size() > 31) wanted.resize(31);

    for (int i = 0; i < count; i++) {
        uintptr_t ch = 0;
        if (!SEH_ReadPtr(arr + i * sizeof(uintptr_t), ch) || !ValidPtr(ch)) continue;
        // Skip characters that belong to remote players
        EntityID eid = registry.GetNetId(reinterpret_cast<void*>(ch));
        if (eid != INVALID_ENTITY && registry.IsRemote(eid)) continue;

        if (doHunger && s_fed.insert(ch).second) {
            float before = 0.f;
            if (SEH_FillHunger(ch, before) && before < HUNGER_FULL) {
                spdlog::info("new_game_setup: new game — hunger {:.0f} -> {:.0f} for char 0x{:X}",
                             before, HUNGER_FULL, ch);
            }
        }

        if (s_setNameOk != 1) continue;
        uintptr_t ap = 0, me = 0;
        if (!SEH_ReadPtr(ch + CHAR_PLATOON, ap) || !ValidPtr(ap)) continue;
        if (s_renamed.count(ap)) continue;
        if (!SEH_ReadPtr(ap + AP_ME, me) || !ValidPtr(me)) continue;
        char name[128];
        if (!SEH_ReadGameString(me + PLATOON_DISPLAYNAME, name, sizeof(name))) continue;
        s_renamed.insert(ap);
        // Kenshi's default name for a new player squad is "Nameless" (+ "_N"); the UI
        // translates it (e.g. "SIN NOMBRE_0"). Only rename those — never a name the player chose.
        if (strncmp(name, "Nameless", 8) != 0) continue;
        if (SEH_SetName(base + RVA_ACTIVEPLATOON_SETNAME, ap, &wanted)) {
            spdlog::info("new_game_setup: squad '{}' renamed to '{}'", name, wanted);
            core.GetNativeHud().AddSystemMessage("Squad renamed to '" + wanted + "'");
        }
    }
}

} // namespace kmp::new_game_setup
