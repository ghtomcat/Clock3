# Hardware Testing Guide

## Task 5: Display Initialization Test

This guide will help you test the TM1637 display initialization with your ESP32 hardware.

---

## Prerequisites

Before testing, ensure you have:
- ✓ ESP32 Development Board
- ✓ TM1637 4-Digit 7-Segment Display Module
- ✓ Jumper wires for connections
- ✓ USB cable for programming
- ✓ Arduino IDE or PlatformIO installed
- ✓ TM1637 library installed (Task 3)

---

## Step 1: Hardware Connections

Connect the TM1637 display to your ESP32:

| TM1637 Pin | ESP32 Pin | Description |
|------------|-----------|-------------|
| CLK        | GPIO 21   | Clock signal |
| DIO        | GPIO 22   | Data I/O |
| VCC        | 3.3V or 5V | Power supply |
| GND        | GND       | Ground |

**Important Notes:**
- The TM1637 works with both 3.3V and 5V
- Double-check connections before powering on
- Ensure wires are firmly connected

**Wiring Diagram:**
```
ESP32                    TM1637 Display
┌─────────┐              ┌──────────┐
│         │              │          │
│ GPIO 21 ├──────────────┤ CLK      │
│ GPIO 22 ├──────────────┤ DIO      │
│ 3.3V/5V ├──────────────┤ VCC      │
│ GND     ├──────────────┤ GND      │
│         │              │          │
└─────────┘              └──────────┘
```

---

## Step 2: Compile and Upload

### Arduino IDE
1. Open `Clock3.ino`
2. Select **Tools > Board > ESP32 Dev Module** (or your specific ESP32 board)
3. Select **Tools > Port** and choose your ESP32's serial port
4. Click **Upload** (right arrow button)
5. Wait for "Done uploading" message

### PlatformIO
1. Open the Clock3 folder in VS Code
2. Click the **Upload** button (→) in the PlatformIO toolbar

**Expected Upload Output:**
```
Compiling...
Linking...
Building .bin
Writing at 0x00010000... (100%)
Hard resetting via RTS pin...
```

---

## Step 3: Test the Display

### What Should Happen

**On the Display:**
- All 4 digits should light up
- All segments (including decimal points) should be illuminated
- Pattern should look like: `8888` with all segments on
- Brightness should be at mid-level (configured as 4 out of 7)

**Visual Reference:**
```
┌───┐ ┌───┐   ┌───┐ ┌───┐
│ 8 │ │ 8 │ : │ 8 │ │ 8 │
└───┘ └───┘   └───┘ └───┘
 ALL   ALL     ALL   ALL
SEGS  SEGS    SEGS  SEGS
 ON    ON      ON    ON
```

---

## Step 4: Monitor Serial Output

### Open Serial Monitor

**Arduino IDE:**
1. Click **Tools > Serial Monitor**
2. Set baud rate to **115200**

**PlatformIO:**
1. Click **Serial Monitor** button (plug icon) in toolbar
2. Baud rate should already be set to 115200

### Expected Serial Output

```
========================================
ESP32 NTP Clock with TM1637 Display
========================================

Initializing...

Initializing TM1637 display... CLK Pin: 21, DIO Pin: 22
Display brightness set to: 4
Displaying test pattern: 8888 (all segments on)
Display initialization complete!

If all segments are lit, hardware is working correctly.
Test pattern will remain on display.

Setup complete

```

---

## Step 5: Verify Success

### Success Criteria ✓

Mark these as complete when verified:

- [ ] Code compiles without errors
- [ ] Code uploads to ESP32 successfully
- [ ] Serial Monitor shows initialization messages
- [ ] All 4 digits on display are lit
- [ ] All segments (A-G + DP) are visible
- [ ] Display brightness is appropriate (not too dim or bright)
- [ ] No flickering or instability
- [ ] Pattern remains on display continuously

---

## Troubleshooting

### Display Not Working (All Segments Off)

**Check Wiring:**
- Verify CLK is connected to GPIO 21
- Verify DIO is connected to GPIO 22
- Check VCC is connected to 3.3V or 5V
- Ensure GND is properly connected

**Check Power:**
- Try switching between 3.3V and 5V for VCC
- Ensure ESP32 is properly powered via USB
- Check for loose connections

**Check Code:**
- Verify Serial Monitor shows "Display initialization complete!"
- Check that config.h has correct pin definitions
- Ensure TM1637 library is installed

### Some Segments Not Lit

**Possible Hardware Issue:**
- The display module may be defective
- Try a different TM1637 module if available
- Check for damaged segments on the display

### Display Very Dim

**Adjust Brightness:**
1. Open `config.h`
2. Change `DISPLAY_BRIGHTNESS` from 4 to 7 (brightest)
3. Re-upload code

### Serial Monitor Shows Garbage Characters

**Wrong Baud Rate:**
- Make sure Serial Monitor is set to **115200 baud**
- Close and reopen Serial Monitor

### Upload Fails

**Common Solutions:**
- Press and hold BOOT button on ESP32 during upload
- Try different USB cable (some are power-only)
- Check that correct port is selected
- Close other programs using the serial port
- Try different USB port on computer

### Display Shows Random Patterns

**Wiring Issue:**
- DIO and CLK pins may be swapped
- Try swapping the CLK and DIO connections
- Check for loose wires

---

## Adjusting Brightness (Optional)

If the display is too bright or too dim, you can adjust it:

1. Open `config.h`
2. Find this line:
   ```cpp
   #define DISPLAY_BRIGHTNESS 4
   ```
3. Change the value:
   - 0 = dimmest
   - 4 = medium (default)
   - 7 = brightest
4. Save and re-upload

---

## Next Steps

Once Task 5 is verified successful:

✓ **Task 5 Complete!** Display initialization works.

**Ready for Tasks 6-9:** Implement custom display patterns:
- Task 6: Attention pattern (-!!-)
- Task 7: Scanning animation
- Task 8: Smiley face (-^^-)
- Task 9: Frowny face (-vv-)

These tasks can be done in parallel once Task 5 is complete.

---

## Reference: Segment Mapping

For understanding how `0xff` lights all segments:

```
Standard 7-Segment Layout:
     A
   ┌───┐
  F│   │B
   ├─G─┤
  E│   │C
   └───┘DP
     D

Bit Mapping (TM1637):
Bit 0 (0x01) = Segment A (top)
Bit 1 (0x02) = Segment B (top right)
Bit 2 (0x04) = Segment C (bottom right)
Bit 3 (0x08) = Segment D (bottom)
Bit 4 (0x10) = Segment E (bottom left)
Bit 5 (0x20) = Segment F (top left)
Bit 6 (0x40) = Segment G (middle)
Bit 7 (0x80) = Decimal Point (DP)

0xFF = 11111111 binary = All segments ON
```

---

## Support

If you continue to have issues:
1. Review the troubleshooting section above
2. Check wiring carefully with a multimeter
3. Try the test code from the [TM1637 library examples](https://github.com/avishorp/TM1637/tree/master/examples)
4. Search for your specific error in [library issues](https://github.com/avishorp/TM1637/issues)

---

**Good luck with your testing!** 🚀
