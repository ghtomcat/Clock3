# Installation Instructions

## Prerequisites

- ESP32 Development Board
- TM1637 4-Digit 7-Segment Display Module
- USB cable for programming
- Arduino IDE 2.x or PlatformIO

---

## Option 1: Arduino IDE Installation

### Step 1: Install ESP32 Board Support

1. Open Arduino IDE
2. Go to **File > Preferences**
3. In "Additional Board Manager URLs", add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools > Board > Boards Manager**
5. Search for "esp32"
6. Install "esp32" by Espressif Systems

### Step 2: Install TM1637 Library

1. Go to **Sketch > Include Library > Manage Libraries**
2. In the Library Manager search box, type: `TM1637`
3. Find **"TM1637" by Avishay Orpaz**
4. Click **Install**
5. Wait for installation to complete

![Library Manager Screenshot - search for TM1637 by Avishay Orpaz]

### Step 3: Select Board and Port

1. Go to **Tools > Board > ESP32 Arduino**
2. Select your board (typically **ESP32 Dev Module**)
3. Go to **Tools > Port**
4. Select the port your ESP32 is connected to
   - macOS: `/dev/cu.usbserial-*` or `/dev/cu.SLAB_USBtoUART`
   - Windows: `COM3`, `COM4`, etc.
   - Linux: `/dev/ttyUSB0`, `/dev/ttyACM0`, etc.

### Step 4: Verify Installation

1. Open `Clock3.ino` in Arduino IDE
2. Click **Verify** (checkmark button)
3. You should see "Done compiling" with no errors

**Note:** You may see a warning about `config.h` not being found - this is expected and will be resolved in Task 4.

---

## Option 2: PlatformIO Installation

### Step 1: Install PlatformIO

If you haven't installed PlatformIO yet:

**VS Code:**
1. Open VS Code
2. Go to Extensions (Ctrl+Shift+X / Cmd+Shift+X)
3. Search for "PlatformIO IDE"
4. Click Install
5. Restart VS Code

**CLI:**
```bash
pip install platformio
```

### Step 2: Open Project

1. Open the `Clock3` folder in VS Code / PlatformIO
2. PlatformIO will automatically detect `platformio.ini`
3. The TM1637 library will be automatically installed when you first build

### Step 3: Build Project

**VS Code:**
- Click the **Build** button (checkmark) in the PlatformIO toolbar at the bottom

**CLI:**
```bash
cd Clock3
pio run
```

### Step 4: Upload to ESP32

**VS Code:**
- Connect your ESP32 via USB
- Click the **Upload** button (right arrow) in the PlatformIO toolbar

**CLI:**
```bash
pio run --target upload
```

**Note:** You may need to adjust the `upload_port` in `platformio.ini` to match your ESP32's port.

---

## Hardware Connection

Before uploading code, connect your TM1637 display to the ESP32:

| TM1637 Pin | ESP32 GPIO | Wire Color (typical) |
|------------|------------|---------------------|
| CLK        | GPIO 21    | Yellow              |
| DIO        | GPIO 22    | Green               |
| VCC        | 3.3V or 5V | Red                 |
| GND        | GND        | Black               |

**Important:**
- The TM1637 module works with both 3.3V and 5V
- Make sure connections are secure
- Double-check GPIO pins match the configuration

---

## Troubleshooting

### Library Not Found

**Arduino IDE:**
- Make sure you installed "TM1637" by **Avishay Orpaz** (not other TM1637 libraries)
- Try closing and reopening Arduino IDE
- Check Library Manager again to confirm installation

**PlatformIO:**
- Delete `.pio` folder and rebuild
- Check that `platformio.ini` contains: `avishorp/TM1637@^1.2.0`

### ESP32 Not Detected

- Check USB cable (some cables are power-only)
- Install CP210x or CH340 USB drivers if needed
- Try different USB port
- Press and hold BOOT button while uploading (some ESP32 boards)

### Compilation Errors

**"config.h: No such file or directory"**
- This is expected until Task 4 is complete
- Continue with next task to create config.h

### Upload Fails

- Make sure no other program (Serial Monitor, etc.) is using the serial port
- Try pressing BOOT button on ESP32 during upload
- Check that correct port is selected
- Try reducing upload speed in Tools > Upload Speed

---

## Verifying Installation

After installation, you should be able to:

1. ✓ Compile the project without library errors
2. ✓ See TM1637Display.h is recognized
3. ✓ Upload to ESP32 successfully
4. ✓ Open Serial Monitor at 115200 baud and see startup messages

---

## Next Steps

Once the library is installed:
1. Proceed to Task 4: Create config.h file
2. Continue with Phase 1 implementation

## Library Documentation

- **GitHub Repository:** https://github.com/avishorp/TM1637
- **Examples:** https://github.com/avishorp/TM1637/tree/master/examples
- **Documentation:** https://github.com/avishorp/TM1637/blob/master/README.md

---

## Support

If you encounter issues:
1. Check the troubleshooting section above
2. Review the [TM1637 library issues page](https://github.com/avishorp/TM1637/issues)
3. Verify ESP32 board support is properly installed
4. Check hardware connections
