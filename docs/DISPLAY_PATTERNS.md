# Display Patterns Reference

## Overview

This document describes all four custom display patterns implemented for the ESP32 NTP Clock. Each pattern uses direct segment control to create custom visual feedback.

---

## Segment Bit Mapping

The TM1637 uses a byte (8 bits) to control each digit's segments:

```
7-Segment Display Layout:
     A (top)
   ┌───┐
  F│   │B (right sides)
   ├─G─┤ (middle)
  E│   │C (right sides)
   └───┘
     D (bottom)
    DP (decimal point)

Bit Values:
Bit 0 (0x01) = Segment A (top horizontal)
Bit 1 (0x02) = Segment B (top right vertical)
Bit 2 (0x04) = Segment C (bottom right vertical)
Bit 3 (0x08) = Segment D (bottom horizontal)
Bit 4 (0x10) = Segment E (bottom left vertical)
Bit 5 (0x20) = Segment F (top left vertical)
Bit 6 (0x40) = Segment G (middle horizontal)
Bit 7 (0x80) = DP (decimal point/colon)
```

---

## Pattern 1: Attention Pattern `-!!-`

**Purpose:** Indicates WiFiManager portal is active, waiting for user configuration

**When Used:** On first boot when no WiFi credentials are saved, or when configuration portal is manually triggered

**Visual:**
```
┌───┐ ┌───┐   ┌───┐ ┌───┐
│ - │ │ ! │ : │ ! │ │ - │
└───┘ └───┘   └───┘ └───┘
```

**Segment Mapping:**
- **Digit 1:** `0x40` = Segment G (horizontal dash)
- **Digit 2:** `0x0E` = Segments B + C + D (vertical line + dot for !)
- **Digit 3:** `0x0E` = Segments B + C + D (vertical line + dot for !)
- **Digit 4:** `0x40` = Segment G (horizontal dash)

**Code:**
```cpp
void displayAttention() {
  uint8_t segments[] = {0x40, 0x0E, 0x0E, 0x40};
  display.setSegments(segments);
}
```

**Calculation:**
```
Exclamation mark (!):
  B (0x02) + C (0x04) + D (0x08) = 0x0E
  Visual: | with dot below
         |
         |
         .
```

---

## Pattern 2: Scanning Animation

**Purpose:** Shows connection/synchronization in progress

**When Used:**
- During WiFi connection attempts
- During NTP synchronization
- During reconnection attempts (5 seconds)

**Visual:**
```
Frame 0:  -  :
Frame 1:   - :
Frame 2:    -:
Frame 3:    :-
Frame 4:    -:
Frame 5:   - :
(repeat)
```

**Animation Details:**
- 6 frames total
- Horizontal dash (segment G) moves across digits
- Pattern: Digit1 → Digit2 → Digit3 → Digit4 → Digit3 → Digit2 → (loop)
- Frame rate: 150ms per frame
- Total cycle: ~900ms (0.9 seconds)

**Segment Mapping:**
```
Frame 0: [0x40, 0x00, 0x00, 0x00]  // Dash on digit 1
Frame 1: [0x00, 0x40, 0x00, 0x00]  // Dash on digit 2
Frame 2: [0x00, 0x00, 0x40, 0x00]  // Dash on digit 3
Frame 3: [0x00, 0x00, 0x00, 0x40]  // Dash on digit 4
Frame 4: [0x00, 0x00, 0x40, 0x00]  // Dash on digit 3 (back)
Frame 5: [0x00, 0x40, 0x00, 0x00]  // Dash on digit 2 (back)
```

**Code:**
```cpp
void displayScanningAnimation() {
  static uint8_t frame = 0;
  static const uint8_t patterns[6][4] = {
    {0x40, 0x00, 0x00, 0x00},  // Frame 0
    {0x00, 0x40, 0x00, 0x00},  // Frame 1
    {0x00, 0x00, 0x40, 0x00},  // Frame 2
    {0x00, 0x00, 0x00, 0x40},  // Frame 3
    {0x00, 0x00, 0x40, 0x00},  // Frame 4
    {0x00, 0x40, 0x00, 0x00}   // Frame 5
  };
  display.setSegments(patterns[frame]);
  frame = (frame + 1) % 6;
}
```

**Usage:**
```cpp
// Call every 150ms
displayScanningAnimation();
delay(150);
```

---

## Pattern 3: Smiley Face `-^^-`

**Purpose:** Indicates successful connection and synchronization

**When Used:**
- After successful WiFi connection AND NTP sync
- After successful reconnection
- Display for 2 seconds, then transition to time display

**Visual:**
```
┌───┐ ┌───┐   ┌───┐ ┌───┐
│ - │ │ ^ │ : │ ^ │ │ - │
└───┘ └───┘   └───┘ └───┘
```

**Segment Mapping:**
- **Digit 1:** `0x40` = Segment G (horizontal dash)
- **Digit 2:** `0x23` = Segments A + F + B (upward angle ^)
- **Digit 3:** `0x23` = Segments A + F + B (upward angle ^)
- **Digit 4:** `0x40` = Segment G (horizontal dash)

**Code:**
```cpp
void displaySmiley() {
  uint8_t segments[] = {0x40, 0x23, 0x23, 0x40};
  display.setSegments(segments);
}
```

**Calculation:**
```
Upward angle (^):
  A (0x01) + F (0x20) + B (0x02) = 0x23
  Visual: ‾\  /‾ (top segments forming peak)
           \/
```

---

## Pattern 4: Frowny Face `-vv-`

**Purpose:** Indicates connection failure or error state

**When Used:**
- Initial WiFi connection failure
- Initial NTP sync failure
- Connection lost during operation
- Remains displayed during 60-second wait between reconnection attempts
- Permanent display after 5 failed reconnection attempts

**Visual:**
```
┌───┐ ┌───┐   ┌───┐ ┌───┐
│ - │ │ v │ : │ v │ │ - │
└───┘ └───┘   └───┘ └───┘
```

**Segment Mapping:**
- **Digit 1:** `0x40` = Segment G (horizontal dash)
- **Digit 2:** `0x1C` = Segments D + E + C (downward angle v)
- **Digit 3:** `0x1C` = Segments D + E + C (downward angle v)
- **Digit 4:** `0x40` = Segment G (horizontal dash)

**Code:**
```cpp
void displayFrowny() {
  uint8_t segments[] = {0x40, 0x1C, 0x1C, 0x40};
  display.setSegments(segments);
}
```

**Calculation:**
```
Downward angle (v):
  D (0x08) + E (0x10) + C (0x04) = 0x1C
  Visual: \  / (bottom segments forming valley)
           \/
          ‾‾
```

---

## Testing Each Pattern

To test patterns individually, temporarily modify the `setup()` function:

### Test Attention Pattern
```cpp
void setup() {
  Serial.begin(115200);
  display.setBrightness(DISPLAY_BRIGHTNESS);

  Serial.println("Testing attention pattern: -!!-");
  displayAttention();
  // Pattern will remain on display
}
```

### Test Scanning Animation
```cpp
void loop() {
  displayScanningAnimation();
  delay(150);  // 150ms between frames
}
```

### Test Smiley Pattern
```cpp
void setup() {
  Serial.begin(115200);
  display.setBrightness(DISPLAY_BRIGHTNESS);

  Serial.println("Testing smiley pattern: -^^-");
  displaySmiley();
  delay(2000);  // Show for 2 seconds as per spec
}
```

### Test Frowny Pattern
```cpp
void setup() {
  Serial.begin(115200);
  display.setBrightness(DISPLAY_BRIGHTNESS);

  Serial.println("Testing frowny pattern: -vv-");
  displayFrowny();
  // Pattern will remain on display
}
```

---

## Pattern Comparison with HTML Demo

Compare your hardware display with the interactive demo:

1. Open `tm1637-animation-demo.html` in a web browser
2. Click each button to see the patterns:
   - "Show Attention Pattern" → -!!-
   - "Back to Animation" → Scanning animation
   - "Show Success Smiley" → -^^-
   - "Show Error Frowny" → -vv-
3. Verify hardware matches the HTML demo exactly

---

## Quick Reference Table

| Pattern | Display | Hex Values | When Used |
|---------|---------|------------|-----------|
| **Attention** | `-!!-` | `0x40, 0x0E, 0x0E, 0x40` | WiFiManager portal active |
| **Scanning** | `----` (animated) | 6 frames, segment G moves | Connecting/syncing |
| **Smiley** | `-^^-` | `0x40, 0x23, 0x23, 0x40` | Success (2 sec) |
| **Frowny** | `-vv-` | `0x40, 0x1C, 0x1C, 0x40` | Error/failure |

---

## State Machine Flow

```
[Power On]
    ↓
[No WiFi Credentials?] ─Yes→ [Show -!!-] → [WiFiManager Portal]
    ↓ No                          ↓ (configured)
[Show Scanning Animation] ←───────┘
    ↓ (connecting)
[WiFi + NTP Sync]
    ↓
[Success?] ─No→ [Show -vv-] → [Wait 60s] → [Retry]
    ↓ Yes
[Show -^^- for 2 sec]
    ↓
[Display Time]
```

---

## Implementation Status

- ✅ **Task 6:** displayAttention() implemented
- ✅ **Task 7:** displayScanningAnimation() implemented
- ✅ **Task 8:** displaySmiley() implemented
- ✅ **Task 9:** displayFrowny() implemented
- ⏳ **Task 10:** Comprehensive testing (next)

---

## Next: Task 10

Create a test routine that cycles through all patterns to verify they match the specification and HTML demo.
