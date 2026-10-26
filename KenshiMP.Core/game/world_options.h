#pragma once
#include "kmp/messages.h"
#include <string>

namespace kmp::world_options {

// Store the host's options (S2C_WorldOptions) and apply them as soon as a world is loaded.
void OnReceived(const MsgWorldOptions& msg);
void OnGameLoaded();      // re-apply (a save load restores the save's own options)
void OnDisconnected();
bool HasOptions();
// Short human-readable lines for the help panel ("" if none). line 0..1
std::string SummaryLine(int line);

} // namespace kmp::world_options
