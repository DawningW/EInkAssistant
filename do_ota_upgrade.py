import sys
import os
import requests

if __name__ == "__main__":
    bin_path = "./build/EInkAssistant.ino.bin"
    if len(sys.argv) > 1:
        bin_path = sys.argv[1]

    host = input("请输入设备的 IP 地址 (默认是 einkassistant.local):\n")
    if not host:
        host = "einkassistant.local"
    url = f"http://{host}/update"

    print(f"Uploading {bin_path} to {url}...")
    with open(bin_path, 'rb') as f:
        files = {"image": (os.path.basename(bin_path), f)}
        try:
            response = requests.post(url, files=files)
            if response.status_code == 200:
                print("Update successful!")
            else:
                print(f"Update failed! code: {response.status_code}, body: {response.text}")
        except requests.exceptions.RequestException as e:
            print(f"Update error: {e}")
