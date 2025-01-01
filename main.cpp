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

/**
 * Note that there is a bug in the Hardware Serial Implementation of the ESP32 core starting with v3.0.0 up that incorrectly
 * initializes the UART with the wrong clock, resulting in barbled output for bad rates above 115200 and below 250000 baud.
 * Since we need buad rate of 230400 for the wheel communication, we have two options: either implement serial communication uing 
 * UART DMA directly (by Adjusting SerialLogger.cpp) using the code below or updating esp32-hal-uart.c method uartBegin, line 511 or 
 * thereabouts. There is a if statement checking the baudrate. If the baudrate is <= 250000 the the clocksource is
 * set to UART_SCLK_REF_TICK, otherwise the clocksource is UART_SCLK_APB. That is fine for baudrates up to 115200. However, for higher rates
 * including 230400 should be UART_SCLK_APB. Therefore changing the if statement from <= 250000 to <= 230400 will fix
 * the issue. 
 * 
 * Code for direct UART DMA:
 *    if (!uart_is_driver_installed(UART_NUM_0))
      { 
        uart_config_t uart_config = { 
          .baud_rate = 230400, 
          .data_bits = UART_DATA_8_BITS, 
          .parity = UART_PARITY_DISABLE, 
          .stop_bits = UART_STOP_BITS_1, 
          .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, 
          //.source_clk = UART_SCLK_REF_TICK, 
          .source_clk = UART_SCLK_APB, 
      }; 
      // Enable UART0 with DMA 
      uart_driver_install(UART_NUM_0, BUF_SIZE, BUF_SIZE, 10, NULL, ESP_INTR_FLAG_IRAM); 
      uart_param_config(UART_NUM_0, &uart_config); 
      uart_set_pin(UART_NUM_0, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE); 
      uart_set_mode(UART_NUM_0, UART_MODE_UART); 

      // write to UART
      const char *data = "Hello, UART0 with DMA!\n"; 
      uart_write_bytes(UART_NUM_0, data, strlen(data)); 
*/


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
//DISPLAY_Wheel *dw = NULL;

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
  

  Logger.Info(F("... Startup"));
  wheel = new Wheel();
  //wheel->display->test();
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
  //Logger.Info_f(F("Free heap: %d"), ESP.getFreeHeap());
  //Logger.Info_f(F("Largest free block: %d"), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
 
  //dw->draw_background(lcars, lcars_size);
  //dw->fill_screen(0xf800);
  //wheel->update_screen();
  //wheel->process_input_change();
  //vTaskDelay(1000);

  //for(int i=0; i<100; i++)
  //{
  //dw->windowScroll(60, 60, 180, 280, 90, 140, buf1, buf2, 5);
 // }
  //free(buf);
  //Logger.Info_f(F("%d-%d-%d-%d"),digitalRead(35),digitalRead(17),digitalRead(18), digitalRead(19));
  Logger.Info_f("Wheel Position: %i", wheel->_wheel_position);
  vTaskDelay(1000);
}
