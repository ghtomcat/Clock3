# Phase 3: NTP Time Synchronization - TODO

**Goal:** Get current time from NTP server and apply timezone/DST adjustments

## Overview
This phase implements NTP time synchronization with timezone conversion and DST handling. The device will sync time from NTP servers, apply the correct timezone offset, and handle DST transitions automatically.

---

## Step 3.1: NTP Client Setup

### Tasks:

- [ ] **Research NTP library options**
  - Compare ESP32 built-in `configTime()` vs `NTPClient` library
  - Decision: Use built-in `configTime()` for native ESP32 support
  - Verify compatibility with current WiFi setup

- [ ] **Add NTP configuration to config.h**
  - Add `NTP_SERVER_PRIMARY` constant (default: "pool.ntp.org")
  - Add `NTP_SERVER_SECONDARY` constant (default: "time.nist.gov")
  - Add `NTP_UPDATE_INTERVAL` constant (default: 3600000ms = 1 hour)
  - Add GMT_OFFSET_SEC for timezone offset calculation
  - Add DAYLIGHT_OFFSET_SEC for DST offset

- [ ] **Implement NTP sync function**
  - Create `syncNTP()` function
  - Use `configTime()` with primary and secondary servers
  - Implement blocking wait for first sync (use `time()` and check for valid time)
  - Add timeout mechanism (max 10 seconds wait)
  - Return success/failure status

- [ ] **Add fallback to secondary server**
  - If primary server fails, configTime automatically tries secondary
  - Log which server was used for debugging

- [ ] **Add Serial debug output**
  - Print NTP sync attempt message
  - Print retrieved Unix timestamp
  - Print human-readable time (for verification)
  - Print sync success/failure status
  - Add `Serial.flush()` after output groups

- [ ] **Test NTP sync functionality**
  - Verify successful sync on first boot
  - Check Serial output shows correct Unix timestamp
  - Verify time is reasonable (year 2025+, not 1970)

---

## Step 3.2: Timezone and DST Handling

### Tasks:

- [ ] **Research timezone library options**
  - Option 1: `ezTime` library (IANA timezone strings, auto-DST)
  - Option 2: `Timezone` library by Jack Christensen
  - Option 3: Manual offset calculation with DST rules
  - Decision: Choose based on memory footprint and ease of use

- [ ] **Add library dependency**
  - Add chosen library to `platformio.ini` lib_deps
  - Or document Arduino Library Manager installation in README

- [ ] **Add timezone configuration to config.h**
  - Add `TIMEZONE` string constant (IANA format, e.g., "America/New_York")
  - Add `AUTO_DST` boolean constant (default: true)
  - Document supported timezone formats in comments

- [ ] **Implement timezone initialization**
  - Create `initTimezone()` function
  - Configure timezone library with IANA string
  - Enable/disable auto-DST based on config
  - Call during startup after WiFi connection

- [ ] **Implement timezone conversion function**
  - Create `getLocalTime(struct tm &timeinfo)` function
  - Get UTC time from ESP32 clock
  - Apply timezone conversion
  - Return local time in tm struct
  - Handle DST transitions

- [ ] **Test timezone handling**
  - Test with US Eastern timezone (EST/EDT)
  - Test with European timezone (CET/CEST)
  - Test with timezone without DST (e.g., Arizona)
  - Verify correct offset calculation
  - Check time displays correctly in Serial output

---

## Step 3.3: Initial Sync Flow

### Tasks:

- [ ] **Extend startup sequence**
  - Modify existing WiFi connection code
  - After WiFi connects, continue showing scanning animation
  - Add NTP sync step before showing success

- [ ] **Implement sync flow logic**
  - WiFi connects → keep scanning animation running
  - Call `syncNTP()` function
  - Wait for successful NTP sync (blocking, with timeout)
  - If sync successful → show smiley face for 2 seconds
  - If sync fails → show frowny face, proceed to reconnection logic

- [ ] **Update state machine**
  - Ensure `STATE_CONNECTING` includes NTP sync
  - Don't transition to `STATE_SUCCESS` until both WiFi AND NTP succeed
  - Add proper error handling for NTP failures

- [ ] **Add Serial output for sync flow**
  - "Connecting to WiFi..." message
  - "WiFi connected" message
  - "Syncing time with NTP server..." message
  - "Time synchronized: [timestamp]" message
  - "Ready to display time" message

- [ ] **Prevent premature time display**
  - Add flag `ntpSynced` (boolean)
  - Only allow transition to time display state if flag is true
  - Ensure internal clock is valid before displaying

- [ ] **Test complete startup flow**
  - Power cycle device
  - Verify scanning animation continues during NTP sync
  - Confirm smiley face appears only after successful NTP sync
  - Check Serial output shows complete sequence
  - Measure total startup time (should be < 10 seconds typically)

---

## Verification Checklist

- [ ] Device successfully syncs time from NTP server on first boot
- [ ] Timezone offset is correctly applied
- [ ] DST is handled automatically (if enabled)
- [ ] Scanning animation continues during entire connection + NTP sync process
- [ ] Smiley face appears for 2 seconds after successful sync
- [ ] Serial output clearly shows NTP sync status and retrieved time
- [ ] Fallback to secondary NTP server works if primary fails
- [ ] Device handles NTP sync failure gracefully (shows frowny face)
- [ ] Time is accurate to the second
- [ ] No premature time display before first successful sync

---

## Testing Scenarios

### Test 1: Normal Boot
- Fresh boot → WiFi connects → NTP syncs → Smiley → (ready for Phase 4)
- Expected result: Smooth transition, correct time in Serial output

### Test 2: NTP Server Unavailable
- Block primary NTP server (router firewall rule)
- Expected result: Fallback to secondary server, still succeeds

### Test 3: Both NTP Servers Unavailable
- Block all NTP traffic
- Expected result: Timeout after 10 seconds, show frowny face

### Test 4: Timezone Verification
- Set timezone to "America/Los_Angeles"
- Set timezone to "Europe/Berlin"
- Set timezone to "Asia/Tokyo"
- Expected result: Time displays correct offset for each timezone

### Test 5: DST Transition
- If testing during DST transition period, verify automatic adjustment
- Manually set system time near DST boundary (if possible)
- Expected result: Correct offset applied before and after DST change

---

## Notes

- NTP sync is BLOCKING on first boot - this is intentional to ensure valid time before display
- Background hourly resyncs (non-blocking) will be implemented in Phase 6
- The ESP32 internal clock maintains time between syncs - no need for continuous NTP queries
- If NTP fails completely, device should enter error/reconnection state (already handled by Phase 2 logic)
- Consider adding NTP sync retry logic (3 attempts) before declaring failure
- Memory usage: Monitor heap during timezone library operations

---

## Dependencies

**Before starting Phase 3:**
- Phase 2 must be complete (WiFi connection working)
- WiFiManager successfully connecting to saved/configured network
- Display patterns working (scanning animation, smiley, frowny)

**After completing Phase 3:**
- Ready for Phase 4: Time Display Implementation
- Time is synced and timezone-adjusted
- Internal clock is running with valid time

---

## Estimated Completion

Phase 3 should take approximately 2-3 development sessions:
- Session 1: NTP client setup and basic sync (Step 3.1)
- Session 2: Timezone library integration and testing (Step 3.2)
- Session 3: Startup flow integration and verification (Step 3.3)
