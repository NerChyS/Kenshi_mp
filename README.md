# Kenshi Online — Working Build (co-op multiplayer for Kenshi)

**Co-op multiplayer mod for Kenshi 1.0.68 (Steam), up to 16 players — fixed and working build.**

This is a fork of [The404Studios/Kenshi-Online](https://github.com/The404Studios/Kenshi-Online) (MIT).
The original alpha crashed when loading a game or when a second player joined; this build fixes those
crashes and adds a few things needed to actually play together (see [What was fixed](#what-was-fixed)).

> Status: **alpha, but playable**. Tested with two players: both connect, see each other with their real
> character (race, face, hair, body), move around together, no crashes.

---

## Download & install (one click)

1. Download **[`release/KenshiOnline_Setup.exe`](release/KenshiOnline_Setup.exe)** (installer in English / Español / Русский).
2. Close Kenshi and run it. If Windows SmartScreen appears: **More info → Run anyway** (the installer is not code-signed).
3. It finds your Kenshi folder automatically (or pick the folder with `kenshi_x64.exe`) → **Install**.

Every player must install the same version.

## Guides (PDF)

| Language | Guide |
|---|---|
| English | [Kenshi_Online_Guide_EN.pdf](docs/guides/Kenshi_Online_Guide_EN.pdf) |
| Español | [Kenshi_Online_Guia_ES.pdf](docs/guides/Kenshi_Online_Guia_ES.pdf) |
| Русский | [Kenshi_Online_Guide_RU.pdf](docs/guides/Kenshi_Online_Guide_RU.pdf) |

## Quick start

**Host (one player):**
1. Run `KenshiMP.Server.exe` from the Kenshi folder and keep the console open.
2. Forward port **27800 UDP** in your router to your PC (or use Radmin VPN / ZeroTier / Tailscale).
3. Give your public IP (or VPN IP) to your friends.

**Everyone (host included):**
1. Kenshi → **New Game** → choose the **Multiplayer** start → **Start**.
2. Create your character (any race) → **Confirm**.
3. Wait ~90 s until the **KENSHI ONLINE** bar appears at the top.
4. **F1 → JOIN GAME** → Server IP (`127.0.0.1` for the host, the host's IP for the others), port `27800` → **CONNECT**.

Keys: `F1` multiplayer menu · `Enter` chat · `Tab` player list.

## What was fixed

Compared with the original 0.3.0-alpha release:

- **Crash on load / in worker threads** — `CharacterCreate`, `AICreate`, `AIPackages` and `TimeUpdate` hooks
  forwarded the wrong number of arguments (stack args 5–6 were dropped), corrupting `Character+0x318`.
- **`/GS` stack-cookie crash** — the `mov rax, rsp` trampoline wrapper now reserves a probed 256 KB stack gap.
- **Wrong `CharacterCreate` pattern** in the source (resolved to an unrelated function).
- **Player not detected** — wrong `GameWorld` address and "character list" offset; now uses the verified
  Steam 1.0.68 layout (`GameWorld` static object, `PlayerInterface` at +0x580, `playerCharacters` at +0x2B0).
- **Crash when the 2nd player joins** — the "teleport to host" called `HavokCharacter::setPosition` with a
  `Character*`; now uses `CharMovement::_setPositionAndTeleport` through the object's vtable.
- **Host freeze after spawning a remote player** — the "FactionRelation" pattern pointed to a tutorial/camera
  UI function; hook and calls disabled.
- **Remote player never spawned** — `RootObjectFactory::create`/`createRandomCharacter` were called with
  missing arguments; now called with their real signatures, the global factory and one of the mod's player factions.
- **Appearance sync (new)** — each client exports its character's appearance with the game's own
  `GameData::saveToFile` (same format as the character editor's Export), sends it through the chat relay
  (hidden), and the receiver applies it with `clearEverything`/`updateFrom` + `AppearanceBase::reload`.
- **kenshi-online.mod** — the *Multiplayer* start only contained an animal (no race choice) and the
  *Singleplayer* start spawned 20 "Player 1" clones; both fixed.
- **Server** — no more ownerless "ghost" characters restored from `world.kmpsave`; hidden data messages are not logged.
- Default server IP is `127.0.0.1` instead of the original author's master server.
- Builds use the static CRT (no Visual C++ redistributable needed).

## What's new in 0.3.3

- **Host world settings** — `server.json` → `worldOptions` (damage, build/research/production speed,
  starvation time, nests, death frequency, easy prospecting, bandits loot player, animals eat, difficult healing).
  Every player gets them on connect; shown in the F1 panel. `masterServerEnabled` is `false` by default.
- **Full hunger** at the start of a Multiplayer game (the start squad was set to "malnourished").
- **F1** no longer opens Kenshi's Tutorials window; typing in chat/fields no longer reaches the game and fast typing no longer drops letters.
- **MULTIPLAYER button** follows the game language (e.g. MULTIJUGADOR).
- Joiners no longer lose the host's character when far away (heartbeat listed only nearby entities);
  no more "Authority violation" spam; auto-connect sends the handshake; positive load countdown; less log/chat noise.
- Known issues: the default squad name ("Nameless") is not yet renamed on non-English games, and connecting
  from the main menu *before* New Game is unreliable for joiners — connect from inside the world (F1 → JOIN GAME).

## Known limitations

- Combat damage is not fully synced; NPC AI runs locally on each PC.
- Running two copies of Kenshi on the **same PC** can crash at autosave (they share the save folder) — use separate PCs.
- Only Kenshi **1.0.68 Steam** is verified.

## Building

Windows + MSVC (or Linux cross-compile with `clang-cl` + [xwin](https://github.com/Jake-Shadle/xwin), see
`cmake/clangcl-toolchain.cmake`). Dependencies (put them in `lib/`): enet v1.3.18, MinHook, spdlog v1.14.1,
nlohmann/json v3.11.3.

```bash
cmake -B out -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded \
      -DCMAKE_TOOLCHAIN_FILE=cmake/clangcl-toolchain.cmake   # only when cross-compiling
cmake --build out --target KenshiMP.Core KenshiMP.Server
```

`KenshiMP.Core`, `KenshiMP.Common` and `KenshiMP.Scanner` must be compiled with `/EHa` (async SEH) so that
`__try/__except` catches faulting memory reads. The installer is built with NSIS from `installer/KenshiOnline.nsi`.

## Credits & license

- Original mod: **The404Studios / KenshiMP contributors** — MIT license (see `LICENSE`).
- Struct offsets cross-checked against [KenshiLib](https://github.com/BFrizzleFoShizzle/KenshiLib).
- Original README: [docs/ORIGINAL_README.md](docs/ORIGINAL_README.md).

---

### Español (resumen)
Mod multijugador cooperativo para Kenshi 1.0.68 (Steam), versión corregida y funcional. Descarga
`release/KenshiOnline_Setup.exe`, instálalo (todos los jugadores) y sigue la
[guía en español](docs/guides/Kenshi_Online_Guia_ES.pdf): Nuevo juego → inicio *Multiplayer* → espera ~90 s → F1 → JOIN GAME.

### Русский (кратко)
Кооперативный мод мультиплеера для Kenshi 1.0.68 (Steam), исправленная рабочая сборка. Скачайте
`release/KenshiOnline_Setup.exe`, установите (всем игрокам) и следуйте
[руководству на русском](docs/guides/Kenshi_Online_Guide_RU.pdf): Новая игра → старт *Multiplayer* → подождите ~90 с → F1 → JOIN GAME.
