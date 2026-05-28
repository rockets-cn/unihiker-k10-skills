#!/usr/bin/env python3
"""Upload firmware to K10 pH Titrator via HTTP OTA."""

import argparse
import sys
import urllib.request


def upload(ip: str, bin_path: str) -> bool:
    url = f"http://{ip}/ota"
    print(f"Uploading {bin_path} to {url} ...")

    with open(bin_path, "rb") as f:
        data = f.read()

    boundary = "----WebKitFormBoundaryK10OTA"
    body = (
        f"--{boundary}\r\n"
        f'Content-Disposition: form-data; name="file"; filename="firmware.bin"\r\n'
        f"Content-Type: application/octet-stream\r\n\r\n"
    ).encode("utf-8")
    body += data
    body += f"\r\n--{boundary}--\r\n".encode("utf-8")

    req = urllib.request.Request(
        url,
        data=body,
        headers={
            "Content-Type": f"multipart/form-data; boundary={boundary}",
            "Connection": "close",
        },
        method="POST",
    )

    try:
        with urllib.request.urlopen(req, timeout=60) as resp:
            result = resp.read().decode("utf-8")
            print(f"Response: {result}")
            if result == "OK":
                print("OTA upload successful. Device will restart in ~1.2s.")
                return True
            else:
                print("OTA upload failed.")
                return False
    except Exception as e:
        print(f"Error: {e}")
        return False


def main():
    parser = argparse.ArgumentParser(description="K10 pH Titrator OTA Uploader")
    parser.add_argument("bin", help="Path to .bin firmware file")
    parser.add_argument("--ip", default="192.168.9.42", help="Device IP address")
    args = parser.parse_args()

    if not upload(args.ip, args.bin):
        sys.exit(1)


if __name__ == "__main__":
    main()
