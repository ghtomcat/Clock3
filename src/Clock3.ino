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
#include <TM1637Display.h>

// Project includes
#include "config.h"

// ============================================
// Global Objects
// ============================================

// Initialize TM1637 display with CLK and DIO pins from config.h
TM1637Display display(CLK_PIN, DIO_PIN);

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
  display.setSegments(segments);
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

  display.setSegments(patterns[frame]);
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
  display.setSegments(segments);
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
  display.setSegments(segments);
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
  Serial.println("========================================");
  Serial.println();
  Serial.println("Initializing...");
  Serial.println();

  // Initialize TM1637 Display
  Serial.print("Initializing TM1637 display... ");
  Serial.print("CLK Pin: ");
  Serial.print(CLK_PIN);
  Serial.print(", DIO Pin: ");
  Serial.println(DIO_PIN);

  // Set display brightness (0-7)
  display.setBrightness(DISPLAY_BRIGHTNESS);
  Serial.print("Display brightness set to: ");
  Serial.println(DISPLAY_BRIGHTNESS);

  Serial.println("Display initialization complete!");
  Serial.println();
  Serial.println("========================================");
  Serial.println("PHASE 1 PATTERN TEST MODE");
  Serial.println("========================================");
  Serial.println("This test cycles through all 4 patterns:");
  Serial.println("  1. Attention Pattern (-!!-)");
  Serial.println("  2. Scanning Animation (moving dash)");
  Serial.println("  3. Smiley Face (-^^-)");
  Serial.println("  4. Frowny Face (-vv-)");
  Serial.println();
  Serial.println("Compare with tm1637-animation-demo.html");
  Serial.println("Verify each pattern displays correctly.");
  Serial.println("========================================");
  Serial.println();
  Serial.println("Setup complete - starting pattern test...");
  Serial.println();
}

// ============================================
// Loop Function - Pattern Testing
// ============================================

void loop() {
  // This is a test routine for Phase 1
  // It cycles through all display patterns to verify they work correctly
  // Compare with tm1637-animation-demo.html for accuracy

  // Pattern 1: Attention (-!!-)
  Serial.println(">>> Displaying: Attention Pattern (-!!-)");
  Serial.println("    Usage: WiFiManager portal active");
  displayAttention();
  delay(2000);  // Show for 2 seconds

  Serial.println();

  // Pattern 2: Scanning Animation
  Serial.println(">>> Displaying: Scanning Animation");
  Serial.println("    Usage: Connecting to WiFi/NTP");
  Serial.println("    Running 7 frames (~1 second)...");
  for (int i = 0; i < 7; i++) {
    displayScanningAnimation();
    delay(150);  // 150ms per frame as specified
  }

  Serial.println();

  // Pattern 3: Smiley Face (-^^-)
  Serial.println(">>> Displaying: Smiley Face (-^^-)");
  Serial.println("    Usage: Successful connection");
  displaySmiley();
  delay(2000);  // Show for 2 seconds

  Serial.println();

  // Pattern 4: Frowny Face (-vv-)
  Serial.println(">>> Displaying: Frowny Face (-vv-)");
  Serial.println("    Usage: Connection failure/error");
  displayFrowny();
  delay(2000);  // Show for 2 seconds

  Serial.println();
  Serial.println("========================================");
  Serial.println("Pattern cycle complete. Repeating...");
  Serial.println("========================================");
  Serial.println();

  delay(1000);  // Brief pause before repeating cycle
}
