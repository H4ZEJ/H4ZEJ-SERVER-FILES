# Horse and Mount Behavior Overview

This document summarizes the code paths that control mount (horse) behaviour after
aligning the `MountSystem` with the legacy horse logic.

## Summoning and Unsummoning
- `CMountActor::Summon` in `game/src/MountSystem.cpp` now mirrors
  `CHARACTER::HorseSummon`. It reuses the owner’s rotation to spawn either close
  (±100 units) or far (2000–2500 units behind the rider), validates the target
  sector, recreates missing or dead mounts, links the rider, and reuses the same
  naming flow before showing the mount. 【F:game/src/MountSystem.cpp†L147-L245】
- `CMountActor::Unsummon` detaches the rider and destroys the NPC just like the
  classic system, resetting internal state used by the follow AI. 【F:game/src/MountSystem.cpp†L129-L144】
- `CMountSystem::Summon` / `Unsummon` manage actor creation and lifecycle so the
  event loop keeps running only while a mount is active. 【F:game/src/MountSystem.cpp†L482-L535】

## Follow AI
- `_UpdateFollowAI` reproduces the horse state machine: it respawns the mount
  when owner distance exceeds ~4500 units or the map changes, starts following
  at 400 units, switches to running beyond 700 units, and otherwise performs the
  idle wander routine (random rotation, collision-aware walk, wait packet). 【F:game/src/MountSystem.cpp†L248-L334】
- The helper `Follow` forwards directly into `CHARACTER::Follow`, so the mount
  inherits the interpolation, collision, and packet emission logic of the classic
  horse behaviour. 【F:game/src/MountSystem.cpp†L356-L362】【F:game/src/char.cpp†L5213-L5296】

## Riding Flow
- `CMountActor::Mount` / `Unmount` (unchanged) drive item affects, while
  `CHARACTER::StartRiding` and `StopRiding` in `game/src/char_horse.cpp` keep the
  same restriction checks (dead, polymorphed, forbidden maps) and summon/despawn
  the physical mount around the rider when mounting or dismounting. 【F:game/src/MountSystem.cpp†L79-L122】【F:game/src/char_horse.cpp†L16-L118】

## Update Loop
- `CMountSystem::Update` continues to drive the per-actor AI tick, cancelling
  itself once every mount is unsummoned to match the old horse loop cadence. 【F:game/src/MountSystem.cpp†L409-L444】
