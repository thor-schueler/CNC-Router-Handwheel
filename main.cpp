// Copyright (c) theRealThor. All rights reserved.
// SPDX-License-Identifier: MIT

/*
 * This is the firmware for a handwheel compatible with Candle 2.0. To use this, you need to run Candle 2.0 on 
 * your desktop, connect the ESP32 to your dektop via USB and configure Candle 2.0 to use the appropriate 
 * handwheel port to communicate with the ESP. 
 *   
 */

#define HIGH_WATER_MARK_LOOP_SKIP 120

#include "Arduino.h"
#include "ESP.h"
#include <SPI.h>

// C99 libraries
#include <cstdlib>
#include <string.h>
#include <time.h>

// local libraries
#include "src/logging/SerialLogger.h"
#include "src/display/display_wheel.h"
#include "src/wheel/wheel.h"

#define TELEMETRY_FREQUENCY_MILLISECS 120000
#define AP_ENABLE_PIN 5

//static Config config;
//static Conveyor *conveyor = NULL;

//static DynamicJsonDocument* twin = nullptr;
//static int telemetry_period = TELEMETRY_FREQUENCY_MILLISECS;
//static bool send_device_telemetry = true;
//static bool twin_ready = false;
//static bool has_wifi = true;
//static uint8_t high_watermark_loop_skip_count = 0;

#ifdef SET_LOOP_TASK_STACK_SIZE
SET_LOOP_TASK_STACK_SIZE(16384);
  //
  // This will only work with ESP-Arduino 2.0.7 or higher. 
  //
#endif

Wheel *wheel = NULL;
DISPLAY_Wheel *dw = NULL;
uint8_t* buf1 = nullptr;
uint8_t* buf2 = nullptr;
//uint8_t buf1[280*90*3];
//uint8_t buf2[280*90*3];

/**
 * @brief Performs system setup activities, including connecting to WIFI, setting time, obtaining the IoTHub info 
 * from DPS if necessary and connecting the IoTHub. Use this method to also register various delegates and command
 * handlers.
 * 
 */
void setup()
{
  int wifi_try_counter = 0;

  Logger.Info_f(F("Copyright 2024, Thor Schueler, Firmware Version: %s"), "0.00.00");
  Logger.Info_f(F("Loop task stack size: %i"), getArduinoLoopTaskStackSize());
  Logger.Info_f(F("Loop task stack high water mark: %i"), uxTaskGetStackHighWaterMark(NULL));
  Logger.Info_f(F("Total heap: %d"), ESP.getHeapSize()); 
  Logger.Info_f(F("Free heap: %d"), ESP.getFreeHeap()); 
  Logger.Info_f(F("Total PSRAM: %d"), ESP.getPsramSize()); 
  Logger.Info_f(F("Free PSRAM: %d"), ESP.getFreePsram());

  if(AP_ENABLE_PIN == 0)
  {
    //config.StartAP();
  }
  else
  {
    //Logger.Info_f(F("Checking Configuration AP Enable on pin GPIO%i."), AP_ENABLE_PIN);
    //pinMode(AP_ENABLE_PIN, INPUT_PULLUP); 
    //if(digitalRead(AP_ENABLE_PIN) == HIGH)
    //{
    //  Logger.Info_f(F("Config AP enabled. Pull GPIO%i low to disable the Config AP."), AP_ENABLE_PIN);
    //  config.StartAP();
    //}
    //else
    //{
    //  Logger.Info_f(F("Config AP disabled. Do not pull GPIO%i low to enable the Config AP."), AP_ENABLE_PIN);  
    //}
  }
  
  uint32_t size = 280*90*3*sizeof(uint8_t);
  //Serial.println(size, DEC);
  Logger.Info_f(F("Free heap: %d"), ESP.getFreeHeap()); 
  Logger.Info_f(F("Largest free block: %d"), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
  //buf1 = (uint8_t *)malloc(size);
  buf1 = (uint8_t *)heap_caps_malloc(size, MALLOC_CAP_8BIT);
  if(buf1 == nullptr) Serial.println(F("allocation buf1 did not succeed"));
  Logger.Info_f(F("Largest free block: %d"), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
 
  buf2 = (uint8_t *)heap_caps_malloc(size, MALLOC_CAP_8BIT);
  if(buf2 == nullptr) Serial.println(F("allocation buf2 did not succeed"));

  pinMode(16, INPUT_PULLUP);
  pinMode(17, INPUT_PULLUP);
  pinMode(18, INPUT_PULLUP);
  pinMode(33, OUTPUT);
  digitalWrite(33, HIGH);

  Logger.Info(F("... Startup"));
  //wheel = new Wheel();
  dw = new DISPLAY_Wheel();
  dw->set_rotation(3);
  dw->init();

  for(int i=0; i< 280*90*3; i++)
  {
    buf1[i] = 0x0;
    buf2[i] = 0x0;
  }

  Logger.Info(F("... Init done"));
  Logger.Info_f(F("Free heap: %d"), ESP.getFreeHeap()); 
}

/**
 * @brief Main loop. Use this loop to execute recurring tasks. In this sample, we will periodically send telemetry
 * and query and update the device twin.  
 * 
 */
void loop()
{
  Logger.Info_f(F("Free heap: %d"), ESP.getFreeHeap());
  Logger.Info_f(F("Largest free block: %d"), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
 
  dw->draw_background(lcars, lcars_size);
  //dw->fill_screen(0xf800);
  //wheel->process_input_change();
  vTaskDelay(1000);

  //for(int i=0; i<100; i++)
  //{
  dw->windowScroll(60, 60, 280, 180, 0, 180, buf1, buf2, 5);
 // }
  //free(buf);
  vTaskDelay(3000);
}
