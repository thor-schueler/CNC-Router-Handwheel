// Copyright (c) Microsoft Corporation. All rights reserved.
// Copyright (c) Thor Schueler. All rights reserved.
// SPDX-License-Identifier: MIT

#include <Arduino.h>
#include <WiFi.h>
#include <HardwareSerial.h>
#include "connection_monitor.h"

ConnectionMonitor* ConnectionMonitor::instance = nullptr;
DISPLAY_Wheel* ConnectionMonitor::display = nullptr;
BluetoothSerial* ConnectionMonitor::bt = nullptr;

/**
 * @brief Gets the instance of Connection Monitor Singleton.
 * @returns Instance of the Connection Monitor Singleton
 */
ConnectionMonitor* ConnectionMonitor::get_instance()
{
    if (instance == nullptr)
    {
        instance = new ConnectionMonitor();
    }
    return instance;
}

/**
 * @brief Generates a new instance of the ConnectionMonitor class.  
 */
ConnectionMonitor::ConnectionMonitor()
{
    bt->begin("CNC Pendant", false, false);
    xTaskCreatePinnedToCore(runner, "connection_monitor", 1024, this, 1, &monitor, 0);
}

/**
 * @brief Task runner for the monitoring task.
 * @param args - Task arguments 
 */
void ConnectionMonitor::runner(void* args)
{
    ConnectionMonitor* _this = (ConnectionMonitor*)args;
    while(true)
    {
        _this->has_wifi = WiFi.status() == WL_CONNECTED;
        if(Serial.available()==0) _this->has_usb = false;
        else
        {
            _this->has_usb = true;
            while(Serial.available() > 0 && Serial.peek() == 0x0) Serial.read(); 
                // Read the available handshake byte. If other bytes are waiting, leave them be....
        }
        if(ConnectionMonitor::bt->hasClient())
        {
            _this->has_bt = true;
        }
        else
        {
            _this->has_bt = false;
        }

        if(ConnectionMonitor::display != nullptr)
        {
            ConnectionMonitor::display->write_connection_status(_this->has_usb, _this->has_wifi, _this->has_bt);
        }
        vTaskDelay(MONITORING_PERIOD);
    }
    vTaskDelete(_this->monitor);
    _this->monitor=nullptr;
}

/**
 * @brief Sets the instance of the display manager to use to update
 * the display.
 * @param display - Pointer to a DISPLAY_Wheel instance. 
 */
void ConnectionMonitor::set_display_instance(DISPLAY_Wheel* display)
{
    ConnectionMonitor::display = display;
}