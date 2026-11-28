<!-- PEANUTS: Peanuts-only profile -->
# LIVE A LIVE (`0100CF801776C000`)

## Status

Bring-up in progress on Android (local `live-a-live` branch, not yet PR'd).

## Builds

- `v65536` (`1.0.1`): `main=cf22083371ddacb2c707e3e12cc92e77`, `subsdk0=dade22cfe6f24efe62d76409522c850b`, `sdk=5bcf525f24eb28658a8d67ac27800d9b` (NintendoSDK 12.3.2, Unreal Engine 4, Wwise, NVN).

## Shaders

14,736 shaders split from `Richard/Content/Paks/Richard-Switch.pak` + `main` (4,332 pairs, 166 facts, 0 conflicts).

## Memory

Uses the same UE4 + SDK 12.3.2 guest layout as OCTOPATH TRAVELER II (`mem_reserve 0xF0000000`, `mem_alias 0xC8000000`, `mem_stack 0x10000000`, `app_pool 0xCD500000`, `sys_resource 0x5000000`).
