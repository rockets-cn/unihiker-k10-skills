# Agent Index

This file is the cold-start map for agents working in this repository. Read it before choosing a K10 skill.

## Read Order

1. Read this file.
2. Select exactly the skill that matches the task.
3. Read that skill's `SKILL.md`.
4. Read only the referenced API or implementation files needed for the task.
5. Prefer the provided scripts for upload, flashing, compile, and OTA work.
6. For AI, voice, OTA, partition-table, or factory-recovery work, read `references/k10-ai-model-flash.md`.

## Task Routing

| Task | Primary skill | Required follow-up |
| --- | --- | --- |
| Create, fix, compile, or upload an Arduino sketch | `unihiker-k10-arduino` | Read Arduino API references before using K10-specific classes. |
| Set up Arduino CLI or K10 BSP | `unihiker-k10-arduino` | Configure both the K10 BSP URL and an ESP32 Core URL before installing `UNIHIKER:esp32`. |
| Diagnose serial upload or port issues | `unihiker-k10-arduino` or `unihiker-k10-micropython` | Use the matching `find-port` or upload script instead of inventing port detection logic. |
| Flash MicroPython firmware | `unihiker-k10-micropython` | Use the bundled firmware and the documented BOOT/RST download-mode sequence. |
| Upload or debug MicroPython code | `unihiker-k10-micropython` | Use `main.py` for auto-run behavior; non-entry files require REPL import or `main.py` import. |
| Add HTTP OTA to an Arduino project | `unihiker-k10-ota` | Add OTA partitions, add an HTTP `/ota` endpoint, and perform one USB upload before wireless updates. |
| Combine K10 AI functions with OTA or recover missing AI models | `unihiker-k10-ota`, `unihiker-k10-arduino`, or `unihiker-k10-platformio` | Read `references/k10-ai-model-flash.md`; preserve model offsets and use a model-refresh upload only when needed. |
| Add OTA to an ESP-NOW project | `unihiker-k10-ota` | Use a maintenance OTA mode with AP/STA networking; do not claim HTTP OTA works over pure ESP-NOW packets. |
| Improve screen animation or sensor display refresh | `unihiker-k10-arduino` or `unihiker-k10-micropython` | Prefer partial redraws and avoid repeated full-screen clearing in loops. |

## Non-Negotiable Constraints

- Arduino and MicroPython firmware are mutually exclusive on the board.
- Arduino FQBN is `UNIHIKER:esp32:k10`.
- Arduino sketches must use same-named sketch directories.
- Arduino canvas calls use `k10.canvas->`.
- Current Arduino CLI cache settings use `build_cache.*`; do not introduce `compiler.cache.*` or `ccache` as standard setup.
- MicroPython `v0.9.2` has an AI + WiFi resource conflict; do not combine them unless the user explicitly validates a newer path.
- HTTP OTA requires OTA partitions and IP networking. ESP-NOW is not an HTTP transport.
- If an OTA-enabled sketch is replaced by a sketch without the OTA endpoint, wireless update capability is lost until USB flashing restores it.
- K10 built-in AI model data lives at fixed flash offsets starting at `0x510000`; OTA app partitions must not overlap these regions.
- A full flash erase followed by `Model=None` can leave AI model partitions blank. Use Mind+ Restore Initial Settings or a one-time Arduino/PlatformIO model-refresh upload.

## Script Preferences

Use scripts when the task involves deterministic device operations:

| Operation | Preferred scripts |
| --- | --- |
| Arduino serial upload | `unihiker-k10-arduino/scripts/upload-arduino.sh`, `upload-arduino.ps1`, `upload_k10.py`, or `upload-k10.bat` |
| Arduino OTA compile/upload | `unihiker-k10-arduino/scripts/compile-ota.sh` or `compile-ota.ps1` |
| MicroPython firmware flashing | `unihiker-k10-micropython/scripts/flash-micropython.sh` or `flash-mp-auto.sh` |
| MicroPython file upload | `unihiker-k10-micropython/scripts/upload-micropython.sh` |
| HTTP OTA binary upload | `unihiker-k10-ota/scripts/ota_upload.py` or `ota_upload.ps1` |

## Verification Commands

Run the relevant checks after documentation or skill changes:

```bash
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py unihiker-k10-arduino
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py unihiker-k10-micropython
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py unihiker-k10-ota
bash -n unihiker-k10-arduino/scripts/*.sh
bash -n unihiker-k10-micropython/scripts/*.sh
```

For code generation tasks, verification should also include the appropriate compile, upload dry run, or firmware-flashing preflight when hardware access is available.
