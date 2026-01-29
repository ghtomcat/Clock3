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

// Project includes
#include "config.h"

// ============================================
// Global Objects
// ============================================

// Initialize TM1637 display with CLK and DIO pins from config.h
TM1637 display(CLK_PIN, DIO_PIN);

// WiFiManager object (global scope required for callbacks)
WiFiManager wm;

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
  Serial.println("Phase 2: WiFi Connection");
  Serial.println("========================================");
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
    Serial.println("WiFi Connection Successful!");
    Serial.println("========================================");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.println("========================================");
    Serial.flush();

    currentState = STATE_WIFI_SUCCESS;
    successDisplayStart = millis();
    displaySmiley();
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
      Serial.println("WiFi Connection Successful!");
      Serial.println("========================================");
      Serial.print("SSID: ");
      Serial.println(WiFi.SSID());
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
      Serial.print("Signal Strength: ");
      Serial.print(WiFi.RSSI());
      Serial.println(" dBm");
      Serial.println("========================================");
      Serial.flush();

      currentState = STATE_WIFI_SUCCESS;
      successDisplayStart = millis();
      displaySmiley();
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
 */
void handleWiFiConnecting() {
  // Update scanning animation every ANIMATION_FRAME_INTERVAL ms
  unsigned long currentMillis = millis();
  if (currentMillis - lastAnimationUpdate >= ANIMATION_FRAME_INTERVAL) {
    lastAnimationUpdate = currentMillis;
    displayScanningAnimation();
  }

  // Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("========================================");
    Serial.println("WiFi Connection Successful!");
    Serial.println("========================================");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.println("========================================");
    Serial.flush();

    currentState = STATE_WIFI_SUCCESS;
    successDisplayStart = millis();
    displaySmiley();
  }
  // Check for timeout
  else if (currentMillis - stateEntryTime >= WIFI_CONNECT_TIMEOUT) {
    Serial.println();
    Serial.println("========================================");
    Serial.println("WiFi Connection Timeout!");
    Serial.println("========================================");
    Serial.print("Status: ");
    Serial.println(WiFi.status());
    Serial.print("Timeout after ");
    Serial.print(WIFI_CONNECT_TIMEOUT / 1000);
    Serial.println(" seconds");
    Serial.println("========================================");
    Serial.flush();

    currentState = STATE_WIFI_FAILED;
    displayFrowny();
  }
}

/**
 * Handle WiFi Success state
 * Display smiley face for 2 seconds
 * After 2 seconds, ready for Phase 3 (NTP sync)
 */
void handleWiFiSuccess() {
  unsigned long currentMillis = millis();

  // Display smiley for SUCCESS_DISPLAY_DURATION ms
  if (currentMillis - successDisplayStart < SUCCESS_DISPLAY_DURATION) {
    // Still showing smiley
    displaySmiley();
  } else {
    // Success display period complete
    // Phase 3 will add transition to NTP_SYNCING state here
    // For now, stay in success state
    Serial.println();
    Serial.println("Success display complete. Ready for Phase 3 (NTP).");
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
