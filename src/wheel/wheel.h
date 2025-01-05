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

/**
 * @brief This structure contains information for a particular command. 
 */
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

        /**
         * @brief Map containing the available commands for the CNC router 
         */
        static std::unordered_map<uint8_t, Command_t> Commands;
        
        /**
         * @brief Writes a status message to the display
         * @param format - the format string
         * @param ... - variable argument list  
         */
        void write_status_message(const String &format, ...);

    protected:

        /**
         * @brief Event handler handling input change events on the PCF8575 
         */
        static void on_PCF8575_input_changed();

        /**
         * @brief Task function to process changes to the inputs on the PCF8575
         * GPIO extender. This funtions runs an endless blocking loop, waiting for notification 
         * from on_PCF8575_input_changed upon which it evaluates the inputs and performs
         * the appropriate actions. 
         * @param args - pointer to task arguments
         */
        static void extended_GPIO_watcher(void* args);

        /**
         * @brief Task function managing the display
         * @param args - pointer to task arguments
         */
        static void display_runner(void* args);

        /**
         * @brief Task function managing wheel movements. This task runs an endless blocking loop,
         * waiting for notification from handle_encoder_change upon which it will process
         * and execute the appropriate action.
         * @param args - pointer to task arguments 
         */
        static void wheel_runner(void* args); 

        /**
         * @brief Task function managing changes from the EMS button. This task runs an endless 
         * blocking loop, waiting for notification from handle_ems_change for perform the appropriate
         * action
         * @param args - pointer to task arguments 
         */
        static void ems_change_runner(void* args);

        /**
         * @brief Event handler monitoring the Axus GPIOs 
         */
        void IRAM_ATTR handle_axis_change(); 

        /**
         * @brief Event handler monitoring the Emergency Shutdown Button 
         */
        void IRAM_ATTR handle_ems_change(); 

        /**
         * @brief Event handler watching the Quadradure encoder GPIOs.
         */
        void IRAM_ATTR handle_encoder_change();

    private: 

        /**
         * @brief Formats a string, essentially a wrapper for vnsprintf
         * @param format - format string
         * @param ... - variable argument list 
         */
        String format_string(const char* format, ...);

        static bool _key_changed;      
        DISPLAY_Wheel *_display = nullptr;
        PCF8575 *_pcf8575 = nullptr;
    
        TaskHandle_t _extendedGPIOWatcher;
        TaskHandle_t _displayRunner;
        TaskHandle_t _encoderWatcher;
        TaskHandle_t _wheelRunner;
        TaskHandle_t _emsChangeRunner;

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
        int16_t _wheel_position = 0x0;
        Axis _selected_axis = Axis::X;
};

#endif