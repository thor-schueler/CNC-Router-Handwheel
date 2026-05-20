// Copyright (c) Thor Schueler. All rights reserved.
// SPDX-License-Identifier: MIT
/*
This is the core GUI library for the TFT display, providing a common
set of graphics primitives (points, lines, circles, etc.).  It needs to be
paired with a hardware-specific library for each display device we carry
(to handle the lower-level functions).

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <Arduino.h>
#include <Wire.h>
#include <driver/i2c.h>

#include "battery_gauge.h"
#include "../logging/SerialLogger.h"

BatteryGauge* BatteryGauge::_instance = nullptr;
DISPLAY_Wheel* BatteryGauge::_display = nullptr;

/**
 * @brief Gets the instance of the Battery Gauge Singleton.
 * @returns Instance of the Battery Gauge Singleton
 */
BatteryGauge* BatteryGauge::get_instance()
{
    if (BatteryGauge::_instance == nullptr)
    {
        BatteryGauge::_instance = new BatteryGauge();
    }
    return BatteryGauge::_instance;
}

/**
 * @brief Generates a new instance of the Battery Gauge class.
 */
BatteryGauge::BatteryGauge()
{
    Logger.Info(F("Initializing Battery Gauge"));
    Logger.Info_f(F("....Initializing interrupt GPIO on ping %d"), MAX17048_INT_PIN);
    pinMode(MAX17048_INT_PIN, INPUT);
    Logger.Info(F("....Initializing MAX17048 fuel gauge"));
    max17048_init();
    Logger.Info(F("....Starting Battery Gauge Task"));
    xTaskCreatePinnedToCore(runner, "battery_gauge", 2304, this, 1, nullptr, 1);
    Logger.Info(F("Battery Gauge initialization complete."));
}

/**
 * @brief Task runner for the monitoring task.
 * @param args - Task arguments, generally expected to be an instance of the BatteryGauge class.
 */
void BatteryGauge::runner(void* args)
{
    BatteryGauge* _this = static_cast<BatteryGauge*>(args);
    vTaskDelay(pdMS_TO_TICKS(100));

    while (true)
    {
        uint16_t status = _this->max17048_getStatus();
        float vcell = _this->max17048_getVoltage();
        float soc = _this->max17048_getSOC();
        float c_rate = _this->max17048_getCRate();
        bool c_pin = digitalRead(MCP73871_C_PIN);
        bool d_pin = digitalRead(MCP73871_D_PIN);

        _this->_status.charging = !c_pin;
        _this->_status.precondition = !c_pin && !d_pin;
        _this->_status.fast_charging = !c_pin && d_pin;        
        _this->_status.vcell = vcell;
        _this->_status.c_rate = c_rate;
        _this->_status.state_of_charge = static_cast<uint8_t>(constrain(roundf(soc), 0.0f, 100.0f));
        _this->_status.time_to_empty =  c_rate > 0.0f ? INFINITY : (c_rate <= -0.1f ? soc /c_rate * -3600.0f : (soc <= 3 ? 0 : INFINITY)); // in seconds
        _this->_status.time_to_full = c_rate > 0.0f ? (c_rate >= 0.1f ? (100-soc) /c_rate * 3600.0f : (soc >= 90 ? 0 : INFINITY)) : INFINITY; // in seconds
        _this->_status.battery_present = (vcell > 2500 && status != 0xFFFF) || !(c_pin && d_pin);
        _this->print_status();

        if(_this->_display != nullptr)
        {
            _this->_display->write_battery_status(_this->_status);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

/**
 * @brief Prints the battery status to the serial console for debugging purposes.
 */
void BatteryGauge::print_status()
{
    BatteryGauge* g = BatteryGauge::get_instance();
    Logger.Info(F("Battery Gauge Status:"));
    Logger.Info_f(F("...Voltage: %.2f mV"), g->_status.vcell);
    Logger.Info_f(F("...State of Charge: %d%%"), g->_status.state_of_charge);
    Logger.Info_f(F("...C-Rate: %.2f %%/h"), g->_status.c_rate);
    Logger.Info_f(F("...Time to Empty: %s"), isinf(g->_status.time_to_empty) ? "inf" : String(g->_status.time_to_empty / 3600.0f, 2) + "h");
    Logger.Info_f(F("...Time to Full: %s"), isinf(g->_status.time_to_full) ? "inf" : String(g->_status.time_to_full / 3600.0f, 2) + "h");
    Logger.Info_f(F("...Charging: %s"), g->_status.charging ? "Yes" : "No");
    Logger.Info_f(F("...Precondition: %s"), g->_status.precondition ? "Yes" : "No");
    Logger.Info_f(F("...Fast Charging: %s"), g->_status.fast_charging   ? "Yes" : "No");
    Logger.Info_f(F("...Battery Present: %s"), g->_status.battery_present ? "Yes" : "No");
}

/**
 * @brief Checks if the I2C bus is initialized.
 * @param port - I2C port to check.
 * @returns True if initialized, false otherwise.
 */
bool BatteryGauge::i2c_is_initialized(i2c_port_t port)
{
    //i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    //esp_err_t err = i2c_master_start(cmd);
    //i2c_cmd_link_delete(cmd);
    //return (err != ESP_ERR_INVALID_STATE);
    return true;
}

/**
 * @brief Gets the voltage from the MAX17048 fuel gauge.
 * @returns Voltage in millivolts.
 */
float BatteryGauge::max17048_getVoltage()
{
    uint16_t raw = max17048_read16(0x02);
    if (raw == 0xFFFF) return 0.0f;
    raw >>= 4;
    return raw * 1.25f;
}

/**
 * @brief Gets the state of charge from the MAX17048 fuel gauge.
 * @returns State of charge as a percentage.
 */
float BatteryGauge::max17048_getSOC()
{
    uint16_t raw = max17048_read16(0x04);
    if (raw == 0xFFFF) return 0.0f;

    uint8_t whole = static_cast<uint8_t>(raw >> 8);
    uint8_t fraction = static_cast<uint8_t>(raw & 0xFF);
    return whole + (fraction / 256.0f);
}

/**
 * @brief Gets the current rate from the MAX17048 fuel gauge.
 * @returns Current charge/discharge rate in % per hour.
 */
float BatteryGauge::max17048_getCRate()
{
    int16_t raw = (int16_t)max17048_read16(0x16);
    return raw / 100.0f; 
}

/**
 * @brief Gets the status from the MAX17048 fuel gauge.
 * @returns Status register value.
 */
uint16_t BatteryGauge::max17048_getStatus()
{
    return max17048_read16(0x1A);
}

/**
 * @brief Gets the version from the MAX17048 fuel gauge.
 * @returns Version register value.
 */
uint16_t BatteryGauge::max17048_getVersion()
{
    return max17048_read16(0x08);
}

/**
 * @brief Performs a quick start on the MAX17048 fuel gauge.
 */
void BatteryGauge::max17048_quickStart()
{
    max17048_write16(0x06, 0x4000);
    Logger.Info(F("BatteryGauge: Fuel gauge quick start initialized."));
}

/**
 * @brief Resets the MAX17048 fuel gauge.
 */
void BatteryGauge::max17048_reset()
{
    max17048_write16(0xFE, 0x5400);
    Logger.Info(F("BatteryGauge: Fuel gauge reset complete."));
}

/**
 * @brief Sets the voltage alert thresholds on the MAX17048 fuel gauge.
 * @param min_mV - Minimum voltage threshold in millivolts.
 * @param max_mV - Maximum voltage threshold in millivolts.
 */
void BatteryGauge::max17048_setVoltageAlert(float min_mV, float max_mV)
{
    uint8_t min_raw = min_mV / 20.0f;  // 20 mV/LSB
    uint8_t max_raw = max_mV / 20.0f;
    uint16_t reg = (max_raw << 8) | min_raw;
    max17048_write16(0x14, reg);
    Logger.Info_f(F("BatteryGauge: Voltage alert configuration complete: (min=%.1f, max=%.1f)"), min_mV, max_mV);
}

/**
 * @brief Initializes the MAX17048 fuel gauge.
 */
void BatteryGauge::max17048_init()
{
    if(!i2c_is_initialized())
    {
        Wire.begin();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    uint16_t version = max17048_getVersion();
    if (version == 0xFFFF)
    {
        Logger.Error(F("BatteryGauge: MAX17048 not detected on I2C bus."));
        return;
    }
    Logger.Info_f(F("BatteryGauge: Detected MAX17048 version 0x%04X"), version);

    vTaskDelay(pdMS_TO_TICKS(10));
    float v = max17048_getVoltage();
    if (v > 3000 && v < 4500) 
    {
        max17048_quickStart();
        Logger.Info(F("BatteryGauge: MAX17048 initialized with QuickStart."));
    }
    else
    {
        Logger.Info(F("BatteryGauge: MAX17048 voltage reading out of range, skipping QuickStart."));
    }
}

/**
 * @brief Reads a 16-bit value from the MAX17048 fuel gauge.
 * @param reg - Register address to read from.
 * @returns 16-bit value read from the register.
 */
uint16_t BatteryGauge::max17048_read16(uint8_t reg)
{
    Wire.beginTransmission(MAX17048_ADDRESS);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0)
    {
        return 0xFFFF;
    }

    if (Wire.requestFrom(MAX17048_ADDRESS, static_cast<uint8_t>(2)) != 2)
    {
        return 0xFFFF;
    }

    uint16_t msb = static_cast<uint16_t>(Wire.read());
    uint16_t lsb = static_cast<uint16_t>(Wire.read());
    return (msb << 8) | lsb;
}

/**
 * @brief Writes a 16-bit value to the MAX17048 fuel gauge.
 * @param reg - Register address to write to.
 * @param value - Value to write to the register.
 */
void BatteryGauge::max17048_write16(uint8_t reg, uint16_t value)
{
    Wire.beginTransmission(MAX17048_ADDRESS);
    Wire.write(reg);
    Wire.write(static_cast<uint8_t>(value >> 8));
    Wire.write(static_cast<uint8_t>(value & 0xFF));
    Wire.endTransmission();
}

