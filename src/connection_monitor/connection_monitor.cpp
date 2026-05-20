// Copyright (c) Microsoft Corporation. All rights reserved.
// Copyright (c) Thor Schueler. All rights reserved.
// SPDX-License-Identifier: MIT

#include <Arduino.h>
#include <WiFi.h>
#include <HardwareSerial.h>
#include "connection_monitor.h"
#include "../logging/SerialLogger.h"

ConnectionMonitor* ConnectionMonitor::instance = nullptr;

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
    Logger.Info(F("Initializing Connection Monitor"));
    Logger.Info(F("....Starting Connection Monitor Task"));
    xTaskCreatePinnedToCore(runner, "connection_monitor", 1024, this, 1, &monitor, 1);
    Logger.Info(F("Done."));
}

/**
 * @brief Task runner for the monitoring task.
 * @param args - Task arguments 
 */
void ConnectionMonitor::runner(void* args)
{
    ConnectionMonitor* _this = (ConnectionMonitor*)args;
    vTaskDelay(pdMS_TO_TICKS(100));
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
        if(_this->bt != nullptr && _this->bt->hasClient())
        {
            _this->has_bt = true;
        }
        else
        {
            _this->has_bt = false;
        }

        if(_this->display != nullptr)
        {
            _this->display->write_connection_status(_this->has_usb, _this->has_wifi, _this->has_bt);
        }
        vTaskDelay(pdMS_TO_TICKS(MONITORING_PERIOD));
    }
    vTaskDelete(_this->monitor);
    _this->monitor=nullptr;
}


/**
 * @brief Sets the instance of the BluetoothSerial class to use to monitor Bluetooth connectivity.
 * @param bt - Pointer to a BluetoothSerial instance. 
 */
void ConnectionMonitor::set_bluetooth_instance(BluetoothSerial* bt)
{
    if(bt == nullptr) Logger.Info(F("ConnectionMonitor: Attempted to set bluetooth instance to null pointer"));
    ConnectionMonitor::instance->bt = bt;
}

/**
 * @brief Sets the instance of the display manager to use to update
 * the display.
 * @param display - Pointer to a DISPLAY_Wheel instance. 
 */
void ConnectionMonitor::set_display_instance(DISPLAY_Wheel* display)
{
    if(display == nullptr) Logger.Info(F("ConnectionMonitor: Attempted to set display instance to null pointer"));
    ConnectionMonitor::instance->display = display;
}