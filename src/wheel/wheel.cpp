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
    pinMode(WHEEL_A, INPUT);
    pinMode(WHEEL_B, INPUT);
    pinMode(TOUCH_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH);

    Logger.Info(F("....Attach event receivers for GPIO"));
    attachInterrupt(digitalPinToInterrupt(AXIS_X), std::bind(&Wheel::handle_axis_change, this), FALLING);
    attachInterrupt(digitalPinToInterrupt(AXIS_Y), std::bind(&Wheel::handle_axis_change, this), FALLING);
    attachInterrupt(digitalPinToInterrupt(AXIS_Z), std::bind(&Wheel::handle_axis_change, this), FALLING);
    attachInterrupt(digitalPinToInterrupt(EMS), std::bind(&Wheel::handle_ems_change, this), CHANGE);

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
    _display_mutex = xSemaphoreCreateBinary(); 
    xSemaphoreGive(_display_mutex);

    Logger.Info("....Create various tasks");
    xTaskCreatePinnedToCore(extended_GPIO_watcher, "extendedGPIOWatcher", 16384, this, 1, &_extendedGPIOWatcher, 0);
    xTaskCreatePinnedToCore(display_runner, "displayRunner", 16384, this, 1, &_displayRunner, 0);

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
                        //Serial.write("\n");
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
            _this->_display->write_axis(_this->_selected_axis);
            _this->_display->write_feed(_this->_selected_feed);
            _this->_display->write_x(_this->_x);
            _this->_display->write_y(_this->_y);
            _this->_display->write_z(_this->_z);
            _this->_display->write_emergency(_this->_has_emergency);
            xSemaphoreGive(_this->_display_mutex);
        }
        vTaskDelay(10);
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
}
