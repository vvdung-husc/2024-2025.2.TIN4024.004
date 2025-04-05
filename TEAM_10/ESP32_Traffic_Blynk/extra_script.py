Import shutil
Import os

firmware_elf = ".pio/build/nodemcu-32s/firmware.elf"
firmware_bin = ".pio/build/nodemcu-32s/firmware.bin"

if os.path.exists(firmware_elf):
    os.system(f"esptool.py --chip esp32 elf2image --output {firmware_bin} {firmware_elf}")
