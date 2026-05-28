# Unihiker K10 Skills

Codex skills for programming the Unihiker K10 board with Arduino/C++ or MicroPython. The repository is intended to be installed as local agent skills, then used by Codex when it needs K10 APIs, upload helpers, firmware flashing steps, or board-specific troubleshooting.

## Contents

| Skill | Purpose |
| --- | --- |
| `unihiker-k10-arduino` | Arduino/C++ development, K10 BSP setup, sketch compilation and upload, Arduino API references, examples, and Windows upload helpers. |
| `unihiker-k10-micropython` | MicroPython firmware flashing, Python file upload, MicroPython API references, and K10 firmware bundle. |

## Install

Copy or symlink the skill folders into your Codex skills directory:

```bash
mkdir -p ~/.agents/skills
cp -R unihiker-k10-arduino ~/.agents/skills/
cp -R unihiker-k10-micropython ~/.agents/skills/
```

On Windows PowerShell:

```powershell
New-Item -ItemType Directory -Force "$env:USERPROFILE\.agents\skills"
Copy-Item -Recurse .\unihiker-k10-arduino "$env:USERPROFILE\.agents\skills\"
Copy-Item -Recurse .\unihiker-k10-micropython "$env:USERPROFILE\.agents\skills\"
```

Restart Codex after installing or updating skills so it can reload the skill metadata.

## Arduino Quick Start

1. Install the K10 board support package:

   ```bash
   arduino-cli config add board_manager.additional_urls https://downloadcd.dfrobot.com.cn/UNIHIKER/package_unihiker_index.json
   arduino-cli core update-index
   arduino-cli core install UNIHIKER:esp32
   ```

2. Optional: enable Arduino CLI's official build cache for faster repeated compiles:

   ```bash
   arduino-cli config set build_cache.path ~/.cache/arduino-build-cache
   arduino-cli config set build_cache.compilations_before_purge 0
   ```

   On Windows PowerShell:

   ```powershell
   arduino-cli config set build_cache.path "$env:LOCALAPPDATA\arduino\build-cache"
   arduino-cli config set build_cache.compilations_before_purge 0
   ```

   Avoid documenting `compiler.cache.*` or `ccache` as standard Arduino CLI setup; current Arduino CLI uses `build_cache.*`.

3. Create a sketch in a same-named directory, for example `hello/hello.ino`.

4. Upload with the helper script:

   ```bash
   bash unihiker-k10-arduino/scripts/upload-arduino.sh hello/hello.ino /dev/cu.usbmodem2201
   ```

Windows users can also use:

```powershell
.\unihiker-k10-arduino\scripts\upload-arduino.ps1 .\hello\hello.ino COM3
```

The Arduino FQBN used by the skill is `UNIHIKER:esp32:k10`.

## MicroPython Quick Start

1. Install flashing and upload tools:

   ```bash
   pip install esptool mpremote
   ```

2. Flash the bundled MicroPython firmware:

   ```bash
   bash unihiker-k10-micropython/scripts/flash-micropython.sh /dev/cu.usbmodem2201
   ```

3. Upload `main.py`:

   ```bash
   bash unihiker-k10-micropython/scripts/upload-micropython.sh main.py /dev/cu.usbmodem2201
   ```

Only `main.py` runs automatically on boot. Other files need to be imported from REPL or by `main.py`.

## Repository Notes

- `SKILL.md` files are the agent-facing entry points and should stay concise enough to load into context.
- Detailed API material belongs in `references/`.
- Deterministic or error-prone workflows belong in `scripts/`.
- The Arduino skill currently bundles `scripts/arduino-cli.exe` for offline Windows use.
- The MicroPython skill currently bundles `firmware/k10-micropython-v0.9.2.bin`.

## Development Checks

Validate skill frontmatter after changing `SKILL.md`:

```bash
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py unihiker-k10-arduino
python ~/.codex/skills/.system/skill-creator/scripts/quick_validate.py unihiker-k10-micropython
```

Run shell syntax checks after editing Bash scripts:

```bash
bash -n unihiker-k10-arduino/scripts/*.sh
bash -n unihiker-k10-micropython/scripts/*.sh
```
