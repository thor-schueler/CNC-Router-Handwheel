// Copyright (c) Thor Schueler. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef _DISPLAY_WHEEL_H_
#define _DISPLAY_WHEEL_H_

#include "Arduino.h"
#include "../display_spi/display_spi.h"
#include "../battery_gauge/battery_status.h"

#define ARROW_MIDDLE_ADJ(i) (uint8_t)((i <= 5) ? std::ceil(static_cast<float>(i) / 2) : std::floor(static_cast<float>(i - 1) / 3))

typedef enum { X=88, Y=89, Z=90, NA=255 } Axis;
struct Feed { 
	static constexpr float NANO = 0.001f; 
	static constexpr float MICRO = 0.01f; 
	static constexpr float MILLI = 0.1f; 
	static constexpr float FULL = 1.0f;
};
typedef enum { UP, DOWN, LEFT, RIGHT} Direction;

extern const unsigned char lcars[] PROGMEM;
extern const unsigned char splash[] PROGMEM;
extern const unsigned char battery[] PROGMEM;
extern const unsigned char battery_charging[] PROGMEM;
extern const unsigned char wifi_on[] PROGMEM;
extern const unsigned char wifi_off[] PROGMEM;
extern const unsigned char usb_on[] PROGMEM;
extern const unsigned char usb_off[] PROGMEM;
extern const unsigned char bt_on[] PROGMEM;
extern const unsigned char bt_off[] PROGMEM;
extern const size_t lcars_size;
extern const size_t splash_size;
extern const size_t battery_size;
extern const size_t battery_charging_size;
extern const size_t wifi_on_size;
extern const size_t wifi_off_size;
extern const size_t usb_on_size;
extern const size_t usb_off_size;
extern const size_t bt_on_size;
extern const size_t bt_off_size;


/**
 * @brief Implements the display for the handwheel
 */
class DISPLAY_Wheel:public DISPLAY_SPI
{
	public:
		/**
		 * @brief Generates a new instance of the DISPLAY_SPI class. 
		 * @details initializes the SPI and LCD pins including CS, RS, RESET 
		 */
		DISPLAY_Wheel();

		/**
		 * @brief Draws an arrow on the display
		 * @param x - The x coordinate of the arrow
		 * @param y - The y coordinate of the arrow
		 * @param d - The direction of the arrow
		 * @param size - The size of the arrow
		 * @param fg - The foreground color of the arrow
		 * @param bg - The background color of the arrow
		 */
		void draw_arrow(int16_t x, int16_t y, Direction d, uint8_t size, int16_t fg, int16_t bg);

		/**
		 * @brief Initializes the display
		 */
		void init();

		/**
		 * @brief Print string
		 * @param st - the string to print
		 * @param x - the x coordinate
		 * @param y - the y coordinate
		 * @param xo - x origin of the print area
		 * @param yo - y origin of the print area
		 */
		size_t print(uint8_t *st, int16_t x, int16_t y, int16_t xo=0, int16_t yo=0) override;

		/**
		 * @brief Print a string in the working area. Advances the cursor to keep track of position
		 * @param s - String to print
		 * @param c - Font color to use.
		 * @param newline - True to add a carriage return
		 */
		void w_area_print(String s, uint16_t color, bool newline);

		/**
		 * @brief Tests the display by going through a routine of drawing various
		 * shapes and information
		 */
		void test();

		/**
		 * @brief Writes the currently selected Axis into the display
		 * @param axis - The value for the axis to print
		 */
		void write_axis(Axis axis);

		/**
		 * @brief Writes the battery status to the display
		 * @param status - The battery status containing information such as state of charge, time to empty, etc.
		 */
		void write_battery_status(BatteryStatus_t status);

		/**
		 * @brief Writes the last command into the display
		 * @param c - the command name.
		 */
		void write_command(String c);

		/**
		 * @brief Writes the connection status to the display
		 * @param has_usb - Whether the device has USB connectivity
		 * @param has_wifi - Whether the device has WiFi connectivity
		 * @param has_bt - Whether the device has Bluetooth connectivity
		 */
		void write_connection_status(bool has_usb, bool has_wifi, bool has_bt);

		/**
		 * @brief Writes emergency indicator to the disaply
		 * @param has_emergency - Whether there is an emergency or not.
		*/
		void write_emergency(bool has_emergency);

		/**
		 * @brief Writes the current feed selection to the display
		 * @param feed - The value for the feed selection to write
		 */
		void write_feed(float feed);

		/**
		 * @brief Writes a status message to  the display
		 * @param format - format sting for the message.
		 * @param ... Argument list for the token replacement in the format string.
		 */
		void write_status(const String &format, ...);

		/**
		 * @brief Writes the current x position to the display
		 * @param x - The value for the x position to write
		 */
		void write_x(float x);

		/**
		 * @brief Writes the current y position to the display
		 * @param y - The value for the y position to write
		 */
		void write_y(float y);

		/**
		 * @brief Writes the current z position to the display
		 * @param z - The value for the z position to write
		 */
		void write_z(float z);	

	protected:

		/**
		 * @brief Implements scrolling for partial screen, both horizontally and vertically
		 * @param x - the top left of the scrolling area x coordinate
		 * @param y - the top left of the scrolling area y coordinate
		 * @param w - the width of the scroll area
		 * @param h - the height of the scorll area
		 * @param dx - the ammount to scroll into the x direction
		 * @param dy - the ammount to scroll tino the y direction
		 * @param bufh - the upper page buffer. Expected to be w*h*3/2
		 * @param bufl - the lower page buffer. Expected to be w*h*3/2
		 * @param inc - the scroll imcrement. Defaults to 1. 
		 * @remarks Since memory on the ESP32 is limited, the page buffer is split into an upper and lower area
		 * to accomodate an scrolling window of about 280*180 (or a bit larger). The buffers needs to be allocated to 
		 * be each w*h/2*3 (since each pixel is represented by 3 bytes). Additionally, it is important that the dy 
		 * parameter is divisible by two and that dy/2 is divisible by inc. 
		 */
		void window_scroll(int16_t x, int16_t y, int16_t wid, int16_t ht, int16_t dx, int16_t dy, uint8_t *bufh, uint8_t *bufl, uint8_t increment=1);

	private: 
		/**
		 * @brief Counts the number of lines in a string (based on the number of newline characters)
		 * @param s - The string to count lines in
		 * @return The number of lines in the string
		 */
		int count_lines(const String& s);

		/**
		 * @brief Prints a time string in the format of dd hh mm to the display
		 *
		 * @param time_seconds - The time in seconds to convert to a string and print
		 * @param x - The x coordinate to print the string at
		 * @param y - The y coordinate to print the string at
		 */
		void print_time_string(uint32_t time_seconds, uint16_t x, uint16_t y);

		uint16_t w_area_x1;
		uint16_t w_area_y1;
		uint16_t w_area_x2;
		uint16_t w_area_y2;
		uint16_t w_area_cursor_x;
		uint16_t w_area_cursor_y;
		uint8_t* buf1 = nullptr;
		uint8_t* buf2 = nullptr;
		bool w_area_initialized = false;
};

#endif