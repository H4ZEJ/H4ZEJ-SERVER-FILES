# Mount and Horse Behavior Report

## Overview
This document explains the end-to-end behaviour of horse-style mounts in the server. It covers how mounts are summoned and dismissed, how they follow their owners, and how riding transitions are handled. Where relevant, it highlights the interactions between the classic horse logic (`CHARACTER::HorseSummon`, `CHARACTER::StateHorse`, `CHARACTER::Follow`, etc.) and the unified `MountSystem` implementation.

## Summoning and Unsummoning
- `CMountSystem::Summon()` looks up or creates a `CMountActor` for the requested mount vnum and calls `CMountActor::Summon()`. It also makes sure the periodic update event is running so the AI continues to tick.【F:game/src/MountSystem.cpp†L411-L472】
- `CMountActor::Summon()` reuses the classic horse spawn offsets. Near summons land within ±100 units; far summons spawn 2000–2500 units behind the owner based on facing direction. Existing mount instances are respawned in place when possible, while dead or invalid actors are cleaned up and recreated. Every successful summon re-links the mount to its owner, resets the empire, and sends a wait packet to stabilise the initial position.【F:game/src/MountSystem.cpp†L120-L196】
- `CMountSystem::Unsummon()` delegates to `CMountActor::Unsummon()` and shuts down the AI timer when no mounts remain. The actor itself clears the summon item link, detaches the rider handle, sends a wait packet, and destroys the character, giving parity with the classic horse dismissal behaviour.【F:game/src/MountSystem.cpp†L375-L444】【F:game/src/MountSystem.cpp†L98-L118】

## Follow AI and Movement
- `_UpdateFollowAI()` inside `CMountActor` mirrors `CHARACTER::StateHorse()`. It starts walking toward the owner at 400 units, switches to running beyond 700 units, and idles between 150–300 units. When a mount is pulled too far away or left on another map, it respawns just behind the player and issues a wait packet before resuming pursuit.【F:game/src/MountSystem.cpp†L198-L266】
- The helper `CMountActor::Follow()` now defers entirely to `CHARACTER::Follow()`, preserving direction interpolation, collision avoidance, and movement packet generation from the classic horse implementation.【F:game/src/MountSystem.cpp†L298-L304】【F:game/src/char.cpp†L5213-L5353】
- `CMountSystem::Update()` keeps all active mounts in sync with their owners, purging orphaned actors and scheduling per-frame AI updates. This ensures follow logic, cleanup, and resummon triggers fire consistently.【F:game/src/MountSystem.cpp†L310-L362】

## Riding Transitions
- `CHARACTER::StartRiding()` enforces the classic restrictions (no riding while dead, polymorphed, or on blocked maps) before removing any summoned mount and switching the player into the riding state with the proper vnum. It also surfaces feedback when requirements are not met.【F:game/src/char_horse.cpp†L16-L64】
- `CHARACTER::StopRiding()` clears the riding state, reapplies quest bookkeeping, and re-summons the previous mount next to the owner. Stat bonuses are normalised after dismounting.【F:game/src/char_horse.cpp†L66-L104】
- Behind the scenes `CHorseRider::StartRiding()` and `CHorseRider::StopRiding()` toggle the shared horse rider flags and stamina events to keep the classic horse mechanics intact while the mount system manages the physical entity.【F:game/src/horse_rider.cpp†L165-L200】

## Classic Horse Support Code
- `CHARACTER::HorseSummon()` still provides the baseline horse summon / unsummon flow, including death handling, naming, and respawn positioning. It is the behavioural template that the `MountSystem` now mirrors for every mount type.【F:game/src/char_horse.cpp†L142-L222】
- `CHARACTER::StateHorse()` defines the original AI thresholds (follow at 400, run at 700, idle near 150–300) and idle wandering logic. These values are reused inside the unified mount follow AI.【F:game/src/char_state.cpp†L1105-L1145】
- `CHARACTER::Follow()` implements the direction smoothing, collision checks, and final movement packet emission used both by the classic horse and the new mount actors.【F:game/src/char.cpp†L5213-L5353】

## Lifecycle Safeguards
- `CMountActor::Update()` continuously validates owner state, mount health, and the backing item link. Any invalid condition immediately unsummons the actor, preventing desynchronised mounts.【F:game/src/MountSystem.cpp†L268-L296】
- When a mount is turned into a ride, `CMountActor::Mount()` and `CMountActor::Unmount()` coordinate item affects, rider state, and horse removal to ensure only one active mount entity exists and the player inherits the correct riding stats.【F:game/src/MountSystem.cpp†L40-L118】

Together these components deliver a unified mount behaviour system that behaves exactly like the classic horse across summoning, following, and riding scenarios.
