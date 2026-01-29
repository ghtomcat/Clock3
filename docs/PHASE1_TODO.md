# Phase 1: Basic Display Control - Todo List

## Overview
This phase focuses on getting the TM1637 display working with all required visual patterns. By the end of this phase, you should be able to display the attention pattern, scanning animation, smiley face, and frowny face.

---

## Task Dependencies

```
Task 1: Set up Arduino project structure
  ├─> Task 2: Research and select TM1637 library
  │     └─> Task 3: Install TM1637 library
  │           └─> Task 5: Initialize TM1637 display ─┐
  └─> Task 4: Create config.h file ─────────────────┘
                                                      │
                    ┌─────────────────────────────────┘
                    │
                    ├─> Task 6: Implement displayAttention() ──┐
                    ├─> Task 7: Implement displayScanningAnimation() ─┤
                    ├─> Task 8: Implement displaySmiley() ────────────┤
                    └─> Task 9: Implement displayFrowny() ────────────┤
                                                                       │
                         ┌─────────────────────────────────────────────┘
                         │
                         └─> Task 10: Test and verify all display patterns
```

---

## Tasks

### Task 1: Set up Arduino project structure
**Status:** Pending
**Blocked by:** None

**Description:**
Create the basic Arduino project structure for the ESP32 clock.

**Steps:**
- [ ] Create main .ino file (Clock3.ino or similar)
- [ ] Set up project folder structure
- [ ] Verify ESP32 board support is installed in Arduino IDE/PlatformIO
- [ ] Configure Serial output at 115200 baud rate in setup()
- [ ] Test that project compiles without errors

**Success Criteria:**
Empty project compiles successfully and can be uploaded to ESP32.

---

### Task 2: Research and select TM1637 library
**Status:** Pending
**Blocked by:** Task 1

**Description:**
Research available TM1637 libraries and select the best one for this project.

**Steps:**
- [ ] Search Arduino Library Manager or PlatformIO registry for TM1637 libraries
- [ ] Check library maintenance status (updated within last year)
- [ ] Verify ESP32 compatibility
- [ ] Review documentation quality and examples
- [ ] Ensure library supports direct segment control for custom patterns
- [ ] Document the chosen library name, author, and version

**Recommendation:**
TM1637Display by Avishay Orpaz is a well-maintained option with good ESP32 support.

**Success Criteria:**
Library is selected and documented, supports all required features.

---

### Task 3: Install TM1637 library
**Status:** Pending
**Blocked by:** Task 2

**Description:**
Install the selected TM1637 library into the Arduino/PlatformIO environment.

**Steps:**
- [ ] Install library via Arduino Library Manager or PlatformIO library manager
- [ ] Add library include statement to main .ino file (e.g., `#include <TM1637Display.h>`)
- [ ] Add config.h include statement
- [ ] Compile project to verify library is found
- [ ] Check for any additional dependencies and install if needed

**Success Criteria:**
Project compiles with library included, no errors.

---

### Task 4: Create config.h file
**Status:** Pending
**Blocked by:** Task 1

**Description:**
Create the initial configuration header file with GPIO pin definitions.

**Steps:**
- [ ] Create config.h file in project directory
- [ ] Add header guards (`#ifndef CONFIG_H`, `#define CONFIG_H`, `#endif`)
- [ ] Define CLK_PIN = 21
- [ ] Define DIO_PIN = 22
- [ ] Define DISPLAY_BRIGHTNESS = 4 (range 0-7, where 0=dimmest, 7=brightest)
- [ ] Add comments explaining each parameter
- [ ] Add note that additional config will be added in later phases

**Example:**
```cpp
#ifndef CONFIG_H
#define CONFIG_H

// GPIO Pin Assignments
#define CLK_PIN 21  // TM1637 CLK pin
#define DIO_PIN 22  // TM1637 DIO pin

// Display Configuration
#define DISPLAY_BRIGHTNESS 4  // Brightness level (0-7)

// Additional configuration will be added in later phases

#endif
```

**Success Criteria:**
config.h exists and can be included in main .ino file.

---

### Task 5: Initialize TM1637 display and test basic output
**Status:** Pending
**Blocked by:** Task 3, Task 4

**Description:**
Initialize the TM1637 display object and verify basic hardware functionality.

**Steps:**
- [ ] Create TM1637 display object using CLK_PIN and DIO_PIN from config.h
- [ ] Initialize display in setup() function
- [ ] Set brightness level from DISPLAY_BRIGHTNESS config
- [ ] Display test pattern "8888" (all segments lit) to verify hardware connection
- [ ] Add Serial.println() debug output to confirm initialization steps
- [ ] Upload to ESP32 hardware and test
- [ ] Verify all 4 digits display correctly

**Example:**
```cpp
#include <TM1637Display.h>
#include "config.h"

TM1637Display display(CLK_PIN, DIO_PIN);

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing TM1637 display...");

  display.setBrightness(DISPLAY_BRIGHTNESS);

  // Test pattern: all segments on
  uint8_t data[] = {0xff, 0xff, 0xff, 0xff};
  display.setSegments(data);

  Serial.println("Display initialized");
}
```

**Success Criteria:**
All segments of all 4 digits light up correctly on hardware.

---

### Task 6: Implement displayAttention() function
**Status:** Pending
**Blocked by:** Task 5

**Description:**
Create displayAttention() function to show -!!- pattern indicating user action required.

**Steps:**
- [ ] Create function signature: `void displayAttention()`
- [ ] Define segment pattern for each digit:
  - Digit 1: segment G (0x40)
  - Digit 2: segments B, C, D (0x06 | 0x08 | 0x10 = 0x1E)
  - Digit 3: segments B, C, D (0x1E)
  - Digit 4: segment G (0x40)
- [ ] Use display.setSegments() to show the pattern
- [ ] Add function header comment explaining purpose
- [ ] Test by calling from setup() temporarily
- [ ] Verify pattern matches specification

**Example:**
```cpp
/**
 * Display attention pattern: -!!-
 * Indicates WiFiManager portal is active and waiting for user to configure WiFi
 */
void displayAttention() {
  uint8_t segments[] = {
    0x40,  // Digit 1: - (segment G)
    0x1E,  // Digit 2: ! (segments B, C, D)
    0x1E,  // Digit 3: ! (segments B, C, D)
    0x40   // Digit 4: - (segment G)
  };
  display.setSegments(segments);
}
```

**Success Criteria:**
Display shows -!!- pattern correctly, matching HTML demo.

---

### Task 7: Implement displayScanningAnimation() function
**Status:** Pending
**Blocked by:** Task 5

**Description:**
Create displayScanningAnimation() function for showing connection progress.

**Steps:**
- [ ] Create static variable to track current animation frame (0-5)
- [ ] Create function signature: `void displayScanningAnimation()`
- [ ] Define 6-frame animation array:
  - Frame 0: segment G on digit 1
  - Frame 1: segment G on digit 2
  - Frame 2: segment G on digit 3
  - Frame 3: segment G on digit 4
  - Frame 4: segment G on digit 3
  - Frame 5: segment G on digit 2
- [ ] Advance frame counter each call (wrap at 6)
- [ ] Add function header comment
- [ ] Test by calling in loop() with 150ms delay
- [ ] Verify smooth back-and-forth animation

**Example:**
```cpp
/**
 * Display scanning animation: horizontal dash moving across digits
 * Call this function every 150ms to animate
 * Used during WiFi/NTP connection attempts
 */
void displayScanningAnimation() {
  static uint8_t frame = 0;
  static const uint8_t patterns[6][4] = {
    {0x40, 0x00, 0x00, 0x00},  // Frame 0: digit 1
    {0x00, 0x40, 0x00, 0x00},  // Frame 1: digit 2
    {0x00, 0x00, 0x40, 0x00},  // Frame 2: digit 3
    {0x00, 0x00, 0x00, 0x40},  // Frame 3: digit 4
    {0x00, 0x00, 0x40, 0x00},  // Frame 4: digit 3
    {0x00, 0x40, 0x00, 0x00}   // Frame 5: digit 2
  };

  display.setSegments(patterns[frame]);
  frame = (frame + 1) % 6;
}
```

**Success Criteria:**
Dash moves smoothly across display in back-and-forth pattern.

---

### Task 8: Implement displaySmiley() function
**Status:** Pending
**Blocked by:** Task 5

**Description:**
Create displaySmiley() function to show -^^- pattern for successful connection.

**Steps:**
- [ ] Create function signature: `void displaySmiley()`
- [ ] Define segment pattern for each digit:
  - Digit 1: segment G (0x40)
  - Digit 2: segments A, F, B (0x01 | 0x20 | 0x02 = 0x23)
  - Digit 3: segments A, F, B (0x23)
  - Digit 4: segment G (0x40)
- [ ] Use display.setSegments() to show the pattern
- [ ] Add function header comment
- [ ] Test by calling from setup() temporarily
- [ ] Verify pattern matches specification

**Example:**
```cpp
/**
 * Display smiley face pattern: -^^-
 * Indicates successful WiFi connection and NTP sync
 * Show for 2 seconds after successful connection
 */
void displaySmiley() {
  uint8_t segments[] = {
    0x40,  // Digit 1: - (segment G)
    0x23,  // Digit 2: ^ (segments A, F, B)
    0x23,  // Digit 3: ^ (segments A, F, B)
    0x40   // Digit 4: - (segment G)
  };
  display.setSegments(segments);
}
```

**Success Criteria:**
Display shows -^^- pattern correctly, matching HTML demo.

---

### Task 9: Implement displayFrowny() function
**Status:** Pending
**Blocked by:** Task 5

**Description:**
Create displayFrowny() function to show -vv- pattern for connection failure.

**Steps:**
- [ ] Create function signature: `void displayFrowny()`
- [ ] Define segment pattern for each digit:
  - Digit 1: segment G (0x40)
  - Digit 2: segments D, E, C (0x08 | 0x10 | 0x04 = 0x1C)
  - Digit 3: segments D, E, C (0x1C)
  - Digit 4: segment G (0x40)
- [ ] Use display.setSegments() to show the pattern
- [ ] Add function header comment
- [ ] Test by calling from setup() temporarily
- [ ] Verify pattern matches specification

**Example:**
```cpp
/**
 * Display frowny face pattern: -vv-
 * Indicates connection failure or error state
 * Display until reconnection attempt or permanent failure
 */
void displayFrowny() {
  uint8_t segments[] = {
    0x40,  // Digit 1: - (segment G)
    0x1C,  // Digit 2: v (segments D, E, C)
    0x1C,  // Digit 3: v (segments D, E, C)
    0x40   // Digit 4: - (segment G)
  };
  display.setSegments(segments);
}
```

**Success Criteria:**
Display shows -vv- pattern correctly, matching HTML demo.

---

### Task 10: Test and verify all display patterns
**Status:** Pending
**Blocked by:** Task 6, Task 7, Task 8, Task 9

**Description:**
Create comprehensive test routine to verify all display patterns work correctly.

**Steps:**
- [ ] Create test sequence in loop() that cycles through all patterns:
  1. Show attention pattern (-!!-) for 2 seconds
  2. Run scanning animation for ~1 second (6 frames at 150ms)
  3. Show smiley face (-^^-) for 2 seconds
  4. Show frowny face (-vv-) for 2 seconds
  5. Repeat cycle
- [ ] Add Serial output indicating which pattern is being displayed
- [ ] Open HTML demo (tm1637-animation-demo.html) for comparison
- [ ] Verify each pattern on hardware matches HTML demo exactly
- [ ] Check segment brightness and clarity
- [ ] Document any issues or needed adjustments
- [ ] Take photos/video of working patterns (optional but recommended)

**Example test loop:**
```cpp
void loop() {
  // Test attention pattern
  Serial.println("Showing attention pattern (-!!-)");
  displayAttention();
  delay(2000);

  // Test scanning animation
  Serial.println("Showing scanning animation");
  for (int i = 0; i < 7; i++) {  // About 1 second (7 frames * 150ms)
    displayScanningAnimation();
    delay(150);
  }

  // Test smiley
  Serial.println("Showing smiley face (-^^-)");
  displaySmiley();
  delay(2000);

  // Test frowny
  Serial.println("Showing frowny face (-vv-)");
  displayFrowny();
  delay(2000);
}
```

**Success Criteria:**
- All four patterns display correctly
- Patterns match HTML demo exactly
- Scanning animation is smooth and continuous
- No flickering or unexpected behavior
- Serial output confirms each pattern transition

---

## Phase 1 Completion Checklist

Once all tasks are complete, verify:

- [ ] Project compiles without errors or warnings
- [ ] TM1637 library is properly installed and integrated
- [ ] config.h contains all required pin and brightness definitions
- [ ] Display initializes correctly on power-up
- [ ] Attention pattern (-!!-) displays correctly
- [ ] Scanning animation moves smoothly across digits
- [ ] Smiley pattern (-^^-) displays correctly
- [ ] Frowny pattern (-vv-) displays correctly
- [ ] All patterns match the specification and HTML demo
- [ ] Code is modular with separate functions for each pattern
- [ ] Code includes comments and function headers
- [ ] Hardware connections are verified and stable

**When Phase 1 is complete, you're ready to move to Phase 2: WiFi Connection with WiFiManager**

---

## Notes

### TM1637 Segment Mapping Reference
```
Standard 7-segment display:
     A
   -----
  |     |
 F|  G  |B
  |-----|
  |     |
 E|     |C
  |-----| .
     D

Segment bit values:
A = 0x01
B = 0x02
C = 0x04
D = 0x08
E = 0x10
F = 0x20
G = 0x40
DP = 0x80 (decimal point/colon)
```

### Troubleshooting

**Display not working:**
- Check wiring: CLK to GPIO 21, DIO to GPIO 22, VCC to 3.3V/5V, GND to GND
- Verify ESP32 board is powered correctly
- Check Serial output for initialization messages
- Try different brightness levels

**Segments incorrect:**
- Verify segment bit values match your library's mapping
- Some libraries may use different bit orders
- Check library documentation or examples

**Animation not smooth:**
- Ensure delay(150) is used between animation frames
- Verify frame counter wraps correctly at frame 6
- Check that no other delays or blocking code interferes
