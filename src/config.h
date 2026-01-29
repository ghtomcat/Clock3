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
#define DISPLAY_BRIGHTNESS 4

// ============================================
// Configuration to be added in later phases:
// ============================================
// - Timezone configuration (TIMEZONE, AUTO_DST)
// - Time display format (TIME_FORMAT_24H, LEADING_ZERO)
// - NTP server configuration
// - Reconnection settings
// ============================================

#endif // CONFIG_H
