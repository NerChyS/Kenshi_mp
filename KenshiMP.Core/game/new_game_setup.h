#pragma once
#include <string>

namespace kmp::new_game_setup {

// Main-menu button the player clicked last (hit-tested in the WndProc hook).
enum class MenuButton { None, Continue, NewGame, LoadGame, Import };
void OnMainMenuButtonClicked(MenuButton b);

// Called when a load starts (loading gap) and when the world finished loading.
void OnLoadStarted(bool fromMainMenu);
void OnGameLoaded();

// Per-frame (cheap, self-throttled). Renames the default "Nameless" squad to the player's
// multiplayer name, and on a freshly started game fills the starting characters' hunger.
void Tick();

} // namespace kmp::new_game_setup
