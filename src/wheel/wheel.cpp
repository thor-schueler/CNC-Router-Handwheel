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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <FunctionalInterrupt.h>
#include "wheel.h"
#include "../logging/SerialLogger.h"

bool Wheel::_key_changed = false;
static Wheel *_instance = nullptr;

std::unordered_map<uint8_t, Command_t>Wheel::Commands = {
    {0, {"Zero Z", "G92 Z0 ; Zero Z Axis", "", ""}},
    {1, {"Zero XY", "G92 X0Y0 ; Zero XY", "", ""}},
    {2, {"Probe Z", "G4 P3;G21G91G38.2Z-30F80; G0Z1; G38.2Z-2F10;\n    G92 Z0; G0Z5M30 ; Probe Z", "", ""}},
    {3, {"Homing", "$H ; Homing cycle", "", ""}},
    {4, {"Origin", "; Got to WCS origin", "", ""}},
    {5, {"Start Spindle", "M3 S6000 ; Start the spindle", "Stop Spindle", "M5; Stop the spindle"}},
    {6, {"Reset", "; Reset the machine", "", ""}},
    {7, {"Unlock", "; Unlock the machine", "", ""}},
    {8, {"NA", "; Command 9 not defined", "", ""}},
    {9, {"NA", "; Command 10 not defined", "", ""}},
    {10, {"NA", "; Command 11 not defined", "", ""}},
    {11, {"NA", "; Command 12 not defined", "", ""}}    
};

/**
 * @brief Creates a new instance of Wheel
 */
Wheel::Wheel()
{
    Logger.Info(F("Startup"));
    Logger.Info(F("....Initialize Display"));
    _display = new DISPLAY_Wheel();
    _display->set_rotation(3);
    _display->init();
    _instance = this;

    Logger.Info(F("....Inititialize GPIO pins"));
    pinMode(AXIS_Z, INPUT_PULLUP);
    pinMode(AXIS_X, INPUT_PULLUP);
    pinMode(AXIS_Y, INPUT_PULLUP);
    pinMode(EMS, INPUT_PULLUP);
    pinMode(WHEEL_A, INPUT_PULLUP);
    pinMode(WHEEL_B, INPUT_PULLUP);
    pinMode(TOUCH_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH);

    Logger.Info(F("....Attach event receivers for GPIO"));
    attachInterrupt(digitalPinToInterrupt(AXIS_X), std::bind(&Wheel::handle_axis_change, this), FALLING);
    attachInterrupt(digitalPinToInterrupt(AXIS_Y), std::bind(&Wheel::handle_axis_change, this), FALLING);
    attachInterrupt(digitalPinToInterrupt(AXIS_Z), std::bind(&Wheel::handle_axis_change, this), FALLING);
    attachInterrupt(digitalPinToInterrupt(EMS), std::bind(&Wheel::handle_ems_change, this), CHANGE);
    attachInterrupt(digitalPinToInterrupt(WHEEL_A), std::bind(&Wheel::handle_encoder_change, this), CHANGE); 
    attachInterrupt(digitalPinToInterrupt(WHEEL_B), std::bind(&Wheel::handle_encoder_change, this), CHANGE);

    Logger.Info(F("....Initialize GPIO Multiplexer"));
    _pcf8575 = new PCF8575(PCF8575_ADDRESS, PCF8575_INT_PIN, Wheel::on_PCF8575_input_changed);
    for(int i=0; i<16; i++) _pcf8575->pinMode(i, INPUT);
    _pcf8575->begin();
    PCF8575::DigitalInput di = _pcf8575->digitalReadAll();
    if(!di.p12) _selected_feed = Feed::FULL;
    else if(!di.p13) _selected_feed = Feed::MILLI;
    else if(!di.p14) _selected_feed = Feed::MICRO;
    else _selected_feed = Feed::NANO;

    Logger.Info("....Initializing Axis and Feed Values");  
    if(!digitalRead(AXIS_X)) _selected_axis = Axis::X;
    else if(!digitalRead(AXIS_Y)) _selected_axis = Axis::Y;
    else if(!digitalRead(AXIS_Z)) _selected_axis = Axis::Z;  
    _has_emergency = digitalRead(EMS);

    Logger.Info(F("....Generating Mutexes"));
    _display_mutex = xSemaphoreCreateBinary();  xSemaphoreGive(_display_mutex);

    Logger.Info("....Create various tasks");
    xTaskCreatePinnedToCore(extended_GPIO_watcher, "extendedGPIOWatcher", 2048, this, 1, &_extendedGPIOWatcher, 0);
    xTaskCreatePinnedToCore(display_runner, "displayRunner", 8192, this, 1, &_displayRunner, 0);
    xTaskCreatePinnedToCore(wheel_runner, "wheelRunner", 2048, this, 1, &_wheelRunner, 0);

    Logger.Info("Startup done");
}

void Wheel::on_PCF8575_input_changed()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(_instance->_extendedGPIOWatcher, &xHigherPriorityTaskWoken); 
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void Wheel::extended_GPIO_watcher(void* args)
{
    Wheel *_this = reinterpret_cast<Wheel *>(args);
    for (;;) 
    { 
        // Wait for the notification to come from the event handler
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        PCF8575::DigitalInput di = _this->_pcf8575->digitalReadAll();
        if(!di.p12) _this->_selected_feed = Feed::FULL;
        else if(!di.p13) _this->_selected_feed = Feed::MILLI;
        else if(!di.p14) _this->_selected_feed = Feed::MICRO;
        else _this->_selected_feed = Feed::NANO;

        if(!(di.p0 & di.p1 & di.p2 & di.p3 & di.p4 & di.p5 & di.p6 & di.p7 & di.p8 & di.p9 & di.p10 & di.p11))
        {
            uint16_t button_state = 0xf000;
            button_state |= (di.p0 & 0x01) << 0; 
            button_state |= (di.p1 & 0x01) << 1; 
            button_state |= (di.p2 & 0x01) << 2; 
            button_state |= (di.p3 & 0x01) << 3; 
            button_state |= (di.p4 & 0x01) << 4; 
            button_state |= (di.p5 & 0x01) << 5; 
            button_state |= (di.p6 & 0x01) << 6; 
            button_state |= (di.p7 & 0x01) << 7; 
            button_state |= (di.p8 & 0x01) << 8; 
            button_state |= (di.p9 & 0x01) << 9; 
            button_state |= (di.p10 & 0x01) << 10; 
            button_state |= (di.p11 & 0x01) << 11; 
            if(button_state != _this->_button_state)
            {
                _this->_button_state = button_state;
                for(int i=0; i<12; i++)
                {
                    if(!(button_state & (1<<i)))
                    {
                        String n, c;
                        if(_this->_command_state & (1<<i))
                        {
                            n = Commands[i]._name_off == "" ? Commands[i]._name_on : Commands[i]._name_off;
                            c = Commands[i]._command_off == "" ? Commands[i]._command_on : Commands[i]._command_off;
                        }
                        else
                        {
                            n = Commands[i]._name_on;
                            c = Commands[i]._command_on;
                        }
                        if (xSemaphoreTake(_this->_display_mutex, portMAX_DELAY) == pdTRUE) 
                        {
                            _this->_display->w_area_print(c, true); 
                            _this->_display->write_command(n); 
                            xSemaphoreGive(_this->_display_mutex);
                        }
                        // write command to serial
                        Serial.println(c);
                        Serial.flush();
                        _this->_command_state ^= (1 << i);
                        _this->_button_state |= (1 << i);                        
                    }
                }
            }
        }
    }
}

void Wheel::display_runner(void* args)
{
    Wheel *_this = reinterpret_cast<Wheel *>(args);
    for (;;) 
    { 
        if (xSemaphoreTake(_this->_display_mutex, portMAX_DELAY) == pdTRUE) 
        {
            int16_t cf = _this->_display->RGB_to_565(0x00, 0xff, 0x00);
            int16_t cb = _this->_display->RGB_to_565(0xff, 0x00, 0x00);
            int16_t cn = _this->_display->RGB_to_565(0x44, 0x44, 0x44);
            
            

            _this->_display->write_axis(_this->_selected_axis);
            _this->_display->write_feed(_this->_selected_feed);
            _this->_display->write_emergency(_this->_has_emergency);
            _this->_display->write_x(_this->_x);
            _this->_display->write_y(_this->_y);
            _this->_display->write_z(_this->_z);
            _this->_display->draw_arrow(162, 14, Direction::LEFT, 3, _this->_selected_axis == Axis::X && _this->_direction == -1 ? cb : cn);
            _this->_display->draw_arrow(185, 14, Direction::RIGHT, 3, _this->_selected_axis == Axis::X && _this->_direction == 1 ? cf : cn);
            _this->_display->draw_arrow(305, 14, Direction::UP, 3, _this->_selected_axis == Axis::Y && _this->_direction == -1 ? cb : cn);
            _this->_display->draw_arrow(290, 14, Direction::DOWN, 3, _this->_selected_axis == Axis::Y && _this->_direction == 1 ? cf : cn);
            _this->_display->draw_arrow(415, 14, Direction::UP, 3, _this->_selected_axis == Axis::Z && _this->_direction == -1 ? cf : cn);
            _this->_display->draw_arrow(400, 14, Direction::DOWN, 3, _this->_selected_axis == Axis::Z && _this->_direction == 1 ? cb : cn);
            xSemaphoreGive(_this->_display_mutex);
        }
        vTaskDelay(10);
    }
}

String Wheel::format_string(const char* format, ...) 
{
    va_list args; 
    va_start(args, format); 
    
    // Determine the size of the formatted string 
    size_t size = vsnprintf(nullptr, 0, format, args) + 1; // Add space for null terminator 
    va_end(args); 
    
    // Create a buffer of the appropriate size
    std::vector<char> buffer(size); 
    
    // Format the string 
    va_start(args, format); 
    vsnprintf(buffer.data(), size, format, args); va_end(args); 
    return String(buffer.data());
}

void Wheel::wheel_runner(void* args)
{
    Wheel *_this = reinterpret_cast<Wheel *>(args);
    for (;;) 
    { 
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // this section is executed for every wheel position change.
        // To tranlsate into the CNC command, we need to use feed and axis
        switch(_this->_selected_axis)
        {
            case Axis::X:
                _this->_x += _this->_selected_feed * _this->_direction;
                break;
            case Axis::Y:
                _this->_y += _this->_selected_feed * _this->_direction;
                break;
            case Axis::Z:
                _this->_z += _this->_selected_feed * _this->_direction;
                break;             
        }
        
        String s = _this->format_string("G21G91%c%c%fF2000", 
                (char)_this->_selected_axis,
                _this->_direction == -1 ? '-': '+' ,
                _this->_selected_feed);
        Serial.println(s);
    }
}


void IRAM_ATTR Wheel::handle_ems_change()
{
    _has_emergency = digitalRead(EMS);
}

void IRAM_ATTR Wheel::handle_axis_change()
{    
    if(!digitalRead(AXIS_X)) _selected_axis = Axis::X;
    if(!digitalRead(AXIS_Y)) _selected_axis = Axis::Y;
    else if(!digitalRead(AXIS_Z)) _selected_axis = Axis::Z;
    _direction = 0;
}

void IRAM_ATTR Wheel::handle_encoder_change()
{
    static int8_t c = 0;
    static const int8_t enconder_state_table[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
    int MSB = digitalRead(WHEEL_A); // Most significant bit 
    int LSB = digitalRead(WHEEL_B); // Least significant bit 
    int encoded = (MSB << 1) | LSB; // Combine the two signals 
    if(encoded != _wheel_encoded)
    {
        int sum = (_wheel_encoded << 2) | encoded;  // Add the two previous bits 
        c += enconder_state_table[sum];
        if(c == 4 || c == -4)
        {
            _wheel_position += c == 4 ? 1 : -1;
            _direction = c > 0 ? 1 : -1;
            c = 0x0;
            _wheel_encoded = encoded;   // Update the last encoded value

            // Signal our job to run the axis....
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            vTaskNotifyGiveFromISR(_instance->_wheelRunner, &xHigherPriorityTaskWoken); 
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        else
        {
            _wheel_encoded = encoded;   // Update the last encoded value  
        }
    }
}