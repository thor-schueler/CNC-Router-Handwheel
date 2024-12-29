// Copyright (c) Thor Schueler. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef _DISPLAY_WHEEL_H_
#define _DISPLAY_WHEEL_H_

#include "Arduino.h"
#include "../display_spi/display_spi.h"

typedef enum { X, Y, Z } Axis;
struct Feed { 
	static constexpr float NANO = 0.001f; 
	static constexpr float MICRO = 0.01f; 
	static constexpr float MILLI = 0.1f; 
	static constexpr float FULL = 1.0f;
};

extern const unsigned char lcars[] PROGMEM;
extern const size_t lcars_size;

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
		 * @brief Initializes the display
		 */
		void init();

		void windowScroll(int16_t x, int16_t y, int16_t wid, int16_t ht, int16_t dx, int16_t dy, uint8_t *bufh, uint8_t *bufl, uint8_t increment=1);

		/**
		 * @brief Print a string in the working area. Advances the cursor to keep track of position
		 * @param s - String to print
		 * @param newline - True to add a carriage return
		 */
		void w_area_print(String s, bool newline);

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
		 * @brief Writes the current feed selection to the display
		 * @param feed - The value for the feed selection to write
		 */
		void write_feed(float feed);

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

	private: 
		uint16_t w_area_x1;
		uint16_t w_area_y1;
		uint16_t w_area_x2;
		uint16_t w_area_y2;
		uint16_t w_area_cursor_x;
		uint16_t w_area_cursor_y;
};

#endif