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
#include <WiFi.h>
#include <SPI.h>

// C99 libraries
#include <cstdlib>
#include <string.h>
#include <time.h>

// local libraries
#include "src/config/config_page.h"
#include "src/logging/SerialLogger.h"
#include "src/display/display_wheel.h"
#include "src/wheel/wheel.h"

#define TELEMETRY_FREQUENCY_MILLISECS 120000
#define AP_ENABLE_PIN 0

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


static Config config;
Wheel *wheel = NULL;
TaskHandle_t wifi_task = NULL;
static bool has_wifi = true;
static bool config_mode = false;

/**
 * @brief Task runner for a one time task connecting to Wifi.
 * @param args - Task arguments 
 */
void connect_WiFi(void* args)
{
  int wifi_try_counter = 0;
  bool connected = false;

  if(wheel != NULL) 
    wheel->write_status_message(F("Attempting to connect to Wifi %s"), config.ssid);
  while (true)
  {
    connected = config.Connect_Wifi();
    if(connected) break;
    if(wifi_try_counter++ > 5) break;
  }
  if(connected) 
  {
    if(wheel != NULL) wheel->write_status_message(F("Wifi connected to %s: %s"), config.ssid, WiFi.localIP().toString());
    config.InitializeTime();
  }
  else
  {
    if(wheel != NULL) wheel->write_status_message(F("Wifi failed to connect to %s"), config.ssid);
  }
  vTaskDelete(wifi_task);
  wifi_task=NULL;
}

#ifdef SET_LOOP_TASK_STACK_SIZE
SET_LOOP_TASK_STACK_SIZE(16384);
  //
  // This will only work with ESP-Arduino 2.0.7 or higher. 
  //
#endif

/**
 * @brief Performs system setup activities, including connecting to WIFI, setting time, obtaining the IoTHub info 
 * from DPS if necessary and connecting the IoTHub. Use this method to also register various delegates and command
 * handlers.
 * 
 */
void setup()
{
  Logger.Info_f(F("Copyright 2024, Thor Schueler, Firmware Version: %s"), "0.00.00");
  Logger.Info_f(F("Loop task stack size: %i"), getArduinoLoopTaskStackSize());
  Logger.Info_f(F("Loop task stack high water mark: %i"), uxTaskGetStackHighWaterMark(NULL));
  Logger.Info_f(F("Total heap: %d"), ESP.getHeapSize()); 
  Logger.Info_f(F("Free heap: %d"), ESP.getFreeHeap()); 
  Logger.Info_f(F("Total PSRAM: %d"), ESP.getPsramSize()); 
  Logger.Info_f(F("Free PSRAM: %d"), ESP.getFreePsram());
  Logger.Info(F("... Startup"));

  // 
  // Configuration Mode
  //
  if(AP_ENABLE_PIN == 0)
  {
    config.StartAP();
  }
  else
  {
    Logger.Info_f(F("Checking Configuration AP Enable on pin GPIO%i."), AP_ENABLE_PIN);
    pinMode(AP_ENABLE_PIN, INPUT_PULLUP); 
    if(digitalRead(AP_ENABLE_PIN) == HIGH)
    {
      Logger.Info_f(F("Config AP enabled. Pull GPIO%i low to disable the Config AP."), AP_ENABLE_PIN);
      config_mode = true;
      config.StartAP();
    }
    else
    {
      config_mode = false;
      Logger.Info_f(F("Config AP disabled. Do not pull GPIO%i low to enable the Config AP."), AP_ENABLE_PIN);  
    }
  }
  wheel = new Wheel();
  xTaskCreatePinnedToCore(connect_WiFi, "wificonnector", 2048, NULL, 1, &wifi_task, 0);
  if(config_mode)
  {
    Logger.Info_f(F("Device is in config mode. Do not pull GPIO %i low to enter normal operations"), AP_ENABLE_PIN);
    return;
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
  vTaskDelay(1000);
}
