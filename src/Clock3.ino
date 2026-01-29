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
  STATE_WIFI_FAILED       // Connection failed (show -vv-)
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
 * Synchronize time with NTP servers
 * Uses blocking sync with timeout for initial boot
 * Returns true on success, false on timeout
 */
bool syncNTP() {
  Serial.println("Syncing with NTP servers...");
  Serial.flush();

  // Configure NTP with primary and secondary servers
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC,
             NTP_SERVER_PRIMARY, NTP_SERVER_SECONDARY);

  // Wait for valid time (blocking with timeout)
  unsigned long startMillis = millis();
  time_t now = 0;

  while (now < 24 * 3600) {  // Valid time should be > 1 day since epoch
    time(&now);
    if (millis() - startMillis > NTP_SYNC_TIMEOUT) {
      Serial.println("NTP sync timeout!");
      Serial.flush();
      return false;
    }
    delay(100);
  }

  // Print synchronized time
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  Serial.print("NTP sync successful! Unix timestamp: ");
  Serial.println(now);
  Serial.print("UTC time: ");
  Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
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

  // Wait for ezTime to sync with NTP
  waitForSync(10);  // Wait up to 10 seconds for ezTime sync

  if (!timeStatus() == timeSet) {
    Serial.println("ezTime sync failed!");
    Serial.flush();
    return false;
  }

  // Set timezone using ezTime
  if (!myTZ.setLocation(TIMEZONE)) {
    Serial.println("Timezone initialization failed!");
    Serial.flush();
    return false;
  }

  Serial.print("Timezone configured: ");
  Serial.println(myTZ.getTimezoneName());
  Serial.print("Current local time: ");
  Serial.println(myTZ.dateTime());
  Serial.flush();

  return true;
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
  Serial.println("Phase 3: NTP Time Synchronization");
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
 * After 2 seconds, ready for Phase 4 (Time Display)
 */
void handleWiFiSuccess() {
  unsigned long currentMillis = millis();

  // Display smiley for SUCCESS_DISPLAY_DURATION ms
  if (currentMillis - successDisplayStart < SUCCESS_DISPLAY_DURATION) {
    // Still showing smiley
    displaySmiley();
  } else {
    // Success display period complete
    // Phase 4 will add transition to TIME_DISPLAY state here
    // For now, stay in success state
    Serial.println();
    Serial.println("Success display complete. Ready for Phase 4 (Time Display).");
    Serial.flush();
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

// ============================================
// Loop Function
// ============================================

void loop() {
  // CRITICAL: Must call wm.process() every loop iteration for non-blocking operation
  wm.process();

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
