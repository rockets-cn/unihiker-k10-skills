---
name: k10-compile-server
description: Use when compiling UNIHIKER K10 PlatformIO projects via the K10 Compile Server (HTTPS port 8900), opening the browser Web Serial flash page for a compiled build, downloading firmware, flashing a K10 connected to the compile server, or optionally uploading compiled firmware with local esptool. Prefer this skill over local `pio run` when the user wants LAN server compilation and client-side flashing without installing PlatformIO or esptool.
---

# K10 Compile Server

## Overview

The K10 Compile Server is a FastAPI service running on **port 8900** (HTTPS) that compiles UNIHIKER K10 PlatformIO projects remotely. Instead of installing PlatformIO + toolchains locally, upload your project, get a `build_id`, and flash from the browser with Web Serial.

Use an existing LAN server, or self-host one from the public server repo:

```text
https://github.com/rockets-cn/unihiker-k10-compile-server
```

Self-hosting details live in `references/server-setup.md`. The Docker setup persists PlatformIO packages/toolchains in the named volume `pio-data`, so `docker compose down && docker compose up -d` keeps the K10 toolchain cache.

If `platformio.ini` references `partitions.csv`, the server expects that file to be present. It returns an error instead of silently generating a partition table.

## Configuration

**服务器地址：** 根据你的网络环境设置 `COMPILE_SERVER` 环境变量，或在命令中通过 `--server` 指定。

```bash
# 方式 A：环境变量（推荐，一次设置一直用）
export COMPILE_SERVER=https://<server-ip>:8900

# 方式 B：每次命令指定
bash k10-compile-server/scripts/compile-project.sh \
  --server https://<server-ip>:8900 \
  --dir /path/to/k10-project
```

默认地址是 `https://localhost:8900`（服务器本机）。局域网客户端请设置 `COMPILE_SERVER=https://<server-ip>:8900`；可在服务器上通过 `ip a` 或 `ifconfig` 查看实际 IP。

## Quick Start

```bash
# 方式 A：环境变量（推荐，设一次就不用每次都写 --server）
export COMPILE_SERVER=https://<server-ip>:8900

# 1. Use the helper script (recommended)
bash k10-compile-server/scripts/compile-project.sh \
  --server $COMPILE_SERVER \
  --dir /path/to/k10-project \
  --web-serial
```

On Windows clients, prefer the PowerShell helper so the workflow does not depend on WSL networking or Unix `zip`:

```powershell
.\k10-compile-server\scripts\compile-project.ps1 `
  -Server https://<server-ip>:8900 `
  -Dir C:\path\to\k10-project `
  -WebSerial
```

The helper compiles the project, waits for completion, then opens:

```text
https://<server-ip>:8900/?build_id=<build_id>
```

Use Chrome/Edge, click "浏览器烧录", choose the K10 serial port, and let the page flash and reset the board.

Choose the upload mode explicitly:

| Goal | Use |
|------|-----|
| Compile remotely and flash from browser without local upload tools | `--web-serial` or `-WebSerial` |
| Compile only and download firmware | `--output ./firmware.bin` or `-Output .\firmware.bin` |
| Flash a K10 plugged into the compile server | `--flash` or `-Flash` |
| Upload from this client with local esptool instead of browser | `--upload-local --port <port>` or `-UploadLocal -Port COM3` |

## Server Management

Check if the server is running:

```bash
sudo systemctl status k10-compile-server 2>/dev/null || echo "not running as service"
```

Health check API:

```bash
curl -k https://localhost:8900/api/health
```

Expected response:
```json
{
  "status": "ok",
  "k10_toolchain_ready": true,
  "active_compiles": 0,
  "waiting_in_queue": 0
}
```

Restart server:

```bash
cd /path/to/unihiker-k10-compile-server/server
bash restart.sh
```

## Compile a Project

### Step 1: Zip the project

The server accepts projects via `POST /api/compile` (zip) or `POST /api/compile/files` (multi-file). The zip method is simplest for scripted use:

```bash
# Zip the project (must contain platformio.ini + src/*.cpp)
cd /path/to/k10-project
zip -r /tmp/k10-project.zip . \
  -x ".pio/*" ".git/*" "build/*"
```

### Step 2: Submit to compile server

```bash
SERVER="https://<server-ip>:8900"

# Upload zip → get build_id
RESP=$(curl -sk -X POST "$SERVER/api/compile" \
  -F "file=@/tmp/k10-project.zip")
BUILD_ID=$(echo "$RESP" | python3 -c "import sys,json;print(json.load(sys.stdin)['build_id'])")
echo "Build ID: $BUILD_ID"
```

### Step 3: Poll until done

```bash
while true; do
  STATUS=$(curl -sk "$SERVER/api/build/$BUILD_ID/status")
  STATE=$(echo "$STATUS" | python3 -c "import sys,json;print(json.load(sys.stdin)['status'])")
  echo "Status: $STATE"
  if [ "$STATE" = "done" ]; then
    echo "Compile complete!"
    SIZE=$(echo "$STATUS" | python3 -c "import sys,json;print(json.load(sys.stdin)['bin_size'])")
    echo "Firmware size: $((SIZE/1024)) KB"
    break
  elif [ "$STATE" = "error" ]; then
    echo "Compile failed!"
    echo "$STATUS" | python3 -c "import sys,json;d=json.load(sys.stdin);print(d.get('error',''),d.get('log','')[-500:])"
    exit 1
  fi
  sleep 3
done
```

### Step 4: Download the firmware

```bash
# Download combined firmware.bin
curl -sk -o firmware.bin "$SERVER/api/build/$BUILD_ID/download"
ls -lh firmware.bin
```

Or get individual flash files with their offsets:

```bash
# Get flash manifest (files + offsets)
curl -sk "$SERVER/api/build/$BUILD_ID/flash-files" | python3 -m json.tool

# Download each file
for FILE in bootloader partitions firmware; do
  curl -sk -o "${FILE}.bin" "$SERVER/api/build/$BUILD_ID/file/${FILE}.bin"
done
```

The standard ESP32-S3 flash layout is:
| File | Flash Offset | Purpose |
|------|-------------|---------|
| `bootloader.bin` | `0x0` | ESP32-S3 bootloader |
| `partitions.bin` | `0x8000` | Partition table |
| `firmware.bin` | `0x10000` | Application code |

## Flash from Browser (Web Serial)

The compile server's web page at `https://<server-ip>:8900/?build_id=<id>` supports direct browser-based flashing of API/CLI-created builds:

1. Compile with `--web-serial` / `-WebSerial`, or otherwise obtain a completed `build_id`.
2. Open `https://<server-ip>:8900/?build_id=<build_id>` in Chrome/Edge.
3. Click "⚡ 浏览器烧录".
4. Select the K10 serial port when prompted.
5. Let the page enter download mode, flash, reset, and release the serial port.

Requirements: Chrome/Edge, HTTPS, USB-C connection to the K10 board.

This mode requires no local PlatformIO, no local K10 toolchain, and no local `esptool`; the browser uses Web Serial and the server-hosted `esptool-js` bundle. Browser security still requires a user gesture: the user must click the flash button and choose the serial port.

The server frontend must support `?build_id=<id>` and initialize its flash manifest from `/api/build/<id>/flash-files`.

If automatic reset is unreliable, the page should show a BOOT/RST fallback. On current server builds, flashing completion uses a DTR/RTS reset-to-firmware sequence and releases the Web Serial connection.

## Flash from Server

If the K10 is connected via USB to the **compile server machine**, use the server-side flash:

```bash
curl -sk -X POST "$SERVER/api/flash/$BUILD_ID"
```

Or via the helper script:

```bash
bash k10-compile-server/scripts/compile-project.sh \
  --server https://<server-ip>:8900 \
  --dir /path/to/k10-project \
  --flash
```

## Upload from Local Client USB with esptool

Prefer browser Web Serial for normal classroom/client workflows. Use this local esptool path only when the user explicitly wants command-line upload and has local Python/esptool available.

If the K10 is connected via USB to the **client machine** running Codex, compile remotely, download the flash files, then upload locally with `esptool`:

```bash
bash k10-compile-server/scripts/compile-project.sh \
  --server https://<server-ip>:8900 \
  --dir /path/to/k10-project \
  --upload-local \
  --port /dev/cu.usbmodemXXXX
```

Windows PowerShell:

```powershell
.\k10-compile-server\scripts\compile-project.ps1 `
  -Server https://<server-ip>:8900 `
  -Dir C:\path\to\k10-project `
  -UploadLocal `
  -Port COM3
```

If the port is omitted, the helper auto-detects only when there is exactly one likely USB serial port. Pass the port explicitly when multiple boards or serial devices are connected.

Client-side upload requires local `esptool` only; it does not require the full PlatformIO K10 toolchain:

```bash
python3 -m pip install esptool
```

```powershell
python -m pip install esptool
```

The local upload writes the standard ESP32-S3 flash layout returned by the server:

```text
0x0     bootloader.bin
0x8000  partitions.bin
0x10000 firmware.bin
```

If automatic reset does not enter the bootloader, hold BOOT, tap RST, then release BOOT while the upload is connecting.

## All-in-One One-Liner (for CI / quick use)

```bash
# Complete pipeline: zip → submit → poll → open browser flash page
bash k10-compile-server/scripts/compile-project.sh \
  --server https://<server-ip>:8900 \
  --dir /path/to/k10-project \
  --web-serial
```

Remote compile and firmware download only:

```bash
bash k10-compile-server/scripts/compile-project.sh \
  --server https://<server-ip>:8900 \
  --dir /path/to/k10-project \
  --output ./firmware.bin
```

## Error Handling

| Symptom | Likely Cause | Fix |
|---------|-------------|-----|
| `"k10_toolchain_ready": false` | Server hasn't built a K10 project yet | Run a dummy build on the server first, or install the PlatformIO K10 platform |
| `error: "未找到 platformio.ini"` | Project missing platformio.ini or zip structure wrong | Make sure platformio.ini is at the zip root |
| `error: "缺少 partitions.csv"` | `platformio.ini` references a partition table that was not uploaded | Add the matching `partitions.csv` next to `platformio.ini` |
| `error: "编译超时"` | Project takes > 5 min to compile | Simplify project, or use local `pio run` for this project |
| Connection refused | Server not running | Start the Docker/systemd service from `references/server-setup.md` |
| SSL cert error on curl | Self-signed cert | Use `-k` flag (insecure) for curl |
| Browser flash page says build_id is expired | Server restarted or build cache expired | Recompile and open the new `?build_id=<id>` URL |
| Web Serial unsupported | Browser is not Chromium-based or page is not HTTPS | Use Chrome/Edge over HTTPS |
| Web Serial cannot see the board | Wrong port, cable, or browser permission | Replug K10, use a data-capable USB-C cable, and select the K10 serial port |
| Browser upload cannot enter download mode | Automatic reset failed | Follow the page's BOOT/RST fallback prompt |
| Server-side flash fails | K10 not connected to server | Check `ls /dev/ttyUSB*` or `ls /dev/ttyACM*` on server |
| Client-side upload fails with missing esptool | Local client lacks upload tool | Run `python -m pip install esptool` |
| Client-side upload cannot find port | Multiple serial devices or no K10 connected | Pass `--port <port>` / `-Port COM3` explicitly |
| Client-side upload cannot connect | K10 did not enter bootloader mode | Hold BOOT, tap RST, then release BOOT while upload is connecting |

## Related Skills

- `unihiker-k10-platformio` — local PlatformIO development (no server needed)
- `unihiker-k10-ota` — OTA firmware update workflow
- `unihiker-k10-arduino` — Arduino CLI development
