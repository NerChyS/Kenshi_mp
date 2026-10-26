#include "world_options.h"
#include "game_types.h"
#include "../core.h"
#include "kmp/memory.h"
#include <spdlog/spdlog.h>
#include <Windows.h>
#include <cmath>
#include <cstdio>
#include <mutex>

namespace kmp::world_options {

// Kenshi's live GameplayOptions object (New Game → Advanced options), Steam 1.0.68:
// a global at RVA 0x2133578. Verified: its static initializer at RVA 0x15BC8F0 does
// "lea rcx,[rip+...] ; jmp GameplayOptions::reset" with this address, and
// MedicalSystem code calls getStarvationTimeInHours() (Steam 0x3E7620) on it.
// Layout from KenshiLib (GameplayOptions.h); reset() (Steam 0x3E75D0) sets the defaults:
//   +0x00 float deathFrequency = 1   +0x04 bool easyProspecting = 0
//   +0x08 float globalDamageMultiplier = 1   +0x0C float buildingSpeed = 1
//   +0x10 float numNestsMult = 1     +0x14 float researchSpeed = 1
//   +0x18 float productionSpeed = 1  +0x1C float hungerTime = 1
//   +0x20 bool banditsLootPlayer = 1 +0x21 bool animalsEat = 1  +0x22 bool difficultHealing = 0
// The game reads these fields when it needs them (damage, build/research/production
// ticks, starvation), so writing them at runtime takes effect immediately. They are also
// saved into the save game, so a joiner's save keeps the host's values.
static constexpr uintptr_t RVA_GAMEPLAY_OPTIONS = 0x2133578;

static std::mutex s_mutex;
static bool s_have = false;
static MsgWorldOptions s_opts{};

static bool SEH_Apply(uintptr_t g, const MsgWorldOptions& o, float before[7]) {
    __try {
        float* f = reinterpret_cast<float*>(g);
        // Sanity: the floats must look like multipliers
        static const int kFloatIdx[7] = {0, 2, 3, 4, 5, 6, 7};
        for (int k = 0; k < 7; k++) {
            float v = f[kFloatIdx[k]];
            if (!(v > 0.0f && v < 100.0f)) return false;
        }
        before[0] = f[2]; before[1] = f[3]; before[2] = f[5]; before[3] = f[6];
        before[4] = f[7]; before[5] = f[4]; before[6] = f[0];
        uint8_t* b = reinterpret_cast<uint8_t*>(g);
        if (o.presentMask & WOPT_DEATH_FREQUENCY)   f[0] = o.deathFrequency;
        if (o.presentMask & WOPT_EASY_PROSPECTING)  b[0x04] = o.easyProspecting ? 1 : 0;
        if (o.presentMask & WOPT_DAMAGE_MULT)       f[2] = o.damageMult;
        if (o.presentMask & WOPT_BUILDING_SPEED)    f[3] = o.buildingSpeed;
        if (o.presentMask & WOPT_NESTS_MULT)        f[4] = o.nestsMult;
        if (o.presentMask & WOPT_RESEARCH_SPEED)    f[5] = o.researchSpeed;
        if (o.presentMask & WOPT_PRODUCTION_SPEED)  f[6] = o.productionSpeed;
        if (o.presentMask & WOPT_HUNGER_TIME)       f[7] = o.hungerTime;
        if (o.presentMask & WOPT_BANDITS_LOOT)      b[0x20] = o.banditsLootPlayer ? 1 : 0;
        if (o.presentMask & WOPT_ANIMALS_EAT)       b[0x21] = o.animalsEat ? 1 : 0;
        if (o.presentMask & WOPT_DIFFICULT_HEALING) b[0x22] = o.difficultHealing ? 1 : 0;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static void ApplyNow() {
    MsgWorldOptions o;
    {
        std::lock_guard lk(s_mutex);
        if (!s_have) return;
        o = s_opts;
    }
    auto& core = Core::Get();
    if (!core.IsGameLoaded()) return;
    if (!game::GetKnownGameWorldObject()) {  // Steam 1.0.68 layout check
        spdlog::warn("world_options: unknown game build — server world options NOT applied");
        return;
    }
    float before[7] = {};
    if (SEH_Apply(Memory::GetModuleBase() + RVA_GAMEPLAY_OPTIONS, o, before)) {
        spdlog::info("world_options: applied host options (mask 0x{:X}); before: damage {} build {} research {} "
                     "production {} hunger {} nests {} death {}", o.presentMask,
                     before[0], before[1], before[2], before[3], before[4], before[5], before[6]);
        core.GetNativeHud().AddSystemMessage("Server world settings applied: " + SummaryLine(0));
    } else {
        spdlog::warn("world_options: GameplayOptions sanity check failed — not applied");
    }
}

void OnReceived(const MsgWorldOptions& msg) {
    {
        std::lock_guard lk(s_mutex);
        s_opts = msg;
        // Clamp (never trust the wire)
        auto clampf = [](float& v) { if (!(v >= 0.05f && v <= 20.0f)) v = 1.0f; };
        clampf(s_opts.deathFrequency); clampf(s_opts.damageMult); clampf(s_opts.buildingSpeed);
        clampf(s_opts.nestsMult); clampf(s_opts.researchSpeed); clampf(s_opts.productionSpeed);
        clampf(s_opts.hungerTime);
        s_have = true;
    }
    spdlog::info("world_options: received from server (mask 0x{:X})", msg.presentMask);
    ApplyNow();
}

void OnGameLoaded() { ApplyNow(); }

void OnDisconnected() {
    std::lock_guard lk(s_mutex);
    s_have = false;
}

bool HasOptions() {
    std::lock_guard lk(s_mutex);
    return s_have;
}

static std::string Num(float v) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%g", std::round(v * 100.0f) / 100.0f);
    return buf;
}

std::string SummaryLine(int line) {
    std::lock_guard lk(s_mutex);
    if (!s_have) return "";
    const auto& o = s_opts;
    std::string out;
    auto add = [&](uint32_t bit, const std::string& txt) {
        if (!(o.presentMask & bit)) return;
        if (!out.empty()) out += ", ";
        out += txt;
    };
    if (line == 0) {
        add(WOPT_DAMAGE_MULT,      "damage x" + Num(o.damageMult));
        add(WOPT_BUILDING_SPEED,   "build x" + Num(o.buildingSpeed));
        add(WOPT_RESEARCH_SPEED,   "research x" + Num(o.researchSpeed));
        add(WOPT_PRODUCTION_SPEED, "production x" + Num(o.productionSpeed));
    } else {
        add(WOPT_HUNGER_TIME,       "starvation time x" + Num(o.hungerTime));
        add(WOPT_NESTS_MULT,        "nests x" + Num(o.nestsMult));
        add(WOPT_BANDITS_LOOT,      std::string("bandits loot: ") + (o.banditsLootPlayer ? "yes" : "no"));
        add(WOPT_DIFFICULT_HEALING, std::string("hard healing: ") + (o.difficultHealing ? "yes" : "no"));
    }
    return out;
}

} // namespace kmp::world_options
