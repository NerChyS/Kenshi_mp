#include "appearance_sync.h"
#include "game_types.h"
#include "../core.h"
#include "kmp/protocol.h"
#include "kmp/memory.h"
#include <spdlog/spdlog.h>
#include <Windows.h>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <map>
#include <mutex>
#include <vector>

namespace kmp::appearance_sync {

// ── Steam 1.0.68 addresses (KenshiLib GOG RVA + region delta, verified by disassembly) ──
static constexpr uintptr_t RVA_GD_CTOR        = 0x66B590;  // GameData::GameData()
static constexpr uintptr_t RVA_GD_VTBL_COPY   = 0x1683098; // vtable the char editor puts on its temp GameData
static constexpr uintptr_t RVA_GD_DTOR        = 0xB5F10;   // GameData::~GameData() (non-deleting)
static constexpr uintptr_t RVA_GD_LOAD        = 0x6C3A70;  // bool GameData::loadFromFile(const string&, itemType)
static constexpr uintptr_t RVA_GD_SAVE        = 0x6C0930;  // bool GameData::saveToFile(const string&)
static constexpr uintptr_t RVA_GD_CLEAR       = 0x57B420;  // void GameData::clearEverything()
static constexpr uintptr_t RVA_GD_UPDATEFROM  = 0x2E6490;  // bool GameData::updateFrom(const GameData*, bool)
static constexpr uintptr_t RVA_APP_SETDATA    = 0x538360;  // void AppearanceBase::setAppearanceData(GameDataCopyStandalone*)
static constexpr uintptr_t RVA_APP_RELOAD     = 0x52D1F0;  // void AppearanceBase::reload()
static constexpr int       ITEMTYPE_APPEARANCE = 0x42;     // what the editor's "Import" passes

static constexpr uintptr_t CHAR_ANIMATION  = 0x448;  // Character::animation
static constexpr uintptr_t CHAR_MOVEMENT   = 0x640;  // Character::movement
static constexpr uintptr_t ANIM_MOVEMENT   = 0xC0;   // AnimationClass::movement
static constexpr uintptr_t ANIM_APPEARANCE = 0xE8;   // AnimationClass::appearance
static constexpr uintptr_t APP_DATA        = 0x148;  // AppearanceBase::appearanceData

using GdCtorFn   = void*(__fastcall*)(void* self);
using GdDtorFn   = void(__fastcall*)(void* self);
using GdLoadFn   = bool(__fastcall*)(void* self, const std::string* path, int type);
using GdSaveFn   = bool(__fastcall*)(void* self, const std::string* path);
using GdClearFn  = void(__fastcall*)(void* self);
using GdUpdFn    = bool(__fastcall*)(void* self, const void* from, bool mod);
using AppSetFn   = void(__fastcall*)(void* app, void* data);
using AppRelFn   = void(__fastcall*)(void* app);

static const char   kPrefix[] = "\x01KA";   // \x01 K A <idx u8> <count u8> <payload>
static constexpr size_t kPrefixLen = 3;
static constexpr size_t kChunk = 900;        // server accepts strings up to 1024 bytes

struct Incoming {
    std::vector<std::string> parts;
    int received = 0;
};

static std::mutex s_mutex;
static std::map<uint32_t, Incoming> s_incoming;          // per sender, being reassembled
static std::map<uint32_t, std::string> s_complete;       // per sender, full blob
static std::map<uint32_t, uintptr_t> s_appliedTo;        // per sender, character it was applied to
static std::atomic<bool> s_broadcastRequested{false};

static uintptr_t Base() { return Memory::GetModuleBase(); }

static bool ValidPtr(uintptr_t p) { return p >= 0x10000 && p < 0x00007FFFFFFFFFFF && (p & 7) == 0; }

static std::string TempPath(const char* name) {
    char dir[MAX_PATH] = {};
    DWORD n = GetTempPathA(MAX_PATH, dir);
    if (n == 0 || n >= MAX_PATH) strcpy_s(dir, ".\\");
    return std::string(dir) + name;
}

// Returns the AppearanceBase* of a character (0 if the layout doesn't match).
static uintptr_t SEH_GetAppearance(uintptr_t ch) {
    __try {
        if (!ValidPtr(ch)) return 0;
        uintptr_t anim = *reinterpret_cast<uintptr_t*>(ch + CHAR_ANIMATION);
        if (!ValidPtr(anim)) return 0;
        uintptr_t mv1 = *reinterpret_cast<uintptr_t*>(anim + ANIM_MOVEMENT);
        uintptr_t mv2 = *reinterpret_cast<uintptr_t*>(ch + CHAR_MOVEMENT);
        if (!ValidPtr(mv1) || mv1 != mv2) return 0;         // layout sanity check
        uintptr_t app = *reinterpret_cast<uintptr_t*>(anim + ANIM_APPEARANCE);
        if (!ValidPtr(app)) return 0;
        uintptr_t data = *reinterpret_cast<uintptr_t*>(app + APP_DATA);
        if (!ValidPtr(data)) return 0;
        return app;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}

static uintptr_t SEH_ReadPtr(uintptr_t a) {
    __try { return *reinterpret_cast<uintptr_t*>(a); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}

static bool SEH_Save(uintptr_t data, const std::string* path) {
    __try {
        return reinterpret_cast<GdSaveFn>(Base() + RVA_GD_SAVE)(reinterpret_cast<void*>(data), path);
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static bool ReadFileBytes(const std::string& path, std::string& out) {
    FILE* f = nullptr;
    if (fopen_s(&f, path.c_str(), "rb") != 0 || !f) return false;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0 || sz > 200000) { fclose(f); return false; }
    out.resize(static_cast<size_t>(sz));
    size_t rd = fread(out.data(), 1, out.size(), f);
    fclose(f);
    return rd == out.size();
}

static bool WriteFileBytes(const std::string& path, const std::string& bytes) {
    FILE* f = nullptr;
    if (fopen_s(&f, path.c_str(), "wb") != 0 || !f) return false;
    size_t wr = fwrite(bytes.data(), 1, bytes.size(), f);
    fclose(f);
    return wr == bytes.size();
}

// Loads `path` into a temporary GameData exactly like the character editor's
// "Import" does, then copies it over the character's appearance data and
// rebuilds the model.
static int SEH_Apply(uintptr_t app, const std::string* path, void* tmpBuf) {
    int stage = 0;
    __try {
        uintptr_t base = Base();
        uintptr_t data = *reinterpret_cast<uintptr_t*>(app + APP_DATA);
        stage = 1;
        reinterpret_cast<GdCtorFn>(base + RVA_GD_CTOR)(tmpBuf);
        *reinterpret_cast<uintptr_t*>(tmpBuf) = base + RVA_GD_VTBL_COPY;
        *(reinterpret_cast<uint8_t*>(tmpBuf) + 0x18) = 1;   // isStandalone
        stage = 2;
        bool ok = reinterpret_cast<GdLoadFn>(base + RVA_GD_LOAD)(tmpBuf, path, ITEMTYPE_APPEARANCE);
        if (ok) {
            stage = 3;
            reinterpret_cast<GdClearFn>(base + RVA_GD_CLEAR)(reinterpret_cast<void*>(data));
            stage = 4;
            reinterpret_cast<GdUpdFn>(base + RVA_GD_UPDATEFROM)(reinterpret_cast<void*>(data), tmpBuf, false);
        }
        stage = 5;
        reinterpret_cast<GdDtorFn>(base + RVA_GD_DTOR)(tmpBuf);
        if (!ok) return -2;
        stage = 6;
        reinterpret_cast<AppSetFn>(base + RVA_APP_SETDATA)(reinterpret_cast<void*>(app), reinterpret_cast<void*>(data));
        stage = 7;
        reinterpret_cast<AppRelFn>(base + RVA_APP_RELOAD)(reinterpret_cast<void*>(app));
        return 0;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return stage == 0 ? -100 : -stage - 10;
    }
}

void RequestBroadcast() { s_broadcastRequested.store(true); }

bool OnChatPayload(uint32_t senderId, const std::string& msg) {
    if (msg.size() < kPrefixLen + 2 || memcmp(msg.data(), kPrefix, kPrefixLen) != 0) return false;
    uint8_t idx = static_cast<uint8_t>(msg[kPrefixLen]);
    uint8_t cnt = static_cast<uint8_t>(msg[kPrefixLen + 1]);
    if (cnt == 0 || idx >= cnt) return true;
    std::lock_guard lk(s_mutex);
    auto& in = s_incoming[senderId];
    if (idx == 0 || in.parts.size() != cnt) { in.parts.assign(cnt, std::string()); in.received = 0; }
    if (in.parts[idx].empty()) in.received++;
    in.parts[idx] = msg.substr(kPrefixLen + 2);
    if (in.received == cnt) {
        std::string blob;
        for (auto& p : in.parts) blob += p;
        s_complete[senderId] = std::move(blob);
        s_appliedTo.erase(senderId);   // (re)apply on next tick
        s_incoming.erase(senderId);
        spdlog::info("appearance_sync: received appearance from player {} ({} bytes)", senderId,
                     s_complete[senderId].size());
    }
    return true;
}

void OnPlayerLeft(uint32_t playerId) {
    std::lock_guard lk(s_mutex);
    s_incoming.erase(playerId);
    s_complete.erase(playerId);
    s_appliedTo.erase(playerId);
}

void Reset() {
    std::lock_guard lk(s_mutex);
    s_incoming.clear();
    s_complete.clear();
    s_appliedTo.clear();
    s_broadcastRequested.store(false);
}

static std::string s_lastSentBytes;   // what we broadcast last (to detect later changes)

static void DoBroadcast() {
    uintptr_t arr = 0; int count = 0;
    if (!game::ReadKnownPlayerCharacters(arr, count) || count <= 0 || !ValidPtr(arr)) return;
    uintptr_t ch = SEH_ReadPtr(arr);
    uintptr_t app = SEH_GetAppearance(ch);
    if (!app) { spdlog::warn("appearance_sync: local appearance not found"); return; }
    uintptr_t data = SEH_ReadPtr(app + APP_DATA);
    std::string path = TempPath("kmp_appearance_local.dat");
    if (!SEH_Save(data, &path)) { spdlog::warn("appearance_sync: saveToFile failed"); return; }
    std::string bytes;
    if (!ReadFileBytes(path, bytes)) { spdlog::warn("appearance_sync: could not read exported appearance"); return; }

    size_t chunks = (bytes.size() + kChunk - 1) / kChunk;
    if (chunks == 0 || chunks > 250) { spdlog::warn("appearance_sync: appearance too big ({})", bytes.size()); return; }
    auto& core = Core::Get();
    for (size_t i = 0; i < chunks; i++) {
        std::string msg(kPrefix, kPrefixLen);
        msg.push_back(static_cast<char>(i));
        msg.push_back(static_cast<char>(chunks));
        msg.append(bytes, i * kChunk, kChunk);
        PacketWriter w;
        w.WriteHeader(MessageType::C2S_ChatMessage);
        w.WriteU32(core.GetLocalPlayerId());
        w.WriteString(msg);
        core.GetClient().SendReliable(w.Data(), w.Size());
    }
    s_lastSentBytes = bytes;
    spdlog::info("appearance_sync: broadcast local appearance ({} bytes, {} chunks)", bytes.size(), chunks);
}

static uintptr_t FindRemoteCharacter(uint32_t playerId) {
    auto& reg = Core::Get().GetEntityRegistry();
    for (EntityID id : reg.GetPlayerEntities(playerId)) {
        uintptr_t obj = reinterpret_cast<uintptr_t>(reg.GetGameObject(id));
        if (ValidPtr(obj)) return obj;
    }
    return 0;
}

void Tick() {
    auto& core = Core::Get();
    if (!core.IsConnected() || !core.IsGameLoaded()) return;

    if (s_broadcastRequested.exchange(false)) DoBroadcast();

    // Re-broadcast when the local look changed since the last broadcast (e.g. the player
    // connected while still in the character editor, or changed appearance later).
    {
        static auto s_lastCheck = std::chrono::steady_clock::now();
        auto t = std::chrono::steady_clock::now();
        static int s_changeBroadcasts = 0;   // cap, in case the export isn't byte-stable
        if (!s_lastSentBytes.empty() && s_changeBroadcasts < 5 &&
            t - s_lastCheck > std::chrono::seconds(20)) {
            s_lastCheck = t;
            uintptr_t arr = 0; int count = 0;
            if (game::ReadKnownPlayerCharacters(arr, count) && count > 0 && ValidPtr(arr)) {
                uintptr_t app = SEH_GetAppearance(SEH_ReadPtr(arr));
                if (app) {
                    uintptr_t data = SEH_ReadPtr(app + APP_DATA);
                    std::string path = TempPath("kmp_appearance_check.dat");
                    std::string bytes;
                    if (SEH_Save(data, &path) && ReadFileBytes(path, bytes) && !bytes.empty() &&
                        bytes != s_lastSentBytes) {
                        s_changeBroadcasts++;
                        spdlog::info("appearance_sync: local appearance changed — re-broadcasting");
                        DoBroadcast();
                    }
                }
            }
        }
    }

    // Apply at most one appearance per tick, throttled.
    static auto s_lastTry = std::chrono::steady_clock::time_point{};
    auto now = std::chrono::steady_clock::now();
    if (now - s_lastTry < std::chrono::milliseconds(500)) return;
    s_lastTry = now;

    uint32_t pid = 0; std::string blob;
    {
        std::lock_guard lk(s_mutex);
        for (auto& [id, b] : s_complete) {
            uintptr_t ch = FindRemoteCharacter(id);
            if (!ch) continue;
            auto it = s_appliedTo.find(id);
            if (it != s_appliedTo.end() && it->second == ch) continue;
            pid = id; blob = b;
            s_appliedTo[id] = ch;   // mark first: never retry a crashing apply in a loop
            break;
        }
    }
    if (!pid) return;

    uintptr_t ch = FindRemoteCharacter(pid);
    uintptr_t app = SEH_GetAppearance(ch);
    if (!app) { spdlog::warn("appearance_sync: remote character of player {} has no appearance", pid); return; }
    std::string path = TempPath(("kmp_appearance_" + std::to_string(pid) + ".dat").c_str());
    if (!WriteFileBytes(path, blob)) { spdlog::warn("appearance_sync: could not write temp file"); return; }

    alignas(16) static uint8_t tmp[0x1000];
    memset(tmp, 0, sizeof(tmp));
    int rc = SEH_Apply(app, &path, tmp);
    if (rc == 0) {
        spdlog::info("appearance_sync: applied appearance of player {} to char 0x{:X}", pid, ch);
        core.GetNativeHud().AddSystemMessage("Remote player appearance synced");
    } else {
        spdlog::error("appearance_sync: apply failed for player {} (code {})", pid, rc);
    }
}

} // namespace kmp::appearance_sync
