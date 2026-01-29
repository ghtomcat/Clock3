# ESP32 NTP Clock Implementation Plan

## Overview
Build an ESP32-based NTP clock with TM1637 display that syncs time via WiFi, handles connection failures gracefully, and displays time with visual feedback patterns.

## Critical Files
- `config.h` - Configuration parameters
- `Clock3.ino` or `main.cpp` - Main implementation
- `README.md` - Documentation

## Implementation Strategy

The implementation will be built incrementally in phases, with each phase adding functionality that builds on the previous one. This approach allows testing at each stage and reduces complexity.

---

## Phase 1: Basic Display Control
**Goal:** Get the TM1637 display working with basic patterns

### Step 1.1: Project Setup and Library Selection
- Create Arduino project structure
- Research and select TM1637 library (e.g., `TM1637Display` by Avishay Orpaz or similar well-maintained library)
- Install required libraries via Arduino Library Manager or PlatformIO
- Create basic config.h with GPIO pin definitions (CLK=21, DIO=22)
- Set up Serial communication at 115200 baud

### Step 1.2: Display Initialization and Test
- Initialize TM1637 display object
- Implement display brightness setting
- Create test pattern to verify display works (show "8888" with all segments)
- Test basic number display functionality

### Step 1.3: Display Pattern Functions
Create modular functions for each visual pattern:
- `displayAttention()` - Show `-!!-` pattern (segments: G, BCD, BCD, G) for WiFiManager portal
- `displayScanningAnimation()` - Horizontal dash moving across digits (segment G)
  - Implement frame-by-frame animation array: [0] → [1] → [2] → [3] → [2] → [1] → repeat
  - Use 150ms frame timing
- `displaySmiley()` - Show `-^^-` pattern (segments: G, AFB, AFB, G)
- `displayFrowny()` - Show `-vv-` pattern (segments: G, DEC, DEC, G)
- Test each pattern individually

**Verification:** Display should show all four patterns correctly when triggered

---

## Phase 2: WiFi Connection with WiFiManager
**Goal:** Establish WiFi connectivity with configuration portal

### Step 2.1: WiFiManager Integration
- Add WiFiManager library dependency
- Initialize WiFiManager object
- Implement autoConnect() to handle saved credentials or launch portal
- Add Serial debug output for WiFi status

### Step 2.2: Connection State Display
- Show attention pattern (-!!-) when WiFiManager portal is active (waiting for user configuration)
- Once credentials are submitted, transition to scanning animation while WiFi is connecting
- Display smiley face for 2 seconds on successful connection
- Display frowny face if connection fails
- Test WiFi configuration flow:
  - First boot (no credentials) → portal launches → show attention pattern
  - User configures WiFi → show scanning animation
  - Subsequent boots → auto-connect to saved network → show scanning animation

**Verification:** Device should show attention pattern when portal is active, launch portal on first boot, auto-connect on subsequent boots, and show appropriate visual feedback for each state

---

## Phase 3: NTP Time Synchronization
**Goal:** Get current time from NTP server

### Step 3.1: NTP Client Setup
- Add NTP client library (e.g., `NTPClient` or use ESP32's built-in `configTime()`)
- Add config.h parameters: NTP_SERVER_PRIMARY, NTP_SERVER_SECONDARY, NTP_UPDATE_INTERVAL
- Implement NTP sync function with fallback to secondary server
- Add Serial output for sync status and retrieved time

### Step 3.2: Timezone and DST Handling
- Add Timezone library dependency (e.g., `ezTime` or `Timezone` library)
- Add config.h parameters: TIMEZONE (IANA string), AUTO_DST
- Implement timezone conversion function
- Configure automatic DST adjustment
- Test with various timezones

### Step 3.3: Initial Sync Flow
- Extend startup sequence:
  1. WiFi connection (scanning animation)
  2. NTP synchronization (continue scanning animation)
  3. Success (smiley face for 2 seconds)
- Ensure device waits for successful NTP sync before proceeding

**Verification:** Device should sync time from NTP server and correctly apply timezone offset

---

## Phase 4: Time Display Implementation
**Goal:** Display time on the 7-segment display

### Step 4.1: Time Formatting Functions
- Add config.h parameters: TIME_FORMAT_24H, LEADING_ZERO
- Implement `format12Hour()` function:
  - Convert 24h to 12h format (0→12, 13→1, etc.)
  - Return hours (1-12) without leading zero
  - Return AM/PM flag
- Implement `format24Hour()` function:
  - Return hours (0-23) without leading zero
- Create `formatTimeForDisplay(hour, minute)` function
  - Handle leading zero logic based on LEADING_ZERO config
  - Return digit array for display

### Step 4.2: Colon Control
- Implement colon blinking logic:
  - 12-hour mode: Top dot for AM, bottom dot for PM
  - Blink at 1 Hz, synchronized with seconds (ON during even seconds, OFF during odd)
  - 24-hour mode: Both dots blink together
- Create `updateColon(seconds)` function

### Step 4.3: Display Update Loop
- Implement main display loop:
  - Update time display every second
  - Update colon based on current second
  - Get time from ESP32 internal clock (keeps running between NTP syncs)
- Test time display for various times (9:05 AM, 12:30 PM, 3:45 AM, etc.)

**Verification:** Time should display correctly in both 12h and 24h formats with proper colon behavior

---

## Phase 5: State Machine and Error Handling
**Goal:** Implement robust state management with reconnection logic

### Step 5.1: State Machine Structure
Define states as enum:
- `STATE_STARTUP` - Initial state
- `STATE_CONNECTING` - Connecting to WiFi/NTP
- `STATE_SUCCESS` - Show success pattern
- `STATE_DISPLAY_TIME` - Normal operation
- `STATE_ERROR` - Show error pattern
- `STATE_RECONNECTING` - Attempting reconnection
- `STATE_PERMANENT_FAILURE` - Given up after max attempts

Create state transition functions and main state machine loop

### Step 5.2: Connection Monitoring
- Implement WiFi connection check in main loop
- Detect connection loss during operation
- Add config.h parameters: RECONNECT_INTERVAL, RECONNECT_ANIMATION_DURATION, MAX_RECONNECT_ATTEMPTS
- Track reconnection attempt counter

### Step 5.3: Reconnection Logic
Implement reconnection state machine:
1. On connection loss → STATE_ERROR (show frowny face)
2. Wait RECONNECT_INTERVAL (60 seconds)
3. → STATE_RECONNECTING (show scanning animation for 5 seconds)
4. Attempt WiFi + NTP reconnection
5. If success → STATE_SUCCESS (smiley for 2 seconds) → STATE_DISPLAY_TIME
6. If fail → STATE_ERROR, increment attempt counter
7. After MAX_RECONNECT_ATTEMPTS → STATE_PERMANENT_FAILURE

### Step 5.4: Internal Clock Continuity
- Ensure ESP32 internal clock continues running during disconnection
- On reconnection, resync with NTP but maintain continuous time display
- Test behavior during WiFi disconnection

**Verification:** Device should handle connection loss gracefully and attempt reconnection according to specification

---

## Phase 6: Background NTP Resync
**Goal:** Hourly NTP resynchronization without disrupting display

### Step 6.1: Non-blocking NTP Update
- Implement hourly NTP resync (NTP_UPDATE_INTERVAL = 3600000ms)
- Use non-blocking timing (check millis() rather than delay())
- Resync in background without affecting time display
- Update internal clock upon successful resync
- Log sync status to Serial

### Step 6.2: Failed Resync Handling
- If hourly resync fails, log error but continue displaying time
- Don't trigger reconnection logic (only for complete connection loss)
- Retry at next interval

**Verification:** Device should resync every hour without display interruption

---

## Phase 7: Configuration and Polish
**Goal:** Complete configuration system and add final polish

### Step 7.1: Complete config.h
Ensure all parameters are properly defined:
- Timezone configuration (TIMEZONE, AUTO_DST)
- Time display format (TIME_FORMAT_24H, LEADING_ZERO)
- NTP configuration (servers, update interval)
- Display brightness (DISPLAY_BRIGHTNESS)
- Reconnection settings (interval, animation duration, max attempts)
- GPIO pins (CLK_PIN, DIO_PIN)

### Step 7.2: Debug Output
Add comprehensive Serial.println() statements for:
- Startup sequence
- WiFi connection status
- NTP sync attempts and results
- State transitions
- Error conditions
- Time updates (periodic, not every second)

### Step 7.3: Code Organization
- Add function header comments
- Organize code into logical sections with comments
- Ensure modular structure (separate functions for each responsibility)
- Add inline comments for complex logic

**Verification:** All configuration options work as expected

---

## Phase 8: Testing and Documentation
**Goal:** Comprehensive testing and documentation

### Step 8.1: Functional Testing
Test all scenarios from specification:
- First boot (no credentials) → portal
- Normal operation → time display
- WiFi disconnection → reconnection flow
- NTP sync failure → retry logic
- Max reconnection attempts → permanent failure
- Timezone and DST changes
- Both 12h and 24h display modes
- Colon blinking behavior

### Step 8.2: README.md Creation
Create comprehensive README with:
- Hardware wiring diagram/table
- Required library installation instructions
- config.h configuration guide
- Upload and usage instructions
- Troubleshooting section:
  - Display not working → check wiring
  - WiFi portal not appearing → factory reset procedure
  - Time incorrect → check timezone setting
  - Connection issues → check network, NTP servers

### Step 8.3: Edge Case Testing
- Very long WiFi disconnections
- Power cycle during various states
- Invalid timezone strings
- NTP server unavailable
- Time display at midnight (11:59 PM → 12:00 AM)
- Leading zero behavior at 9:XX, 10:XX, etc.

**Verification:** All test cases pass, documentation is complete and accurate

---

## Implementation Notes

### Library Recommendations
- **TM1637:** `TM1637Display` by Avishay Orpaz (actively maintained, simple API)
- **WiFiManager:** `WiFiManager` by tzapu (standard choice for ESP32)
- **NTP:** ESP32 built-in `configTime()` function (native support) OR `NTPClient` library
- **Timezone:** `ezTime` library (supports IANA timezone strings and auto-DST)

### Critical Design Decisions
1. **State Machine:** Use enum-based state machine with explicit state transitions
2. **Timing:** Use millis() for all timing (never use delay() in main loop)
3. **Non-blocking:** All operations must be non-blocking to maintain display responsiveness
4. **Error Recovery:** Always maintain graceful degradation (show time even if disconnected)

### Testing Strategy
- Test each phase independently before moving to next
- Use Serial monitor extensively for debugging
- Test both success and failure paths at each stage
- Verify display patterns match specification exactly

---

## Success Criteria
- Device automatically connects to WiFi on boot
- Time displays accurately with proper timezone/DST adjustment
- Visual feedback (animations, smiley, frowny) matches specification
- Colon blinking behavior correct for both 12h/24h modes
- Connection loss handled gracefully with automatic reconnection
- Hourly NTP resync works without disrupting display
- All configuration options in config.h work as expected
- Code is modular, readable, and well-commented
