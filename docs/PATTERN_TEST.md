# Phase 1 Pattern Testing Guide

## Overview

This test routine verifies that all four display patterns work correctly and match the specification. The code continuously cycles through all patterns, allowing you to visually compare them with the HTML demo.

---

## Test Cycle Sequence

The display will automatically cycle through these patterns in order:

1. **Attention Pattern** (`-!!-`) - 2 seconds
2. **Scanning Animation** (moving dash) - ~1 second (7 frames)
3. **Smiley Face** (`-^^-`) - 2 seconds
4. **Frowny Face** (`-vv-`) - 2 seconds
5. Brief pause, then repeat

**Total Cycle Time:** ~8 seconds

---

## Running the Test

### Step 1: Upload Test Code

1. Make sure your hardware is connected:
   - CLK → GPIO 21
   - DIO → GPIO 22
   - VCC → 3.3V or 5V
   - GND → GND

2. Open `Clock3.ino` in Arduino IDE or PlatformIO

3. Upload to your ESP32

### Step 2: Open Serial Monitor

**Arduino IDE:**
- Tools > Serial Monitor
- Set baud rate to **115200**

**PlatformIO:**
- Click Serial Monitor button (plug icon)

### Expected Serial Output

```
========================================
ESP32 NTP Clock with TM1637 Display
========================================

Initializing...

Initializing TM1637 display... CLK Pin: 21, DIO Pin: 22
Display brightness set to: 4
Display initialization complete!

========================================
PHASE 1 PATTERN TEST MODE
========================================
This test cycles through all 4 patterns:
  1. Attention Pattern (-!!-)
  2. Scanning Animation (moving dash)
  3. Smiley Face (-^^-)
  4. Frowny Face (-vv-)

Compare with tm1637-animation-demo.html
Verify each pattern displays correctly.
========================================

Setup complete - starting pattern test...

>>> Displaying: Attention Pattern (-!!-)
    Usage: WiFiManager portal active

>>> Displaying: Scanning Animation
    Usage: Connecting to WiFi/NTP
    Running 7 frames (~1 second)...

>>> Displaying: Smiley Face (-^^-)
    Usage: Successful connection

>>> Displaying: Frowny Face (-vv-)
    Usage: Connection failure/error

========================================
Pattern cycle complete. Repeating...
========================================
```

---

## Verification Checklist

### Open HTML Demo for Comparison

1. Open `tm1637-animation-demo.html` in your web browser
2. Keep it visible alongside your hardware display
3. Use the buttons to view each pattern on the HTML demo
4. Compare with your hardware during each phase of the test cycle

### Pattern 1: Attention Pattern (-!!-)

**Display Duration:** 2 seconds

**What to Check:**
- [ ] Left digit shows horizontal dash (-)
- [ ] Second digit shows exclamation mark (!)
  - [ ] Vertical line (segments B + C)
  - [ ] Dot at bottom (segment D)
- [ ] Third digit shows exclamation mark (!)
  - [ ] Vertical line (segments B + C)
  - [ ] Dot at bottom (segment D)
- [ ] Right digit shows horizontal dash (-)
- [ ] Pattern matches HTML demo "Show Attention Pattern" button
- [ ] Display is stable (no flickering)

**Expected Visual:**
```
┌───┐ ┌───┐   ┌───┐ ┌───┐
│ - │ │ ! │ : │ ! │ │ - │
│   │ │ | │   │ | │ │   │
│   │ │ · │   │ · │ │   │
└───┘ └───┘   └───┘ └───┘
```

---

### Pattern 2: Scanning Animation

**Display Duration:** ~1 second (7 frames at 150ms each)

**What to Check:**
- [ ] Horizontal dash (-) appears on left digit first
- [ ] Dash moves smoothly to second digit
- [ ] Dash moves to third digit
- [ ] Dash moves to fourth digit (rightmost)
- [ ] Dash moves back to third digit
- [ ] Dash moves back to second digit
- [ ] Animation loops smoothly (no jumps)
- [ ] Timing is approximately 150ms per frame
- [ ] Pattern matches HTML demo "Back to Animation" button
- [ ] Only one dash is visible at a time

**Expected Animation Sequence:**
```
Frame 0: -  :      (dash on digit 1)
Frame 1:  - :      (dash on digit 2)
Frame 2:   -:      (dash on digit 3)
Frame 3:    :-     (dash on digit 4)
Frame 4:   -:      (dash on digit 3)
Frame 5:  - :      (dash on digit 2)
(repeat)
```

---

### Pattern 3: Smiley Face (-^^-)

**Display Duration:** 2 seconds

**What to Check:**
- [ ] Left digit shows horizontal dash (-)
- [ ] Second digit shows upward angle (^)
  - [ ] Top horizontal segment (A)
  - [ ] Top-left vertical segment (F)
  - [ ] Top-right vertical segment (B)
  - [ ] Forms upward peak shape
- [ ] Third digit shows upward angle (^)
  - [ ] Same segments as second digit
- [ ] Right digit shows horizontal dash (-)
- [ ] Pattern matches HTML demo "Show Success Smiley" button
- [ ] Overall appearance resembles happy face
- [ ] Display is stable (no flickering)

**Expected Visual:**
```
┌───┐ ┌───┐   ┌───┐ ┌───┐
│ - │ │ ‾\│ : │/‾ │ │ - │
│   │ │  /│   │\  │ │   │
│   │ │   │   │   │ │   │
└───┘ └───┘   └───┘ └───┘
```

---

### Pattern 4: Frowny Face (-vv-)

**Display Duration:** 2 seconds

**What to Check:**
- [ ] Left digit shows horizontal dash (-)
- [ ] Second digit shows downward angle (v)
  - [ ] Bottom horizontal segment (D)
  - [ ] Bottom-left vertical segment (E)
  - [ ] Bottom-right vertical segment (C)
  - [ ] Forms downward valley shape
- [ ] Third digit shows downward angle (v)
  - [ ] Same segments as second digit
- [ ] Right digit shows horizontal dash (-)
- [ ] Pattern matches HTML demo "Show Error Frowny" button
- [ ] Overall appearance resembles sad face
- [ ] Display is stable (no flickering)

**Expected Visual:**
```
┌───┐ ┌───┐   ┌───┐ ┌───┐
│ - │ │   │ : │   │ │ - │
│   │ │  \│   │/  │ │   │
│   │ │ _/│   │\_ │ │   │
└───┘ └───┘   └───┘ └───┘
```

---

## Common Issues and Solutions

### Patterns Don't Match HTML Demo

**Segment Values May Be Wrong:**
1. Check segment bit calculations in code comments
2. Verify segment mapping matches your TM1637 module
3. Some clone modules may have different segment mappings

**Try This:**
- Note which segments appear incorrect
- Adjust the hex values in the function
- Refer to DISPLAY_PATTERNS.md for calculations

### Animation Is Too Fast or Too Slow

**Adjust Frame Timing:**
1. Open `Clock3.ino`
2. Find this line in `loop()`:
   ```cpp
   delay(150);  // 150ms per frame
   ```
3. Change 150 to a different value:
   - Slower: 200ms
   - Faster: 100ms
4. Re-upload

### Some Segments Don't Light Up

**Hardware Issue:**
- Display module may have defective segments
- Try different TM1637 module if available
- Check if the issue is consistent across all patterns

**Connection Issue:**
- Check CLK and DIO wire connections
- Ensure wires are firmly seated
- Try different GPIO pins (update config.h)

### Display Flickers

**Power Issue:**
- Try powering VCC from 5V instead of 3.3V (or vice versa)
- Check that USB power supply is adequate
- Use powered USB hub if needed

**Software Issue:**
- Delays may be too short
- Add small delay in scanning animation loop

### Pattern Cycle Stops

**ESP32 Crashed:**
- Check Serial Monitor for error messages
- Verify library is properly installed
- Try resetting ESP32 (press RESET button)

---

## Testing Completion Criteria

Mark Phase 1 as **COMPLETE** when all of these are verified:

### Display Functionality
- [ ] All 4 patterns display correctly
- [ ] Patterns match HTML demo exactly
- [ ] Scanning animation is smooth and continuous
- [ ] No flickering or instability
- [ ] Brightness is appropriate (level 4)
- [ ] Transitions between patterns are clean

### Code Quality
- [ ] Code compiles without errors or warnings
- [ ] Serial output shows all expected messages
- [ ] Pattern timing matches specification:
  - [ ] Attention: 2 seconds
  - [ ] Scanning: 150ms per frame
  - [ ] Smiley: 2 seconds
  - [ ] Frowny: 2 seconds

### Documentation
- [ ] All functions have header comments
- [ ] Segment calculations are documented
- [ ] config.h contains all Phase 1 parameters
- [ ] Code is well-organized and readable

---

## What's Next After Phase 1?

Once all patterns are verified, you're ready for **Phase 2: WiFi Connection with WiFiManager**

Phase 2 will:
- Integrate WiFiManager library
- Implement WiFi configuration portal
- Use attention pattern (-!!-) when portal is active
- Use scanning animation during connection
- Use smiley/frowny for success/failure feedback

---

## Disabling Test Mode

When Phase 1 testing is complete and you move to Phase 2, the test loop will be replaced with the actual clock functionality. The pattern functions will remain and be called by the state machine logic.

**To temporarily disable test mode:**
1. Comment out the loop() function code
2. Replace with simple delay
3. Patterns can still be called individually from setup()

---

## Photo/Video Documentation (Optional)

Consider documenting your working display:
- Take photos of each pattern
- Record short video of complete cycle
- Compare side-by-side with HTML demo
- Useful for troubleshooting later phases

---

## Success! 🎉

When all patterns work correctly, Phase 1 is complete!

You have successfully:
- ✅ Set up Arduino/PlatformIO project
- ✅ Installed and configured TM1637 library
- ✅ Created configuration system (config.h)
- ✅ Initialized display hardware
- ✅ Implemented all 4 custom patterns
- ✅ Verified patterns match specification

**Phase 1 Status: READY FOR PHASE 2** 🚀
