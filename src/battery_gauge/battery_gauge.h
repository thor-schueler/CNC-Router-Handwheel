// Copyright (c) Thor Schueler. All rights reserved.
// SPDX-License-Identifier: MIT

#include "Arduino.h"
#include <Wire.h>
#include <driver/i2c.h>
#include "battery_status.h"
#include "../wheel/wheel.h"

#ifndef _BATTERY_GAUGE_H_
#define _BATTERY_GAUGE_H_

#define MAX17048_ADDRESS 0x36
#define MAX17048_INT_PIN 34

#define MCP73871_C_PIN 39
#define MCP73871_D_PIN 36

/**
 * @brief Implements the batter gauge functionality using the MAX17048 fuel gauge IC. This is a singleton pattern implementation. 
 * Obtain the instance using BatteryGauge::get_instance()
 * 
 */
class BatteryGauge
{
    public:

        /**
         * @brief Gets the instance of the Battery Gauge Singleton.
         * @returns Instance of the Battery Gauge Singleton
         */
        static BatteryGauge* get_instance();

        /** 
         * @brief Gets the status of the battery.
         * @returns Current battery status.
         */
        static BatteryStatus_t get_status() { return BatteryGauge::get_instance()->_status; }

        /**
         * @brief Prints the battery status to the serial console for debugging purposes.
         */
        static void print_status();

        /** 
         * @brief Sets the wheel instance for the battery gauge.
         * @param wheel - Pointer to the wheel instance.
         */
        static void set_wheel_instance(Wheel* wheel){ BatteryGauge::get_instance()->_wheel = wheel;  }

    protected:

        /**
         * @brief Task runner for the monitoring task.
         * @param args - Task arguments, generally epxected to be an instance of the BatteryGauge class. 
         */
        static void runner(void* args);

        /** 
         * @brief Gets the voltage from the MAX17048 fuel gauge.
         * @returns Voltage in volts.
         */
        float max17048_getVoltage();

        /** 
         * @brief Gets the state of charge from the MAX17048 fuel gauge.
         * @returns State of charge as a percentage.
         */
        float max17048_getSOC();
        
        /** 
         * @brief Gets the current rate from the MAX17048 fuel gauge.
         * @returns Current rate in mA.
         */
         float max17048_getCRate();
        
        /** 
         * @brief Gets the status from the MAX17048 fuel gauge.
         * @returns Status register value.
         */
        uint16_t max17048_getStatus();
        
        /** 
         * @brief Gets the version from the MAX17048 fuel gauge.
         * @returns Version register value.
         */
        uint16_t max17048_getVersion();
        
        /** 
         * @brief Performs a quick start on the MAX17048 fuel gauge.
         */
        void max17048_quickStart() ;
        
        /** 
         * @brief Resets the MAX17048 fuel gauge.
         */
        void max17048_reset() ;
        
        /** 
         * @brief Sets the voltage alert thresholds on the MAX17048 fuel gauge.
         * @param min_mV - Minimum voltage threshold in millivolts.
         * @param max_mV - Maximum voltage threshold in millivolts.
         */
        void max17048_setVoltageAlert(float min_mV, float max_mV);
        
        /** 
         * @brief Initializes the MAX17048 fuel gauge.
         */
        void max17048_init();

    private:

        /**
         * @brief Generates a new instance of the Battery Gauge class.  
         */
        BatteryGauge();
        
        /** 
         * @brief Reads a 16-bit value from the MAX17048 fuel gauge.
         * @param reg - Register address to read from.
         * @returns 16-bit value read from the register.
         */
        uint16_t max17048_read16(uint8_t reg);
        
        /** 
         * @brief Writes a 16-bit value to the MAX17048 fuel gauge.
         * @param reg - Register address to write to.
         * @param value - Value to write to the register.
         */
        void max17048_write16(uint8_t reg, uint16_t value);

        /**
         * @brief Checks if the I2C bus is initialized.
         * @param port - I2C port to check.
         * @returns True if initialized, false otherwise.
         */
        bool i2c_is_initialized(i2c_port_t port = I2C_NUM_0);


        static BatteryGauge* _instance;
        static Wheel* _wheel;
        TaskHandle_t _task_handle;
        BatteryStatus_t _status;

};


#endif