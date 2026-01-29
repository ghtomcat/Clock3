# ESP32 NTP Clock

A WiFi-enabled clock for ESP32 that displays time on a TM1637 4-digit 7-segment display. Features automatic NTP synchronization, timezone/DST support, and WiFiManager for easy configuration.

> **Development Status:** Currently implementing Phase 2 (WiFi Connection). See `docs/IMPLEMENTATION_PLAN.md` for the complete phased development plan.

## Features

- **NTP Time Synchronization**: Automatically syncs with internet time servers
- **Automatic Timezone & DST**: Configurable timezone with automatic daylight saving adjustments
- **WiFi Configuration Portal**: Easy setup via WiFiManager (no hardcoded credentials)
- **12/24 Hour Display**: Configurable time format with AM/PM indication
- **Visual Status Indicators**: Animated patterns show connection status
- **Auto-Reconnection**: Automatically attempts to reconnect if WiFi is lost
- **Smart Error Handling**: Clear visual feedback for all connection states

## Hardware Requirements

- **ESP32** development board (any standard variant)
- **TM1637** 4-digit 7-segment display module
- Jumper wires
- USB cable for programming

### Wiring

| TM1637 Pin | ESP32 GPIO |
|------------|------------|
| CLK        | GPIO 21    |
| DIO        | GPIO 22    |
| VCC        | 3.3V or 5V |
| GND        | GND        |

## Software Requirements

### PlatformIO

This project uses **PlatformIO** for development. All required libraries are automatically managed via `platformio.ini`:
- **TM1637 Driver** by akj7
- **WiFiManager** by tzapu
- **ezTime** for timezone/DST support (to be added in Phase 3)
- ESP32 platform with built-in WiFi and NTP support

### Installation

[Install PlatformIO](https://platformio.org/install) if you haven't already (via VS Code extension or CLI).

## Installation

1. Clone this repository:
   ```bash
   git clone <repository-url>
   cd Clock3
   ```

2. Open in PlatformIO (VS Code or CLI)

3. Configure settings in `config.h` (see Configuration section)

4. Build and upload:
   ```bash
   pio run --target upload
   ```

5. Monitor serial output (optional but recommended):
   ```bash
   pio device monitor
   # Or combined: pio run --target upload && pio device monitor
   ```

6. Connect to the WiFi configuration portal on first boot

## Configuration

All settings are in `config.h`:

### Timezone and Time Format

```cpp
TIMEZONE            // IANA timezone (e.g., "America/New_York", "Europe/London")
AUTO_DST            // true = automatic DST adjustment
TIME_FORMAT_24H     // false = 12-hour, true = 24-hour
LEADING_ZERO        // Show leading zero on hours
```

### NTP Settings

```cpp
NTP_SERVER_PRIMARY     // Default: "pool.ntp.org"
NTP_SERVER_SECONDARY   // Default: "time.nist.gov"
NTP_UPDATE_INTERVAL    // Resync interval (default: 3600000 = 1 hour)
```

### Display Settings

```cpp
DISPLAY_BRIGHTNESS  // 0-7 (0=dimmest, 7=brightest)
```

### Reconnection Settings

```cpp
RECONNECT_INTERVAL              // Retry interval (default: 60000 = 1 minute)
RECONNECT_ANIMATION_DURATION    // Animation duration (default: 5000 = 5 seconds)
MAX_RECONNECT_ATTEMPTS          // Give up after attempts (default: 5)
```

## Usage

### First Boot

1. Power on the ESP32
2. Display shows `-!!-` (WiFiManager portal active)
3. Connect to WiFi network "ESP32_Clock" or similar
4. Browser should auto-open to configuration portal (or navigate to 192.168.4.1)
5. Select your WiFi network and enter password
6. Device connects and syncs time
7. Display shows `-^^-` (success) for 2 seconds, then displays time

### Normal Operation

**12-Hour Format:**
- Hours 1-12 displayed without leading zero
- AM: Top colon dot illuminated
- PM: Bottom colon dot illuminated
- Colon blinks at 1 Hz (on during even seconds)

**24-Hour Format:**
- Hours 0-23 displayed without leading zero
- Both colon dots blink together at 1 Hz

### Display Patterns

| Pattern | Meaning |
|---------|---------|
| `-!!-` | WiFiManager portal waiting for configuration |
| Scanning animation | Connecting to WiFi / Syncing NTP |
| `-^^-` | Connection successful (shown for 2 seconds) |
| `-vv-` | Connection failed or lost |
| Time display | Normal operation |

### Reconnection Behavior

If WiFi connection is lost:
1. Display shows `-vv-` (error)
2. Waits 1 minute
3. Shows scanning animation for 5 seconds while reconnecting
4. If successful: Shows `-^^-` for 2 seconds, resumes time display
5. If failed: Returns to `-vv-` and retries
6. After 5 failed attempts: Remains in error state (requires reset)

## Troubleshooting

**Display shows `-!!-`**
- WiFiManager portal is active
- Connect to ESP32's WiFi network and configure credentials

**Display shows `-vv-`**
- WiFi connection failed or lost
- Check WiFi credentials and signal strength
- Device will auto-retry up to 5 times
- Reset device to retry configuration

**Time is incorrect**
- Check `TIMEZONE` setting in `config.h`
- Verify `AUTO_DST` setting matches your needs
- Ensure NTP servers are accessible

**Display is too dim/bright**
- Adjust `DISPLAY_BRIGHTNESS` (0-7) in `config.h`

## Serial Monitor

Use PlatformIO's device monitor (automatically configured to **115200 baud**):

```bash
pio device monitor
```

The monitor displays:
- WiFi connection status
- NTP synchronization info
- Error messages and debug output
- State transitions

**Tip:** The monitor auto-reconnects when ESP32 resets, and `Serial.flush()` is used throughout the code to prevent character dropping on USB serial.

## Project Structure

```
Clock3/
├── src/
│   ├── Clock3.ino        # Main application code
│   └── config.h          # User configuration settings
├── docs/
│   ├── IMPLEMENTATION_PLAN.md   # Complete phased development plan
│   ├── PHASE2_TODO.md          # Phase 2 task list
│   └── PHASE3_TODO.md          # Phase 3 task list
├── platformio.ini        # PlatformIO configuration and dependencies
├── tm1637-animation-demo.html  # Interactive display pattern demo
├── CLAUDE.md            # Development guidance for Claude Code
└── README.md            # This file
```

## Demo

Open `tm1637-animation-demo.html` in a browser to see interactive demonstrations of all display patterns.

## Development

This project is developed in phases using PlatformIO. See `docs/IMPLEMENTATION_PLAN.md` for the complete implementation strategy.

**Current phase:** Phase 2 - WiFi Connection with WiFiManager

To contribute or continue development:
1. Read the implementation plan in `docs/IMPLEMENTATION_PLAN.md`
2. Check phase-specific TODO files in `docs/` directory
3. Follow the guidance in `CLAUDE.md` for code architecture

## License

This project is open source. Feel free to modify and distribute.
