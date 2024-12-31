// Copyright (c) Thor Schueler. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef _WHEEL_H_
#define _WHEEL_H_

#include "Arduino.h"
#include "PCF8575.h"
#include "../display/display_wheel.h"

#define PCF8575_ADDRESS 0x20
#define PCF8575_INT_PIN 4

#define AXIS_X 18
#define AXIS_Y 17
#define AXIS_Z 35
    // originally we used GPIO16 for AXIS_Z, but this is an issue on 
    // EPS32 units that have PSRAM as GPIO16 serves as CS line for 
    // the PSRAM. Any transition from High to Low activates the chip
    // and hnags up the ESP32....

#define EMS 19
#define WHEEL_A 23
#define WHEEL_B 27

#define TOUCH_CS 33

/**
 * @brief Implements the basic wheel functionality
 */
class Wheel
{
    public:
        /**
         * @brief Creates a new instance of Wheel
         */
        Wheel();
        void process_input_change();
        
        DISPLAY_Wheel *display = nullptr;
    protected:
        static void on_PCF8575_input_changed();

    private: 
        static bool key_changed;
        PCF8575 *pcf8575 = nullptr;
        
        float x = 0.0;
        float y = 0.0;
        float z = 0.0;
        float selected_feed = Feed::NANO;
        Axis selected_axis = Axis::X;
};

#endif