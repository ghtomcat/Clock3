# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an ESP32-based NTP clock project that displays time on a TM1637 4-digit 7-segment display. The clock synchronizes via NTP with automatic timezone and DST adjustments, and uses WiFiManager for configuration.

## Hardware Configuration

**Target Platform:** ESP32 (any standard dev board)
**Display:** TM1637 4-digit 7-segment display module

**GPIO Pin Assignments:**
- CLK: GPIO 21
- DIO: GPIO 22
- Power: 3.3V or 5V

## Development Environment

This project is designed for Arduino IDE or PlatformIO with ESP32 board support.

**Serial Configuration:**
- Baud Rate: 115200
- Debug output should be enabled for connection status, NTP sync info, and errors

**Serial Monitoring:**

Use PlatformIO's built-in device monitor for reliable serial output:

```bash
# Monitor serial output (auto-detects port and baud rate from platformio.ini)
pio device monitor

# Upload and monitor in one command
pio run --target upload && pio device monitor

# Exit monitor: Ctrl+C
```

**Important Serial Output Notes:**
- Add `Serial.flush()` after groups of `Serial.println()` calls to prevent character dropping on USB serial
- The PlatformIO monitor auto-reconnects when the ESP32 resets
- Alternative tools: `screen /dev/cu.usbserial-XXXX 115200` (exit: Ctrl+A, K, Y)
- **Note for Claude Code**: `pio device monitor` requires an interactive TTY and cannot be run through Claude Code's non-interactive environment (returns `termios.error: (19, 'Operation not supported by device')`). Open a separate terminal window to view serial output.

**Required Libraries:**
- TM1637 display library (any well-maintained version)
- WiFiManager library (for WiFi configuration portal)
- NTP client library (for time synchronization)
- Timezone library (for DST handling)
- ESP32 built-in WiFi

## Code Architecture

### Configuration System

All user-configurable parameters must be in `config.h`:

**Timezone and Time Format:**
- `TIMEZONE`: IANA timezone string (e.g., "America/New_York")
- `AUTO_DST`: Boolean for automatic DST adjustment
- `TIME_FORMAT_24H`: false = 12-hour format, true = 24-hour format
- `LEADING_ZERO`: Whether to show leading zero on hours

**NTP Settings:**
- `NTP_SERVER_PRIMARY`: Primary NTP server (default: "pool.ntp.org")
- `NTP_SERVER_SECONDARY`: Fallback NTP server (default: "time.nist.gov")
- `NTP_UPDATE_INTERVAL`: Resync interval in milliseconds (default: 3600000 = 1 hour)

**Display Settings:**
- `DISPLAY_BRIGHTNESS`: 0-7 (where 0=dimmest, 7=brightest)

**Reconnection Settings:**
- `RECONNECT_INTERVAL`: Retry interval in milliseconds (default: 60000 = 1 minute)
- `RECONNECT_ANIMATION_DURATION`: Show scanning animation for 5 seconds during reconnect
- `MAX_RECONNECT_ATTEMPTS`: Give up after this many failed attempts (default: 5)

### State Machine Architecture

The clock operates in distinct states:

1. **Startup State**: Check for saved WiFi credentials, launch WiFiManager portal if none exist
2. **WiFiManager Portal State**: Display attention pattern (`-!!-`) while waiting for user to configure WiFi via portal
3. **Connecting State**: Show scanning animation while connecting to WiFi and syncing NTP
4. **Success State**: Display smiley face pattern (`-^^-`) for 2 seconds after successful connection
5. **Normal Operation State**: Display time with blinking colon
6. **Error State**: Display frowny face pattern (`-vv-`) on connection failure
7. **Reconnecting State**: Show scanning animation for 5 seconds while attempting reconnection
8. **Permanent Failure State**: After 5 failed reconnection attempts, remain in frowny face state

### Display Patterns

**Attention Pattern (`-!!-`) - WiFiManager Portal Active:**
- Digit 1: segment G (middle horizontal)
- Digit 2: segments B, C, D (exclamation mark - vertical line with dot)
- Digit 3: segments B, C, D (exclamation mark - vertical line with dot)
- Digit 4: segment G (middle horizontal)
- Indicates user action required - WiFi configuration portal is waiting for credentials

**Scanning Animation (used during connection/reconnection):**
- Horizontal dash moves across all 4 digits using middle segment (segment G)
- Pattern sequence: Digit1 → Digit2 → Digit3 → Digit4 → Digit3 → Digit2 → (repeat)
- Animation speed: ~150ms per frame

**Smiley Face Pattern (`-^^-`):**
- Digit 1: segment G (middle horizontal)
- Digit 2: segments A, F, B (top segments forming upward angle)
- Digit 3: segments A, F, B (top segments forming upward angle)
- Digit 4: segment G (middle horizontal)
- Display for 2 seconds after successful connection

**Frowny Face Pattern (`-vv-`):**
- Digit 1: segment G (middle horizontal)
- Digit 2: segments D, E, C (bottom segments forming downward angle)
- Digit 3: segments D, E, C (bottom segments forming downward angle)
- Digit 4: segment G (middle horizontal)
- Display when connection fails or is lost

### Time Display Requirements

**12-hour format (TIME_FORMAT_24H = false):**
- Display hours 1-12 with no leading zero (e.g., `9:05`, not `09:05`)
- AM/PM indication using colon dots:
  - AM: Top dot illuminated
  - PM: Bottom dot illuminated
- Colon blinks at 1 Hz synchronized with seconds (ON during even seconds, OFF during odd seconds)

**24-hour format (TIME_FORMAT_24H = true):**
- Display hours 0-23 with no leading zero
- Both colon dots blink together at 1 Hz

**Update Behavior:**
- Display updates every second based on ESP32 internal clock
- NTP resynchronization occurs every hour in background
- Time automatically adjusts for timezone and DST

### Error Handling and Reconnection Logic

**Initial Connection Failure:**
1. Display frowny face pattern
2. Wait 1 minute (RECONNECT_INTERVAL)
3. Show scanning animation for 5 seconds while reconnecting
4. If successful: Show smiley face for 2 seconds, then display time
5. If failed: Return to frowny face and repeat
6. After MAX_RECONNECT_ATTEMPTS (5) failed attempts: Remain in frowny face state (no more retries)

**Connection Loss During Operation:**
- Follow same reconnection logic as initial failure
- Internal clock continues running during disconnection
- Upon successful reconnection: Resync with NTP, show smiley, resume display

**Permanent Failure State:**
- Continue displaying frowny face
- Stop attempting reconnections
- No sleep mode (continue running normally)
- Requires manual reset/power cycle to retry

## Code Structure

The codebase should be organized into modular functions:
- Display pattern functions (scanning animation, smiley, frowny)
- WiFi handling functions
- NTP sync functions
- Time display and formatting functions
- State machine management
- Error handling and reconnection logic

## Visual Reference

The `tm1637-animation-demo.html` file provides interactive demonstrations of all display patterns. Open it in a browser to see the scanning animation, smiley face, and frowny face patterns in action.

## Key Implementation Notes

- WiFiManager handles credential storage automatically - no manual credential saving needed
- Device waits for first successful NTP sync before displaying time
- The internal clock continues running even when disconnected, but resyncs on reconnection
- No sleep modes are required - display remains on continuously
- Choose actively maintained libraries (updated within last year) with good ESP32 compatibility
