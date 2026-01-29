# TM1637 Library Selection - Research Summary

## Date: January 28, 2026

## Requirements

For this ESP32 NTP Clock project, the library must support:
- ✓ Direct segment control for custom patterns (-!!-, -^^-, -vv-, scanning animation)
- ✓ ESP32 compatibility
- ✓ Simple, straightforward API
- ✓ Active maintenance and documentation
- ✓ Ability to control individual segments via byte arrays

---

## Libraries Evaluated

### 1. TM1637Display by Avishay Orpaz ⭐ SELECTED
**Repository:** https://github.com/avishorp/TM1637

**Pros:**
- ✓ Has `setSegments()` function for direct segment control
- ✓ Simple, focused API - single class TM1637Display
- ✓ Wide adoption and extensive documentation
- ✓ ESP32 compatible (works with 3.3V and 5V)
- ✓ Current version: 1.2.0
- ✓ Example code shows custom patterns: `uint8_t data[] = { 0xff, 0xff, 0xff, 0xff }`
- ✓ Any two GPIO pins can be used for CLK and DIO
- ✓ Lightweight and easy to use

**Cons:**
- Limited to TM1637 only (not an issue for this project)

**Key Functions:**
```cpp
TM1637Display display(CLK_PIN, DIO_PIN);
display.setBrightness(0-7);
display.setSegments(uint8_t segments[], uint8_t length, uint8_t pos);
```

**Example Usage:**
```cpp
uint8_t data[] = { 0x40, 0x1E, 0x1E, 0x40 };  // Custom pattern
display.setSegments(data);
```

---

### 2. SevenSegmentTM1637 by bremme
**Repository:** https://github.com/bremme/arduino-tm1637

**Pros:**
- Extensive feature set
- Inherits Print class (can use print() and println())
- Uses LCDAPI 1.0
- Good for text and number display

**Cons:**
- More complex than needed for this project
- Heavier library with more overhead
- Print class functionality not required

---

### 3. AceSegment by bxparks
**Repository:** https://github.com/bxparks/AceSegment

**Pros:**
- Supports multiple controller chips (TM1637, TM1638, MAX7219, HT16K33, 74HC595)
- Extensible framework
- Well maintained (recent updates)

**Cons:**
- Overkill for single TM1637 project
- More complex API
- Additional abstraction layer not needed

---

### 4. TM16xx by maxint-rd
**Repository:** https://github.com/maxint-rd/TM16xx

**Pros:**
- Supports many TM16xx chips
- Recently updated (2025 support for TM1621/TM1622)
- Adafruit GFX support
- Button detection features

**Cons:**
- Multi-chip support adds unnecessary complexity
- Larger library size
- More features than required

---

### 5. TM1638plus by gavinlyonsrepo
**Repository:** https://github.com/gavinlyonsrepo/tm1638plus

**Pros:**
- Supports TM1638, TM1637, and MAX7219
- Version 2.2.0 added TM1637 support

**Cons:**
- Primary focus is TM1638
- TM1637 support was added later (potentially less mature)

---

## Decision: TM1637Display by Avishay Orpaz

### Rationale

The **TM1637Display library by Avishay Orpaz** is the best choice for this project because:

1. **Perfect Feature Match:** The `setSegments()` function provides exactly what we need for custom patterns
2. **Simplicity:** Single-purpose library with minimal complexity
3. **Proven Track Record:** Widely used in tutorials and projects with ESP32
4. **Direct Control:** Byte array approach allows precise segment control for our custom faces
5. **Documentation:** Well-documented with clear examples
6. **Community Support:** Large user base means better troubleshooting resources

### Segment Control Example

The library uses a byte array where each byte controls one digit's segments:

```cpp
// Segment bit mapping:
// A = 0x01, B = 0x02, C = 0x04, D = 0x08
// E = 0x10, F = 0x20, G = 0x40, DP = 0x80

// Our custom patterns:
uint8_t attention[] = {0x40, 0x1E, 0x1E, 0x40};  // -!!-
uint8_t smiley[]    = {0x40, 0x23, 0x23, 0x40};  // -^^-
uint8_t frowny[]    = {0x40, 0x1C, 0x1C, 0x40};  // -vv-
```

### Installation

**Arduino IDE:**
1. Open Library Manager (Sketch > Include Library > Manage Libraries)
2. Search for "TM1637"
3. Install "TM1637" by Avishay Orpaz

**PlatformIO:**
```ini
lib_deps =
    avishorp/TM1637@^1.2.0
```

---

## Sources

- [ESP32 - TM1637 4-Digit 7-Segment Display Tutorial](https://esp32io.com/tutorials/esp32-tm1637-4-digit-7-segment-display)
- [GitHub - avishorp/TM1637: Arduino library for TM1637 (LED Driver)](https://github.com/avishorp/TM1637)
- [TM1637Test Example Code](https://github.com/avishorp/TM1637/blob/master/examples/TM1637Test/TM1637Test.ino)
- [Interface ESP32 With TM1637 LED Driver - Makerguides](https://www.makerguides.com/a-guide-to-interface-esp32-with-tm1637-7-segment-led-driver/)
- [TM1637 Arduino Tutorial - Last Minute Engineers](https://lastminuteengineers.com/tm1637-arduino-tutorial/)
- [GitHub - bxparks/AceSegment](https://github.com/bxparks/AceSegment)
- [GitHub - maxint-rd/TM16xx](https://github.com/maxint-rd/TM16xx)

---

**Decision Final:** Use **TM1637Display by Avishay Orpaz** (version 1.2.0)
