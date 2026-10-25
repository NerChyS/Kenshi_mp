#pragma once
#include <cstdint>
#include <string>

// Appearance sync: sends the local character's appearance (race, gender, face,
// hair, body sliders) to the other players and applies received appearances to
// the remote player's puppet character.
//
// Transport: chunked through the existing chat relay (messages starting with
// "\x01KA"), so no protocol change is needed. Chunks are hidden from chat.
namespace kmp::appearance_sync {

// Ask for the local appearance to be (re)broadcast on the next game tick.
void RequestBroadcast();

// Network thread: chat payload starting with the appearance prefix.
// Returns true if the message was an appearance chunk (and must not be shown).
bool OnChatPayload(uint32_t senderId, const std::string& message);

// Forget a player's appearance (player left).
void OnPlayerLeft(uint32_t playerId);

// Game thread: send pending broadcast, apply received appearances.
void Tick();

// Clear everything (disconnect).
void Reset();

} // namespace kmp::appearance_sync
