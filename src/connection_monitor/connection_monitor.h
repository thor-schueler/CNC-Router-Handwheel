// Copyright (c) Thor Schueler. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef _CONNECTION_MONITOR_H_
#define _CONNECTION_MONITOR_H_

#include <Arduino.h>
#include <BluetoothSerial.h>
#include "../display/display_wheel.h"


#define MONITORING_PERIOD 1000

/**
 * @brief Implements monitoring functionality for wifi, bluetooth and usb connections
 * @details This is a singleton pattern implementation. Obtain the instance using ConnectionMonitor::get_instance()
 */
class ConnectionMonitor
{
    public:

        /**
         * @brief Gets the instance of Connection Monitor Singleton.
         * @returns Instance of the Connection Monitor Singleton
         */
        static ConnectionMonitor* get_instance();

        /**
         * @brief Sets the instance of the BluetoothSerial class to use to monitor Bluetooth connectivity.
         * @param bt - Pointer to a BluetoothSerial instance. 
         */
        static void set_bluetooth_instance(BluetoothSerial* bt);

        /**
         * @brief Sets the instance of the display manager to use to update
         * the display.
         * @param display - Pointer to a DISPLAY_Wheel instance. 
         */
        static void set_display_instance(DISPLAY_Wheel* display);

    protected:

        /**
         * @brief Task runner for the monitoring task.
         * @param args - Task arguments 
         */
        static void runner(void* args);

    private:

        /**
		 * @brief Generates a new instance of the ConnectionMonitor class.  
		 */
        ConnectionMonitor();

        bool has_usb = false;
        bool has_wifi = false;
        bool has_bt = false;
        
        TaskHandle_t monitor = NULL;
        
        static ConnectionMonitor* instance;
        DISPLAY_Wheel* display = nullptr;
        BluetoothSerial* bt = nullptr;

};


#endif