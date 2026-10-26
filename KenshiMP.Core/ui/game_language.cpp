#include "game_language.h"
#include <spdlog/spdlog.h>
#include <Windows.h>
#include <fstream>
#include <algorithm>
#include <cctype>

namespace kmp {

static std::string ReadLanguageFromCfg(const std::string& path) {
    std::ifstream f(path);
    if (!f) return "";
    std::string line;
    while (std::getline(f, line)) {
        // trim
        auto notSpace = [](unsigned char c) { return !std::isspace(c); };
        line.erase(line.begin(), std::find_if(line.begin(), line.end(), notSpace));
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        key.erase(std::find_if(key.rbegin(), key.rend(), notSpace).base(), key.end());
        std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return (char)std::tolower(c); });
        if (key != "language") continue;
        std::string val = line.substr(eq + 1);
        val.erase(val.begin(), std::find_if(val.begin(), val.end(), notSpace));
        val.erase(std::find_if(val.rbegin(), val.rend(), notSpace).base(), val.end());
        return val;
    }
    return "";
}

const std::string& GetGameLanguage() {
    static std::string s_lang;
    static bool s_done = false;
    if (s_done) return s_lang;
    s_done = true;

    std::string raw;
    char exePath[MAX_PATH] = {};
    if (GetModuleFileNameA(nullptr, exePath, MAX_PATH)) {
        std::string dir(exePath);
        size_t slash = dir.find_last_of("\\/");
        if (slash != std::string::npos) dir = dir.substr(0, slash);
        raw = ReadLanguageFromCfg(dir + "\\settings.cfg");
    }
    if (raw.empty()) raw = ReadLanguageFromCfg("settings.cfg");

    std::string code = raw.substr(0, std::min<size_t>(2, raw.size()));
    std::transform(code.begin(), code.end(), code.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    static const char* known[] = {"en", "es", "ru", "de", "fr", "it", "pl", "pt", "zh", "ja", "ko"};
    s_lang = "en";
    for (auto* k : known) if (code == k) s_lang = k;
    spdlog::info("GameLanguage: settings.cfg language='{}' -> using '{}'", raw, s_lang);
    return s_lang;
}

const char* GetMultiplayerButtonCaption() {
    const std::string& l = GetGameLanguage();
    if (l == "es") return "MULTIJUGADOR";
    if (l == "ru") return u8"МУЛЬТИПЛЕЕР";
    if (l == "de") return "MEHRSPIELER";
    if (l == "fr") return "MULTIJOUEUR";
    if (l == "it") return "MULTIGIOCATORE";
    if (l == "pl") return "WIELU GRACZY";
    if (l == "pt") return "MULTIJOGADOR";
    if (l == "zh") return u8"多人游戏";
    if (l == "ja") return u8"マルチプレイ";
    if (l == "ko") return u8"멀티플레이";
    return "MULTIPLAYER";
}

void GetHelpLines(int mode, bool serverLaunched, const char* out[6]) {
    for (int i = 0; i < 6; i++) out[i] = "";
    const std::string& l = GetGameLanguage();

    if (l == "es") {
        if (mode == 0) {
            out[0] = serverLaunched ? u8"¡Servidor iniciado! Deja su consola abierta." : u8"Cómo jugar:";
            out[1] = u8"1. Anfitrión: HOST GAME (inicia KenshiMP.Server.exe).";
            out[2] = u8"2. CLOSE > Nueva partida > 'Multiplayer' > Confirmar.";
            out[3] = u8"3. Espera 'Multiplayer ready' (como mucho ~90 s).";
            out[4] = u8"4. F1 > JOIN GAME > IP del anfitrión (él: 127.0.0.1) > CONNECT.";
            out[5] = u8"¿No conectan? Abre el puerto 27800 UDP o usa una VPN.";
        } else if (mode == 1) {
            out[0] = u8"Mundo cargado - listo para conectar:";
            out[1] = u8"1. Anfitrión: HOST GAME si el servidor no está abierto.";
            out[2] = u8"2. JOIN GAME > IP > CONNECT.";
            out[3] = u8"   Anfitrión: 127.0.0.1   Resto: IP del anfitrión";
            out[4] = u8"¿No conectan? Redirige 27800 UDP o usa Radmin VPN/";
            out[5] = u8"ZeroTier; ejecuta el servidor como admin una vez.";
        } else {
            out[0] = u8"Conectado.";
            out[1] = u8"Enter: chat    Tab: lista de jugadores";
            out[2] = u8"F1: abrir/cerrar este menú";
        }
        return;
    }
    if (l == "ru") {
        if (mode == 0) {
            out[0] = serverLaunched ? u8"Сервер запущен! Не закрывайте его консоль." : u8"Как играть:";
            out[1] = u8"1. Хост: HOST GAME (запускает KenshiMP.Server.exe).";
            out[2] = u8"2. CLOSE > Новая игра > 'Multiplayer' > Подтвердить.";
            out[3] = u8"3. Ждите 'Multiplayer ready' (не более ~90 с).";
            out[4] = u8"4. F1 > JOIN GAME > IP хоста (хост: 127.0.0.1) > CONNECT.";
            out[5] = u8"Не подключаются? Откройте порт 27800 UDP или VPN.";
        } else if (mode == 1) {
            out[0] = u8"Мир загружен - можно подключаться:";
            out[1] = u8"1. Хост: HOST GAME, если сервер не запущен.";
            out[2] = u8"2. JOIN GAME > IP > CONNECT.";
            out[3] = u8"   Хост: 127.0.0.1   Остальные: IP хоста";
            out[4] = u8"Не подключаются? Проброс 27800 UDP или Radmin VPN/";
            out[5] = u8"ZeroTier; один раз запустите сервер от админа.";
        } else {
            out[0] = u8"Подключено.";
            out[1] = u8"Enter: чат    Tab: список игроков";
            out[2] = u8"F1: открыть/закрыть меню";
        }
        return;
    }
    if (mode == 0) {
        out[0] = serverLaunched ? "Server started! Keep its console open. Next:" : "How to play:";
        out[1] = "1. Host: HOST GAME (starts KenshiMP.Server.exe).";
        out[2] = "2. CLOSE > New Game > 'Multiplayer' > Confirm.";
        out[3] = "3. Wait for 'Multiplayer ready' (at most ~90 s).";
        out[4] = "4. F1 > JOIN GAME > host IP (host: 127.0.0.1) > CONNECT.";
        out[5] = "Friends can't join? Open port 27800 UDP / use a VPN.";
    } else if (mode == 1) {
        out[0] = "World loaded - ready to connect:";
        out[1] = "1. Host: HOST GAME if the server isn't running.";
        out[2] = "2. JOIN GAME > IP > CONNECT.";
        out[3] = "   Host: 127.0.0.1   Others: the host's IP";
        out[4] = "Friends can't join? Forward 27800 UDP, or use";
        out[5] = "Radmin VPN/ZeroTier; run the server as admin once.";
    } else {
        out[0] = "Connected.";
        out[1] = "Enter: chat    Tab: player list";
        out[2] = "F1: open/close this menu";
    }
}

const char* GetJoinHintPreLoad() {
    const std::string& l = GetGameLanguage();
    if (l == "es") return u8"Mejor: conecta con F1 cuando el mundo ya esté cargado.\n"
                          u8"Si conectas aquí, después inicia Nueva partida > 'Multiplayer'.";
    if (l == "ru") return u8"Лучше подключаться по F1, когда мир уже загружен.\n"
                          u8"Если подключитесь здесь, затем: Новая игра > 'Multiplayer'.";
    return "Best: connect with F1 once the world has loaded.\n"
           "If you connect here, then start New Game > 'Multiplayer'.";
}

const char* GetJoinHintLoaded() {
    const std::string& l = GetGameLanguage();
    if (l == "es") return u8"Anfitrión: 127.0.0.1, resto: IP del anfitrión.\nPuerto 27800 (UDP).";
    if (l == "ru") return u8"Хост: 127.0.0.1, остальные: IP хоста.\nПорт 27800 (UDP).";
    return "Host uses 127.0.0.1, others the host's IP.\nPort 27800 (UDP).";
}

} // namespace kmp
