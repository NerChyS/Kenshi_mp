#pragma once
#include <string>

namespace kmp {

// Kenshi's UI language, read once from <Kenshi>/settings.cfg ("language=es_ES").
// Returns a lowercase 2-letter code ("en", "es", "ru", ...); "en" when unknown.
const std::string& GetGameLanguage();

// Localized caption for the main-menu MULTIPLAYER button (UTF-8).
const char* GetMultiplayerButtonCaption();

// Localized help lines for the F1 / MULTIPLAYER panel (UTF-8, en/es/ru; others → en).
// mode: 0 = main menu (no world), 1 = world loaded & not connected, 2 = connected.
// serverLaunched only affects mode 0. Fills 6 entries (may be "").
void GetHelpLines(int mode, bool serverLaunched, const char* out[6]);

// Localized short status lines used by the Join page (UTF-8, two lines separated by '\n').
const char* GetJoinHintPreLoad();
const char* GetJoinHintLoaded();

} // namespace kmp
