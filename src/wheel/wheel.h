// Copyright (c) Thor Schueler. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef _WHEEL_H_
#define _WHEEL_H_

#include <unordered_map>
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

typedef struct Command
{
    String _name_on; 
    String _command_on;
    String _name_off;
    String _command_off;
} Command_t;

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
        static std::unordered_map<uint8_t, Command_t> Commands;
        int16_t _wheel_position = 0x0;


    protected:

        static void on_PCF8575_input_changed();
        static void extended_GPIO_watcher(void* args);
        static void display_runner(void* args);
        static void wheel_runner(void* args);

        void IRAM_ATTR handle_axis_change(); 
        void IRAM_ATTR handle_ems_change(); 
        void IRAM_ATTR handle_encoder_change();



    private: 
        static bool _key_changed;
        String format_string(const char* format, ...);      

        PCF8575 *_pcf8575 = nullptr;
        DISPLAY_Wheel *_display = nullptr;

        TaskHandle_t _extendedGPIOWatcher;
        TaskHandle_t _displayRunner;
        TaskHandle_t _encoderWatcher;
        TaskHandle_t _wheelRunner;

        SemaphoreHandle_t _display_mutex;
        
        float _x = 0.0;
        float _y = 0.0;
        float _z = 0.0;
        float _selected_feed = Feed::NANO;
        bool _has_emergency = false;
        uint16_t _button_state = 0xff;
        uint16_t _command_state = 0x00;

        int8_t _direction = 0;
        int16_t _wheel_encoded = 0x0;
        Axis _selected_axis = Axis::X;
};

#endif