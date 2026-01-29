# ESP32 NTP Clock with TM1637 Display - Technical Specification

## Project Overview
An ESP32-based network time clock that displays the current time on a TM1637 4-digit 7-segment display, synchronized via NTP with automatic timezone and DST adjustments.

---

## Hardware Requirements

### Components
- **ESP32 Development Board** (any standard dev board)
- **TM1637 4-Digit 7-Segment Display Module**

### Wiring Connections
| TM1637 Pin | ESP32 GPIO |
|------------|------------|
| CLK        | GPIO 21    |
| DIO        | GPIO 22    |
| VCC        | 3.3V/5V    |
| GND        | GND        |

---

## Software Requirements

### Development Environment
- Arduino IDE or PlatformIO
- ESP32 board support package

### Required Libraries (well-maintained versions)
- **TM1637 display library** (developer's choice)
- **WiFiManager library** for WiFi configuration
- **NTP client library** for time synchronization
- **Timezone library** for DST handling
- ESP32 built-in WiFi

### Serial Communication
- **Baud Rate:** 115200
- **Debug Output:** Enabled for troubleshooting (connection status, NTP sync info, errors)

---

## Configuration (config.h)

Create a `config.h` file with the following configurable parameters:

```cpp
// Timezone Configuration
#define TIMEZONE "America/New_York"  // IANA timezone string
#define AUTO_DST true                // Automatic daylight saving time adjustment

// Time Display Format
#define TIME_FORMAT_24H false        // false = 12-hour format, true = 24-hour format
#define LEADING_ZERO false           // false = no leading zero (9:05), true = leading zero (09:05)

// NTP Configuration
#define NTP_SERVER_PRIMARY "pool.ntp.org"    // Primary NTP server
#define NTP_SERVER_SECONDARY "time.nist.gov" // Secondary NTP server (optional)
#define NTP_UPDATE_INTERVAL 3600000          // Update every hour (in milliseconds)

// Display Configuration
#define DISPLAY_BRIGHTNESS 4         // 0-7, where 0=dimmest, 7=brightest

// Reconnection Settings
#define RECONNECT_INTERVAL 60000     // Try reconnecting every 60 seconds (in milliseconds)
#define RECONNECT_ANIMATION_DURATION 5000  // Show scanning animation for 5 seconds during reconnect
#define MAX_RECONNECT_ATTEMPTS 5     // Give up after 5 failed reconnection attempts

// GPIO Pin Assignments (fixed, but documented here)
#define CLK_PIN 21
#define DIO_PIN 22
```

---

## Functional Requirements

### 1. Startup Sequence

#### 1.1 WiFi Connection
- On startup, attempt to connect to previously saved WiFi network
- **If no saved network exists:** Launch WiFiManager configuration portal
  - **Display behavior:** Show attention pattern `-!!-` to indicate user action required
  - User connects to ESP32 AP (named "ESP32-Clock" or similar) and configures WiFi credentials via web interface
  - Credentials are saved for future use
  - Once credentials are submitted, transition to scanning animation
- **Display behavior during connection:** Show scanning animation (horizontal dash moving across all 4 digits, back and forth)

#### 1.2 NTP Time Synchronization
- After successful WiFi connection, retrieve current time from NTP server
- Use primary NTP server, fall back to secondary if primary fails
- **Display behavior during NTP sync:** Continue showing scanning animation
- Device waits for first successful NTP sync before displaying time

#### 1.3 Success Indication
- Upon successful WiFi connection AND NTP sync:
  - Display smiley face pattern: `-^^-` (see visual reference)
  - Show for 2 seconds
  - Transition to displaying current time

### 2. Normal Operation - Time Display

#### 2.1 Time Format
- **12-hour mode** (when `TIME_FORMAT_24H = false`):
  - Display hours 1-12
  - No leading zero on hours (e.g., `9:05`, not `09:05`)
  - AM/PM indication using colon dots:
    - **AM:** Top dot illuminated
    - **PM:** Bottom dot illuminated
  
- **24-hour mode** (when `TIME_FORMAT_24H = true`):
  - Display hours 0-23
  - No leading zero on hours (e.g., `9:05`, not `09:05`)
  - Both colon dots blink together

#### 2.2 Colon Behavior
- Colon blinks at 1 Hz (once per second)
- Blinking synchronized with actual seconds:
  - ON during even seconds (0, 2, 4, ...)
  - OFF during odd seconds (1, 3, 5, ...)

#### 2.3 Time Updates
- Display updates every second based on ESP32 internal clock
- NTP resynchronization occurs every hour (3600000ms) in background
- Time automatically adjusts for timezone and DST based on config

### 3. Error Handling

#### 3.1 Initial Connection Failure
If WiFi connection or initial NTP sync fails:
1. Display frowny face pattern: `-vv-` (see visual reference)
2. Wait 1 minute
3. Show scanning animation for 5 seconds while attempting reconnection
4. If successful: Show smiley face for 2 seconds, then display time
5. If failed: Return to frowny face and repeat
6. After 5 failed attempts: Remain in frowny face state (no more retries)

#### 3.2 Connection Loss During Operation
If WiFi connection is lost after clock is successfully running:
1. Display frowny face pattern: `-vv-`
2. Follow same reconnection logic as initial failure (see 3.1)
3. Internal clock continues running during disconnection
4. Upon successful reconnection: Resync with NTP, show smiley, resume display

#### 3.3 Permanent Failure State
After 5 failed reconnection attempts:
- Continue displaying frowny face
- Stop attempting reconnections
- Continue running normally (no sleep mode)
- Requires manual reset/power cycle to retry

### 4. Visual Patterns Reference

#### Attention Pattern (User Action Required)
```
Display Pattern: -!!-
Digit 1: - (middle horizontal segment G)
Digit 2: ! (segments B, C, D - vertical line with dot below)
Digit 3: ! (segments B, C, D - vertical line with dot below)
Digit 4: - (middle horizontal segment G)
Use Case: WiFiManager configuration portal is active, waiting for user to configure WiFi
```

#### Scanning Animation (Connecting)
- Horizontal dash (`-`) moves across all 4 digits
- Pattern: Digit1 → Digit2 → Digit3 → Digit4 → Digit3 → Digit2 → (repeat)
- Animation speed: ~150ms per frame
- Uses middle horizontal segment (segment G) of each digit

#### Smiley Face (Success)
```
Display Pattern: -^^-
Digit 1: - (middle horizontal segment G)
Digit 2: ^ (segments A, F, B - top segments forming upward angle)
Digit 3: ^ (segments A, F, B - top segments forming upward angle)
Digit 4: - (middle horizontal segment G)
```

#### Frowny Face (Error)
```
Display Pattern: -vv-
Digit 1: - (middle horizontal segment G)
Digit 2: v (segments D, E, C - bottom segments forming downward angle)
Digit 3: v (segments D, E, C - bottom segments forming downward angle)
Digit 4: - (middle horizontal segment G)
```

---

## Code Structure Requirements

### File Organization
```
project/
├── main.ino or main.cpp
├── config.h
├── README.md
└── (library dependencies specified in platformio.ini or Arduino library manager)
```

### Code Quality
- Clean, readable code with standard formatting
- Basic inline comments for complex logic
- Function headers with brief descriptions
- Modular design (separate functions for display patterns, WiFi handling, NTP sync, etc.)

### README Contents
- Hardware wiring diagram/instructions
- Required library installation instructions  
- Configuration instructions (editing config.h)
- Upload and usage instructions
- Troubleshooting tips

---

## State Machine Overview

```
[POWER ON]
    ↓
[WiFiManager Check] → No saved network → [Config Portal: Show -!!-]
    ↓ (saved network exists)                    ↓ (user configures)
[Connect to WiFi] ←─────────────────────────────┘
    ↓ (show scanning animation)
[NTP Sync]
    ↓
[Success?] ─No→ [Show Frowny] → [Wait 60s] → [Retry (max 5)]
    ↓ Yes                             ↓
[Show Smiley 2s]                [Give up after 5 attempts]
    ↓
[Display Time] ←──────────────┐
    ↓                         │
[Connection OK?] ─No→ [Show Frowny] → [Reconnect logic]
    ↓ Yes                              ↓ (success)
[Continue] ───────────────────────────┘
```

---

## Testing Checklist

### Basic Functionality
- [ ] Display shows attention pattern (-!!-) when WiFiManager portal is active
- [ ] WiFiManager portal launches when no credentials saved
- [ ] Display transitions to scanning animation after WiFi credentials are submitted
- [ ] Successful connection shows smiley face for 2 seconds
- [ ] Time displays correctly after sync
- [ ] Colon blinks in sync with seconds
- [ ] Time updates every second

### Time Format Testing
- [ ] 12-hour format displays correctly (no leading zero)
- [ ] 24-hour format displays correctly (no leading zero)
- [ ] AM indicator (top dot) works in 12-hour mode
- [ ] PM indicator (bottom dot) works in 12-hour mode

### Error Handling
- [ ] Frowny face displays on initial connection failure
- [ ] Reconnection attempts happen every 60 seconds
- [ ] Scanning animation shows during reconnect attempts (5 seconds)
- [ ] Device gives up after 5 failed attempts
- [ ] Frowny face displays when connection lost during operation

### NTP and Timezone
- [ ] Hourly NTP resync occurs in background
- [ ] Timezone offset applied correctly
- [ ] DST adjustment works when enabled
- [ ] Time remains accurate over extended periods

### Configuration
- [ ] Brightness setting applies correctly
- [ ] Timezone changes work as expected
- [ ] NTP server configuration works
- [ ] All config.h parameters function properly

---

## Development Notes

### Library Selection Criteria
- Choose actively maintained libraries (updated within last year)
- Prefer libraries with good documentation
- Verify ESP32 compatibility
- Check for examples that match our use case

### Power Considerations
- No sleep modes required
- Display remains on continuously
- Normal ESP32 power consumption acceptable

### Future Enhancement Possibilities (Not Required)
- OTA firmware updates
- Web interface for configuration
- Multiple timezone support
- Temperature display mode
- Alarm functionality

---

## Visual Reference
See accompanying HTML file (`tm1637-animation-demo.html`) for interactive demonstrations of:
- Attention pattern (user action required)
- Scanning animation pattern
- Smiley face success pattern
- Frowny face error pattern

---

**Specification Version:** 1.0  
**Date:** January 28, 2026  
**Target Platform:** ESP32 (any standard dev board)  
**Display:** TM1637 4-digit 7-segment display
