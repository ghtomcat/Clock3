/*
 * ESP32 NTP Clock with TM1637 Display
 *
 * An ESP32-based network time clock that displays the current time on a
 * TM1637 4-digit 7-segment display, synchronized via NTP with automatic
 * timezone and DST adjustments.
 *
 * Hardware:
 * - ESP32 Development Board
 * - TM1637 4-Digit 7-Segment Display Module
 *   - CLK -> GPIO 21
 *   - DIO -> GPIO 22
 *   - VCC -> 3.3V/5V
 *   - GND -> GND
 *
 * Serial Communication: 115200 baud
 */

// Library includes
#include <TM1637.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <time.h>
#include <ezTime.h>

// Project includes
#include "config.h"

// ============================================
// Global Objects
// ============================================

// Initialize TM1637 display with CLK and DIO pins from config.h
TM1637 display(CLK_PIN, DIO_PIN);

// WiFiManager object (global scope required for callbacks)
WiFiManager wm;

// ezTime timezone object (Phase 3)
Timezone myTZ;

// NTP sync status flag (Phase 3)
bool ntpSynced = false;

// ============================================
// State Machine
// ============================================

// Clock states
enum ClockState {
  STATE_WIFI_PORTAL,      // WiFiManager portal active (show -!!-)
  STATE_WIFI_CONNECTING,  // Connecting to WiFi (show scanning animation)
  STATE_WIFI_SUCCESS,     // Connected successfully (show -^^- for 2s)
  STATE_WIFI_FAILED,      // Connection failed (show -vv-)
  STATE_TIME_DISPLAY      // Normal operation - display time (Phase 4)
};

ClockState currentState = STATE_WIFI_PORTAL;
ClockState previousState = STATE_WIFI_PORTAL;

// ============================================
// Timing Variables
// ============================================

unsigned long lastAnimationUpdate = 0;
unsigned long successDisplayStart = 0;
unsigned long stateEntryTime = 0;

// ============================================
// WiFiManager Callback Functions
// ============================================

/**
 * Callback when WiFiManager enters configuration portal mode
 * This happens when no saved credentials exist or connection fails
 */
void configModeCallback(WiFiManager *myWiFiManager) {
  Serial.println();
  Serial.println("========================================");
  Serial.println("WiFiManager Configuration Portal Active");
  Serial.println("========================================");
  Serial.print("Access Point SSID: ");
  Serial.println(myWiFiManager->getConfigPortalSSID());
  Serial.print("Portal IP Address: ");
  Serial.println(WiFi.softAPIP());
  Serial.println();
  Serial.println("Connect to this WiFi network and navigate to:");
  Serial.println("http://192.168.4.1");
  Serial.println("to configure WiFi credentials.");
  Serial.println("========================================");
  Serial.flush();

  currentState = STATE_WIFI_PORTAL;
}

/**
 * Callback when WiFiManager saves new configuration
 * Called after user submits credentials via portal
 */
void saveConfigCallback() {
  Serial.println();
  Serial.println("========================================");
  Serial.println("WiFi Credentials Saved!");
  Serial.println("========================================");
  Serial.println("Attempting to connect with new credentials...");
  Serial.flush();

  currentState = STATE_WIFI_CONNECTING;
  stateEntryTime = millis();
}

// ============================================
// Helper Functions
// ============================================

/**
 * Convert state enum to human-readable string for debugging
 */
const char* getStateName(ClockState state) {
  switch (state) {
    case STATE_WIFI_PORTAL: return "WIFI_PORTAL";
    case STATE_WIFI_CONNECTING: return "WIFI_CONNECTING";
    case STATE_WIFI_SUCCESS: return "WIFI_SUCCESS";
    case STATE_WIFI_FAILED: return "WIFI_FAILED";
    case STATE_TIME_DISPLAY: return "TIME_DISPLAY";
    default: return "UNKNOWN";
  }
}

// ============================================
// Display Pattern Functions
// ============================================

/**
 * Display attention pattern: -!!-
 * Indicates WiFiManager portal is active and waiting for user to configure WiFi
 *
 * Segment mapping:
 * - Digit 1: segment G (middle horizontal dash)
 * - Digit 2: segments B, C, D (exclamation mark - vertical line with dot)
 * - Digit 3: segments B, C, D (exclamation mark - vertical line with dot)
 * - Digit 4: segment G (middle horizontal dash)
 */
void displayAttention() {
  uint8_t segments[] = {
    0x40,  // Digit 1: - (segment G = 0x40)
    0x0E,  // Digit 2: ! (segments B + C + D = 0x02 + 0x04 + 0x08 = 0x0E)
    0x0E,  // Digit 3: ! (segments B + C + D = 0x0E)
    0x40   // Digit 4: - (segment G = 0x40)
  };
  display.displayRawBytes(segments, 4);
}

/**
 * Display scanning animation: horizontal dash moving across digits
 * Call this function every 150ms to animate
 * Used during WiFi/NTP connection attempts
 *
 * Animation sequence: Digit1 → Digit2 → Digit3 → Digit4 → Digit3 → Digit2 → (repeat)
 */
void displayScanningAnimation() {
  static uint8_t frame = 0;

  // 6-frame animation pattern
  static const uint8_t patterns[6][4] = {
    {0x40, 0x00, 0x00, 0x00},  // Frame 0: dash on digit 1
    {0x00, 0x40, 0x00, 0x00},  // Frame 1: dash on digit 2
    {0x00, 0x00, 0x40, 0x00},  // Frame 2: dash on digit 3
    {0x00, 0x00, 0x00, 0x40},  // Frame 3: dash on digit 4
    {0x00, 0x00, 0x40, 0x00},  // Frame 4: dash on digit 3 (back)
    {0x00, 0x40, 0x00, 0x00}   // Frame 5: dash on digit 2 (back)
  };

  display.displayRawBytes(patterns[frame], 4);
  frame = (frame + 1) % 6;  // Cycle through 6 frames
}

/**
 * Display smiley face pattern: -^^-
 * Indicates successful WiFi connection and NTP sync
 * Show for 2 seconds after successful connection
 *
 * Segment mapping:
 * - Digit 1: segment G (middle horizontal dash)
 * - Digit 2: segments A, F, B (upward angle forming ^)
 * - Digit 3: segments A, F, B (upward angle forming ^)
 * - Digit 4: segment G (middle horizontal dash)
 */
void displaySmiley() {
  uint8_t segments[] = {
    0x40,  // Digit 1: - (segment G = 0x40)
    0x23,  // Digit 2: ^ (segments A + F + B = 0x01 + 0x20 + 0x02 = 0x23)
    0x23,  // Digit 3: ^ (segments A + F + B = 0x23)
    0x40   // Digit 4: - (segment G = 0x40)
  };
  display.displayRawBytes(segments, 4);
}

/**
 * Display frowny face pattern: -vv-
 * Indicates connection failure or error state
 * Display until reconnection attempt or permanent failure
 *
 * Segment mapping:
 * - Digit 1: segment G (middle horizontal dash)
 * - Digit 2: segments D, E, C (downward angle forming v)
 * - Digit 3: segments D, E, C (downward angle forming v)
 * - Digit 4: segment G (middle horizontal dash)
 */
void displayFrowny() {
  uint8_t segments[] = {
    0x40,  // Digit 1: - (segment G = 0x40)
    0x1C,  // Digit 2: v (segments D + E + C = 0x08 + 0x10 + 0x04 = 0x1C)
    0x1C,  // Digit 3: v (segments D + E + C = 0x1C)
    0x40   // Digit 4: - (segment G = 0x40)
  };
  display.displayRawBytes(segments, 4);
}

// ============================================
// NTP Synchronization Functions (Phase 3)
// ============================================

/**
 * Synchronize time with NTP servers using ezTime
 * ezTime handles both NTP sync and timezone application
 * Returns true on success, false on timeout
 */
bool syncNTP() {
  Serial.println("Syncing with NTP servers using ezTime...");
  Serial.flush();

  // Set NTP servers for ezTime
  setServer(NTP_SERVER_PRIMARY);

  // Wait for ezTime to sync with NTP
  Serial.println("Waiting for NTP sync...");
  if (!waitForSync(15)) {  // Wait up to 15 seconds for sync
    Serial.println("NTP sync timeout!");
    Serial.flush();
    return false;
  }

  Serial.println("NTP sync successful!");
  Serial.print("UTC time: ");
  Serial.println(UTC.dateTime());
  Serial.flush();

  return true;
}

/**
 * Initialize timezone using ezTime library
 * Applies IANA timezone string and handles DST automatically
 * Returns true on success, false on failure
 */
bool initTimezone() {
  Serial.print("Initializing timezone: ");
  Serial.println(TIMEZONE);
  Serial.flush();

  // Set timezone using ezTime (NTP already synced in syncNTP)
  if (!myTZ.setLocation(TIMEZONE)) {
    Serial.println("Timezone initialization failed!");
    Serial.flush();
    return false;
  }

  // Wait a moment for timezone data to load
  delay(500);

  Serial.print("Timezone configured: ");
  Serial.println(myTZ.getTimezoneName());
  Serial.print("UTC offset: ");
  Serial.print(myTZ.getOffset());
  Serial.println(" minutes");
  Serial.print("Current local time: ");
  Serial.println(myTZ.dateTime());
  Serial.flush();

  return true;
}

// ============================================
// Time Display Functions (Phase 4)
// ============================================

// Segment encoding for digits 0-9
// Bit mapping: A=0x01, B=0x02, C=0x04, D=0x08, E=0x10, F=0x20, G=0x40, DP=0x80
const uint8_t digitSegments[10] = {
  0x3F,  // 0: A+B+C+D+E+F
  0x06,  // 1: B+C
  0x5B,  // 2: A+B+D+E+G
  0x4F,  // 3: A+B+C+D+G
  0x66,  // 4: B+C+F+G
  0x6D,  // 5: A+C+D+F+G
  0x7D,  // 6: A+C+D+E+F+G
  0x07,  // 7: A+B+C
  0x7F,  // 8: All segments
  0x6F   // 9: A+B+C+D+F+G
};

/**
 * Display current time on TM1637 display
 * Handles 12/24 hour format and blinking colon
 */
void displayTime() {
  // Get current local time from ezTime timezone object
  // Note: Call without parameters to get timezone-adjusted values
  int hour = myTZ.hour();
  int minute = myTZ.minute();
  int second = myTZ.second();

  // Convert to 12-hour format if needed
  bool isPM = false;
  if (!TIME_FORMAT_24H) {
    isPM = (hour >= 12);
    if (hour == 0) {
      hour = 12;  // Midnight is 12 AM
    } else if (hour > 12) {
      hour -= 12;  // Convert to 12-hour format
    }
  }

  // Format digits
  int digit1 = hour / 10;
  int digit2 = hour % 10;
  int digit3 = minute / 10;
  int digit4 = minute % 10;

  // Determine colon state (blink at 1 Hz - ON during even seconds)
  bool colonOn = (second % 2 == 0);

  // Build segment array
  uint8_t segments[4];

  // Digit 1 (hours tens) - blank if 0 and LEADING_ZERO is false
  if (!LEADING_ZERO && digit1 == 0) {
    segments[0] = 0x00;  // Blank
  } else {
    segments[0] = digitSegments[digit1];
  }

  // Digit 2 (hours ones) - add colon bit if blinking on
  segments[1] = digitSegments[digit2];
  if (colonOn) {
    segments[1] |= 0x80;  // Add colon bit
  }

  // Digits 3 and 4 (minutes)
  segments[2] = digitSegments[digit3];
  segments[3] = digitSegments[digit4];

  // Display using raw bytes (same method as patterns)
  display.displayRawBytes(segments, 4);
}

// ============================================
// Setup Function
// ============================================

void setup() {
  // Initialize Serial communication for debugging
  Serial.begin(115200);

  // Wait for Serial to be ready
  delay(100);

  Serial.println();
  Serial.println("========================================");
  Serial.println("ESP32 NTP Clock with TM1637 Display");
  Serial.println("Phase 4: Time Display Implementation");
  Serial.println("========================================");
  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());
  Serial.println();

  // Initialize TM1637 Display
  Serial.println("Initializing TM1637 display...");
  Serial.print("  CLK Pin: ");
  Serial.print(CLK_PIN);
  Serial.print(", DIO Pin: ");
  Serial.println(DIO_PIN);

  display.begin();
  display.setBrightness(DISPLAY_BRIGHTNESS);

  Serial.print("  Brightness: ");
  Serial.println(DISPLAY_BRIGHTNESS);
  Serial.println("Display initialization complete!");
  Serial.println();

  // Initialize WiFi
  Serial.println("Initializing WiFi...");
  WiFi.mode(WIFI_STA);  // Station mode
  Serial.println("  WiFi mode: STA");

  // Configure WiFiManager
  wm.setConfigPortalBlocking(false);  // CRITICAL: Non-blocking mode
  wm.setAPCallback(configModeCallback);
  wm.setSaveConfigCallback(saveConfigCallback);
  wm.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT);

  Serial.print("  Portal timeout: ");
  Serial.print(WIFI_PORTAL_TIMEOUT);
  Serial.println(" seconds");
  Serial.print("  AP Name: ");
  Serial.println(WIFI_AP_NAME);
  Serial.println();

  // Attempt to connect with saved credentials or start portal
  Serial.println("Starting WiFiManager autoConnect...");
  Serial.flush();

  bool connected = wm.autoConnect(WIFI_AP_NAME);

  if (connected) {
    // Successfully connected with saved credentials
    Serial.println();
    Serial.println("========================================");
    Serial.println("Connected with saved WiFi credentials!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Syncing NTP...");
    Serial.println("========================================");
    Serial.flush();

    // Show scanning animation while syncing
    currentState = STATE_WIFI_CONNECTING;
    stateEntryTime = millis();
    lastAnimationUpdate = millis();
    displayScanningAnimation();

    // Attempt NTP sync
    if (syncNTP()) {
      if (initTimezone()) {
        Serial.println("Time synchronization complete!");
        Serial.flush();
        ntpSynced = true;
        currentState = STATE_WIFI_SUCCESS;
        successDisplayStart = millis();
        displaySmiley();
      } else {
        Serial.println("Timezone configuration failed!");
        Serial.flush();
        currentState = STATE_WIFI_FAILED;
        displayFrowny();
      }
    } else {
      Serial.println("NTP sync failed!");
      Serial.flush();
      currentState = STATE_WIFI_FAILED;
      displayFrowny();
    }
  } else {
    // No saved credentials or connection failed - portal will be active
    Serial.println("No saved credentials - portal will start");
    Serial.flush();
    currentState = STATE_WIFI_PORTAL;
    displayAttention();
  }

  Serial.println();
  Serial.println("Setup complete!");
  Serial.println();
}

// ============================================
// State Handler Functions
// ============================================

/**
 * Handle WiFi Portal state
 * Display attention pattern and wait for portal to close
 * Phase 3: Extended to include NTP sync after successful portal connection
 */
void handleWiFiPortal() {
  // Display attention pattern
  displayAttention();

  // Check if portal has closed
  if (!wm.getConfigPortalActive()) {
    // Portal closed - check WiFi status
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.println("========================================");
      Serial.println("WiFi Connected! Syncing NTP...");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
      Serial.println("========================================");
      Serial.flush();

      // Transition to connecting state for NTP sync
      currentState = STATE_WIFI_CONNECTING;
      stateEntryTime = millis();
      lastAnimationUpdate = millis();
    } else {
      Serial.println();
      Serial.println("========================================");
      Serial.println("WiFi Connection Failed!");
      Serial.println("========================================");
      Serial.print("Status: ");
      Serial.println(WiFi.status());
      Serial.println("Portal closed without successful connection");
      Serial.println("========================================");
      Serial.flush();

      currentState = STATE_WIFI_FAILED;
      displayFrowny();
    }
  }
}

/**
 * Handle WiFi Connecting state
 * Display scanning animation and check connection status
 * Phase 3: Extended to include NTP sync after WiFi connection
 */
void handleWiFiConnecting() {
  // Update scanning animation every ANIMATION_FRAME_INTERVAL ms
  unsigned long currentMillis = millis();
  if (currentMillis - lastAnimationUpdate >= ANIMATION_FRAME_INTERVAL) {
    lastAnimationUpdate = currentMillis;
    displayScanningAnimation();
  }

  // Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED && !ntpSynced) {
    // WiFi connected, now sync NTP
    Serial.println();
    Serial.println("========================================");
    Serial.println("WiFi Connected! Syncing NTP...");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println("========================================");
    Serial.flush();

    // Attempt NTP sync (blocking with timeout)
    if (syncNTP()) {
      // Initialize timezone
      if (initTimezone()) {
        Serial.println("Time synchronization complete!");
        Serial.flush();
        ntpSynced = true;

        // Transition to success state
        currentState = STATE_WIFI_SUCCESS;
        successDisplayStart = millis();
        displaySmiley();
      } else {
        Serial.println("Timezone configuration failed!");
        Serial.flush();
        currentState = STATE_WIFI_FAILED;
        displayFrowny();
      }
    } else {
      Serial.println("NTP sync failed!");
      Serial.flush();
      currentState = STATE_WIFI_FAILED;
      displayFrowny();
    }
  }
  // Check for WiFi connection timeout (before NTP sync)
  else if (!ntpSynced && currentMillis - stateEntryTime >= WIFI_CONNECT_TIMEOUT) {
    Serial.println();
    Serial.println("========================================");
    Serial.println("WiFi connection timeout!");
    Serial.println("========================================");
    Serial.flush();
    currentState = STATE_WIFI_FAILED;
    displayFrowny();
  }
}

/**
 * Handle WiFi Success state
 * Display smiley face for 2 seconds after successful WiFi + NTP sync
 * After 2 seconds, transition to time display (Phase 4)
 */
void handleWiFiSuccess() {
  unsigned long currentMillis = millis();

  // Display smiley for SUCCESS_DISPLAY_DURATION ms
  if (currentMillis - successDisplayStart < SUCCESS_DISPLAY_DURATION) {
    // Still showing smiley
    displaySmiley();
  } else {
    // Success display period complete - transition to time display
    Serial.println();
    Serial.println("Transitioning to time display mode...");
    Serial.flush();

    currentState = STATE_TIME_DISPLAY;
  }
}

/**
 * Handle WiFi Failed state
 * Display frowny face
 * Phase 5 will add reconnection logic here
 */
void handleWiFiFailed() {
  displayFrowny();
  // Phase 5 will implement reconnection logic here
}

/**
 * Handle Time Display state (Phase 4)
 * Display current time with blinking colon
 * Updates every second
 */
void handleTimeDisplay() {
  static unsigned long lastUpdate = 0;
  unsigned long currentMillis = millis();

  // Update display every 500ms to ensure colon blink is smooth
  // (every 500ms catches second transitions more reliably)
  if (currentMillis - lastUpdate >= 500) {
    lastUpdate = currentMillis;
    displayTime();
  }
}

// ============================================
// Loop Function
// ============================================

void loop() {
  // CRITICAL: Must call wm.process() every loop iteration for non-blocking operation
  wm.process();

  // Update ezTime for timezone synchronization (Phase 3+)
  events();

  // Execute state-specific handler
  switch (currentState) {
    case STATE_WIFI_PORTAL:
      handleWiFiPortal();
      break;

    case STATE_WIFI_CONNECTING:
      handleWiFiConnecting();
      break;

    case STATE_WIFI_SUCCESS:
      handleWiFiSuccess();
      break;

    case STATE_WIFI_FAILED:
      handleWiFiFailed();
      break;

    case STATE_TIME_DISPLAY:
      handleTimeDisplay();
      break;
  }

  // Log state transitions
  if (currentState != previousState) {
    Serial.println();
    Serial.print("State Transition: ");
    Serial.print(getStateName(previousState));
    Serial.print(" -> ");
    Serial.println(getStateName(currentState));
    Serial.flush();

    previousState = currentState;
    stateEntryTime = millis();
  }
}
