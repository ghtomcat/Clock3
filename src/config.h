/*
 * Configuration File for ESP32 NTP Clock
 *
 * This file contains all user-configurable parameters for the clock.
 * Additional configuration options will be added in later phases.
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// GPIO Pin Assignments (Phase 1)
// ============================================

// TM1637 Display Pins
#define CLK_PIN 21  // Clock pin for TM1637 display
#define DIO_PIN 22  // Data I/O pin for TM1637 display

// ============================================
// Display Configuration (Phase 1)
// ============================================

// Display brightness level
// Valid range: 0-7 (0 = dimmest, 7 = brightest)
#define DISPLAY_BRIGHTNESS 2

// ============================================
// WiFi Configuration (Phase 2)
// ============================================

// WiFiManager Portal Settings
#define WIFI_AP_NAME "ESP32-NTP-Clock"  // Access point name for configuration portal
#define WIFI_PORTAL_TIMEOUT 60          // Portal timeout in seconds

// WiFi Connection Settings
#define WIFI_CONNECT_TIMEOUT 30000      // Connection timeout in milliseconds (30 seconds)

// Display Animation Settings
#define ANIMATION_FRAME_INTERVAL 150    // Scanning animation frame interval in ms
#define SUCCESS_DISPLAY_DURATION 2000   // Smiley face display duration in ms (2 seconds)

// ============================================
// NTP Configuration (Phase 3)
// ============================================

// NTP Server Settings
#define NTP_SERVER_PRIMARY "pool.ntp.org"
#define NTP_SERVER_SECONDARY "time.nist.gov"

// NTP Sync Settings
#define NTP_SYNC_TIMEOUT 10000          // 10 seconds
#define NTP_UPDATE_INTERVAL 3600000     // 1 hour (for Phase 6)

// Timezone Settings
#define TIMEZONE "Europe/Zurich"        // IANA timezone string
#define AUTO_DST true                   // Automatic DST adjustment

// ============================================
// Time Display Configuration (Phase 4)
// ============================================

// Time Format Settings
#define TIME_FORMAT_24H true            // false = 12-hour format, true = 24-hour format
#define LEADING_ZERO false              // Show leading zero on hours (e.g., 09:30 vs 9:30)

// ============================================
// Configuration to be added in later phases:
// ============================================
// - Reconnection settings
// ============================================

#endif // CONFIG_H
