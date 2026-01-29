# Phase 2 TODO: WiFi Connection with WiFiManager

**Goal:** Establish WiFi connectivity with configuration portal

**Status:** Not Started
**Dependencies:** Phase 1 (Basic Display Control) must be completed

---

## Tasks

### 1. Add WiFiManager library dependency
**Status:** [ ] Pending

Install the WiFiManager library by tzapu for ESP32. This can be done via Arduino Library Manager (search for "WiFiManager") or PlatformIO (add to platformio.ini). Verify the library is properly installed and compatible with ESP32.

---

### 2. Initialize WiFiManager object and implement autoConnect
**Status:** [ ] Pending

In the main code file:
- Include WiFiManager.h header
- Create a WiFiManager object in global scope
- In setup(), call autoConnect() method with an appropriate AP name (e.g., "ESP32-NTP-Clock")
- This will automatically handle saved credentials or launch the configuration portal if no credentials exist
- Add error handling for autoConnect() return value

---

### 3. Add Serial debug output for WiFi status
**Status:** [ ] Pending

Add comprehensive Serial.println() statements to track WiFi connection status:
- When WiFiManager portal launches (no saved credentials)
- Portal IP address and SSID
- When user submits credentials via portal
- When attempting to connect to WiFi
- Connection success with IP address
- Connection failure with reason
- Current WiFi status checks during operation

---

### 4. Display attention pattern when WiFiManager portal is active
**Status:** [ ] Pending

Implement logic to show the attention pattern (-!!-) on the TM1637 display when WiFiManager is waiting for user configuration:
- Detect when WiFiManager portal is active (waiting for credentials)
- Call displayAttention() function (from Phase 1) to show -!!- pattern
- Pattern should remain visible until user submits WiFi credentials
- This indicates to user that action is required via the portal

---

### 5. Show scanning animation during WiFi connection
**Status:** [ ] Pending

After credentials are submitted (or when auto-connecting with saved credentials):
- Transition from attention pattern to scanning animation
- Call displayScanningAnimation() function (from Phase 1) continuously while WiFi.status() != WL_CONNECTED
- Animation should run at ~150ms per frame
- Continue until connection succeeds or fails
- Ensure animation is non-blocking (use millis() timing, not delay())

---

### 6. Display smiley face on successful WiFi connection
**Status:** [ ] Pending

When WiFi connection succeeds (WiFi.status() == WL_CONNECTED):
- Call displaySmiley() function to show -^^- pattern
- Display for exactly 2 seconds
- Use millis() for timing (non-blocking)
- Log successful connection to Serial with IP address
- After 2 seconds, transition to next state (ready for Phase 3 NTP sync)

---

### 7. Display frowny face on WiFi connection failure
**Status:** [ ] Pending

When WiFi connection fails after attempting to connect:
- Call displayFrowny() function to show -vv- pattern
- Display remains until next reconnection attempt
- Log connection failure to Serial with error details
- Implement basic error state that can later be extended with reconnection logic in Phase 5

---

### 8. Test WiFi configuration flow end-to-end
**Status:** [ ] Pending

Comprehensive testing of Phase 2 WiFi functionality:

**Test Case 1 - First boot (no saved credentials):**
- Erase WiFi credentials (WiFi.disconnect(true))
- Power cycle device
- Verify attention pattern (-!!-) displays
- Verify portal launches and is accessible
- Connect to ESP32 AP and configure WiFi
- Verify scanning animation appears
- Verify smiley face shows for 2 seconds on success

**Test Case 2 - Subsequent boot (saved credentials):**
- Power cycle with credentials saved
- Verify scanning animation appears immediately (no attention pattern)
- Verify auto-connection to saved network
- Verify smiley face on success

**Test Case 3 - Connection failure:**
- Configure with invalid WiFi credentials
- Verify scanning animation during attempt
- Verify frowny face appears on failure

Verify all Serial debug output is clear and helpful.

---

## Verification Criteria

Phase 2 is complete when:
- [ ] Device shows attention pattern (-!!-) when WiFiManager portal is active (waiting for user configuration)
- [ ] Portal launches on first boot (no saved credentials)
- [ ] Device auto-connects on subsequent boots using saved credentials
- [ ] Scanning animation displays during connection attempts
- [ ] Smiley face (-^^-) displays for 2 seconds on successful connection
- [ ] Frowny face (-vv-) displays when connection fails
- [ ] All WiFi status changes are logged to Serial output
- [ ] Visual feedback matches specification exactly

---

## Next Steps

After completing Phase 2, proceed to **Phase 3: NTP Time Synchronization**
