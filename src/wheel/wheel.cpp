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

#include "wheel.h"
#include "../logging/SerialLogger.h"

bool Wheel::key_changed = false;

/**
 * @brief Creates a new instance of Wheel
 */
Wheel::Wheel()
{
    Logger.Info(F("Startup"));
    Logger.Info(F("....Initialize Display"));
    display = new DISPLAY_Wheel();
    display->set_rotation(3);
    display->init();

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


    Logger.Info(F("....Initialize GPIO Multiplexer"));
    pcf8575 = new PCF8575(PCF8575_ADDRESS, PCF8575_INT_PIN, Wheel::on_PCF8575_input_changed);
    pcf8575->pinMode(P0, INPUT);
    pcf8575->pinMode(P1, INPUT);
    pcf8575->pinMode(P2, INPUT);
    pcf8575->pinMode(P3, INPUT);
    pcf8575->pinMode(P4, INPUT);
    pcf8575->pinMode(P5, INPUT);
    pcf8575->pinMode(P6, INPUT);
    pcf8575->pinMode(P7, INPUT);
    pcf8575->pinMode(P8, INPUT);
    pcf8575->pinMode(P9, INPUT);
    pcf8575->pinMode(P10, INPUT);
    pcf8575->pinMode(P11, INPUT);
    pcf8575->pinMode(P12, INPUT);
    pcf8575->pinMode(P13, INPUT);
    pcf8575->pinMode(P14, INPUT);
    pcf8575->pinMode(P15, INPUT);
    pcf8575->begin();

    Logger.Info("Startup done");
}

void Wheel::on_PCF8575_input_changed()
{
    Wheel::key_changed = true;
}

void Wheel::process_input_change()
{
    if(Wheel::key_changed)
    {
        Wheel::key_changed = false;
        PCF8575::DigitalInput di = pcf8575->digitalReadAll();
        String s = String(di.p0) + "-" + String(di.p1) + "-" + String(di.p2) + "-" + String(di.p3) + "-" + 
                    String(di.p4) + "-" + String(di.p5) + "-" + String(di.p6) + "-" + String(di.p7) + "-" + 
                    String(di.p8) + "-" + String(di.p9) + "-" + String(di.p10) + "-" + String(di.p11) + "-" + 
                    String(di.p12) + "-" + String(di.p13) + "-" + String(di.p14) + "-" + String(di.p15) +"\n";
        display->w_area_print(s, true); 
    }
}