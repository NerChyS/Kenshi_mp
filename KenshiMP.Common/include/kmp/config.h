#pragma once
#include "constants.h"
#include <string>
#include <vector>
#include <cstdint>

namespace kmp {

struct ClientConfig {
    std::string playerName     = "Player";
    std::string lastServer     = "127.0.0.1";       // default: own PC (host)
    uint16_t    lastPort       = KMP_DEFAULT_PORT;
    bool        autoConnect    = true;
    float       overlayScale   = 1.0f;
    std::string masterServer   = "162.248.94.149";   // Master server address
    uint16_t    masterPort     = 27801;               // Master server port
    std::vector<std::string> favoriteServers = {"162.248.94.149:27800"};
    bool        useSyncOrchestrator = false; // New 7-stage sync pipeline (set true to test)

    bool Load(const std::string& path);
    bool Save(const std::string& path) const;

    static std::string GetDefaultPath();     // Shared path (Injector writes here)
    static std::string GetInstancePath();    // PID-specific path (Core saves here)
};

struct ServerConfig {
    std::string serverName   = "KenshiMP Server";
    uint16_t    port         = KMP_DEFAULT_PORT;
    int         maxPlayers   = KMP_MAX_PLAYERS;
    std::string password;
    std::string savePath     = "world.kmpsave";
    int         tickRate     = KMP_TICK_RATE;
    bool        pvpEnabled   = true;
    float       gameSpeed    = 1.0f;
    std::string masterServer = "162.248.94.149"; // Master server address
    uint16_t    masterPort   = 27801;            // Master server port
    bool        masterServerEnabled = false;     // register on the public master server list

    // World options (server.json "worldOptions"): host's Kenshi gameplay options that every
    // client applies when it connects. Only keys present in server.json are sent.
    bool        worldOptionsEnabled = true;
    uint32_t    worldOptionsMask    = 0;         // which of the fields below were set
    float       optDeathFrequency   = 1.0f;
    float       optDamageMult       = 1.0f;
    float       optBuildingSpeed    = 1.0f;
    float       optNestsMult        = 1.0f;
    float       optResearchSpeed    = 1.0f;
    float       optProductionSpeed  = 1.0f;
    float       optHungerTime       = 1.0f;
    bool        optEasyProspecting  = false;
    bool        optBanditsLootPlayer = true;
    bool        optAnimalsEat       = true;
    bool        optDifficultHealing = false;

    bool Load(const std::string& path);
    bool Save(const std::string& path) const;
};

} // namespace kmp
