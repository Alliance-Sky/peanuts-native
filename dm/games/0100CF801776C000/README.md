<!-- PEANUTS: new game profile for LIVE A LIVE (0100CF801776C000) -->
# LIVE A LIVE (`0100CF801776C000`)

Update 1.0.1 (`v65536`) over the base game, NintendoSDK `nnSdk-12_3_2-Release`, NVN API 55.15. Square Enix / historia, Unreal Engine 4 (`FMallocBinned3`), modules `main`, `subsdk0`, `sdk`.

Status: playable on Android (OnePlus 13R, Snapdragon 8 Gen 3 / Adreno 750, 11.0 GiB RAM, Android 16) at locked 60 fps (`fps_target 60`, true 1x speed) through title screen, Chapter Select, software keyboard (`swkbd`), Wild West overworld/saloon, and Mad Dog battle.

## Build

- `v65536	1.0.1	main=cf22083371ddacb2c707e3e12cc92e77	subsdk0=dade22cfe6f24efe62d76409522c850b	sdk=5bcf525f24eb28658a8d67ac27800d9b`

## Shaders

- `14,736` shaders (`166` compute, `12,404` pixel, `2,166` vertex), `0` conflicts, `16,820` SPIR-V modules (`122.2 MB` compressed `dmsh_vk.lib`), every module valid for `spirv-val --target-env vulkan1.3`.

## Android

Tested 2026-11-28 on OnePlus 13R (Adreno 750, 11.0 GiB, Android 16), `v65536` (`1.0.1`), `fps_target 60` (`rhi.SyncInterval` patched `2 -> 1`, `interval 1 100 %`):

- **Turnip (Mr. Purple T30, `26.3.0-T30-1.4.359`, driver ID 18):**
  - Warm launch (`DM_VK_PSO_ASYNC=2`, 237 keys prewarmed): locked **59.8–60.2 fps** in town/saloon (`1,500–1,835` draws/frame, GPU `p50 3.0–4.2 ms`) and **60.0–60.1 fps** in battle (`793–990` draws/frame, GPU `p50 3.2–3.6 ms`, `0` dropped frames), `game 0.990–1.026` of real time (`tempo 1.000`, `0` underruns on stream `#1`), `swkbd` OK, `0` crashes, `thermal 0`.
  - Cold launch with empty cache under `DM_VK_PSO_ASYNC=2` hits the known UE4 skipped-draw exposure washout ([`docs/reference/rendering.md`](../../../docs/reference/rendering.md#a-draw-skipped-for-its-pipeline-left-a-result-the-game-kept-android)); fixed on launch 2+ with warm cache or on cold launch with `DM_VK_PSO_ASYNC=0`.
- **Qualcomm system driver (`512.762.41` / `0x802fa029`, driver ID 8):**
  - Because `DM_VK_PREWARM=0` is forced on Qualcomm drivers before `512.800`, default `DM_VK_PSO_ASYNC=2` skips draws on both cold (`10,641` skipped) and warm (`398–1,035` skipped) launches, causing a white/green title blow-out, and hit `SIGSEGV` in `vulkan.adreno.so+0x1f3a48` on `vk-enc` when entering Twilight of Edo Japan. `game.tsv` therefore recommends installed Turnip (`driver=18`) on Qualcomm drivers before `512.800`.
  - **Workaround on Qualcomm system driver:** turning **Game settings › Background pipelines** off (`DM_VK_PSO_ASYNC=0`) fixes the washout from cold start (`0` bytes cache read) and ran `4,614` frames through Wild West town/saloon and Mad Dog battle with `0` crashes (`326` pipelines, `0` failed), locked **60.0–60.1 fps** in town/saloon (`0` dropped frames, GPU `p50 5.4–6.6 ms`) and **58.0–59.2 fps** in battle, `0` underruns on stream `#1`, `thermal 2`.
- **Not checked:** Qualcomm drivers `512.800+`, other chapters beyond Wild West and Twilight of Edo Japan, dual-screen handhelds / companion.

## Open

- Cold launch with an empty pipeline cache under `DM_VK_PSO_ASYNC=2` washes out the title screen until the cache warms (or with `DM_VK_PSO_ASYNC=0`). No `vk_prewarm.bin` is committed per `CONTRIBUTING.md`.
