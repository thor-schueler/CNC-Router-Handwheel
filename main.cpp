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
#include <esp_heap_caps.h>
#include <BluetoothSerial.h>

// C99 libraries
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <new>

// local libraries
#include "src/config/config_page.h"
#include "src/logging/SerialLogger.h"
#include "src/display/display_wheel.h"
#include "src/connection_monitor/connection_monitor.h"
#include "src/wheel/wheel.h"

#define TELEMETRY_FREQUENCY_MILLISECS 120000
#define AP_ENABLE_PIN 0

/**
 * Note that there is a bug in the Hardware Serial Implementation of the ESP32 core starting with v3.0.0 up that incorrectly
 * initializes the UART with the wrong clock, resulting in barbled output for bad rates above 115200 and below 250000 baud.
 * Since we need buad rate of 230400 for the wheel communication, we have two options: either implement serial communication using 
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
BluetoothSerial bt;
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

  while(true)
  {  
    if(WiFi.status() != WL_CONNECTED)
    {
      if(wheel != NULL && !connected) wheel->write_status_message(F("Attempting to connect to Wifi %s"), config.ssid);
      if(wheel !=NULL && WiFi.status() == WL_CONNECTION_LOST) wheel->write_status_message(F("Connection to Wifi %s lost, attempting to reconnect"), config.ssid);
      if(wheel !=NULL && WiFi.status() == WL_CONNECTION_LOST) wheel->write_status_message(F("Connection to Wifi %s failed, retrying..."), config.ssid);      
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
      Logger.Info_f(F("Free heap: %d"), ESP.getFreeHeap()); 
      vTaskDelay(pdMS_TO_TICKS(5000));
    }
    else vTaskDelay(pdMS_TO_TICKS(5000));
  }

  vTaskDelete(wifi_task);
  wifi_task=NULL;
}

#ifdef SET_LOOP_TASK_STACK_SIZE
SET_LOOP_TASK_STACK_SIZE(3052);
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
  // Enable PSRAM if available. This will allow us to use the PSRAM for dynamic memory allocation, 
  // which is crucial for the display and other memory intensive operations.
  esp_log_level_set("*", ESP_LOG_NONE);
  if (psramFound()) {
    heap_caps_malloc_extmem_enable(256);   // allow malloc/new to use PSRAM, threshold in bytes
  }

  //
  // Initialize configuration data from EEPROM
  //
  config.Initialize(false);
  Command_t::merge(&config.Commands, &Wheel::Commands);
  Logger.SetSpeed(config.baud_rate);

  //
  // Startup
  //
  Logger.Info_f(F("Copyright 2024, Thor Schueler, Firmware Version: %s"), "0.00.00");
  Logger.Info_f(F("Loop task stack size: %i"), getArduinoLoopTaskStackSize());
  Logger.Info_f(F("Loop task stack high water mark: %i"), uxTaskGetStackHighWaterMark(NULL));
  Logger.Info_f(F("Total heap: %d"), ESP.getHeapSize()); 
  Logger.Info_f(F("Free heap: %d"), ESP.getFreeHeap()); 
  Logger.Info_f(F("Total PSRAM: %d"), ESP.getPsramSize()); 
  Logger.Info_f(F("Free PSRAM: %d"), ESP.getFreePsram());
  Logger.Info(F("... Startup"));
  config.Print();

  // 
  // Configuration Mode
  //
  if(AP_ENABLE_PIN == 0)
  {
    //config.StartAP();
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

  xTaskCreatePinnedToCore(connect_WiFi, "wificonnector", 2560, NULL, 1, &wifi_task, 1);
  if(config_mode)
  {
    Logger.Info_f(F("Device is in config mode. Do not pull GPIO %i low to enter normal operations"), AP_ENABLE_PIN);
    return;
  }  

  vTaskDelay(pdMS_TO_TICKS(5000));
    // this delay is critical to give the BT stack enough time to initialize before we start initializing WiFi.
    // these two cannot initialize in parallel, at least not without significant refactoring of the code and 
    // careful management of the BT stack initialization. 
    // Without this, the BT stack will crash with a Guru Meditation error
  Logger.Info_f(F("Free heap: %d"), ESP.getFreeHeap()); 
  Logger.Info(F("Starting Bluetooth"));


  bt.disableSSP();
  bt.begin("CNC Pendant", false, true);
  Logger.Info_f(F("Free heap: %d"), ESP.getFreeHeap()); 

  ConnectionMonitor::get_instance()->set_display_instance(wheel->get_display());
  ConnectionMonitor::get_instance()->set_bluetooth_instance(&bt);
  wheel->set_bluetooth_instance(&bt);

  Logger.Info(F("... Init done"));
  Logger.Info_f(F("Free heap: %d"), ESP.getFreeHeap()); 
  Logger.Info_f(F("Free PSRAM: %d"), ESP.getFreePsram());
}

/**
 * @brief Main loop. Use this loop to execute recurring tasks. In this sample, we will periodically send telemetry
 * and query and update the device twin.  
 * 
 */
void loop()
{
  vTaskDelay(10000);
  Logger.Info_f(F("Loop task stack high water mark: %i"), uxTaskGetStackHighWaterMark(NULL));
  Logger.Info_f(F("Wifi task stack high water mark: %i"), uxTaskGetStackHighWaterMark(wifi_task));

  //Logger.Info_f(F("GPIO task stack high water mark: %i"), uxTaskGetStackHighWaterMark(wheel->_extendedGPIOWatcher));
  //Logger.Info_f(F("DISPLAY task stack high water mark: %i"), uxTaskGetStackHighWaterMark(wheel->_displayRunner));
  //Logger.Info_f(F("Wheel task stack high water mark: %i"), uxTaskGetStackHighWaterMark(wheel->_wheelRunner));
  //Logger.Info_f(F("EMS task stack high water mark: %i"), uxTaskGetStackHighWaterMark(wheel->_emsChangeRunner));
  //Logger.Info_f(F("Monitor task stack high water mark: %i"), uxTaskGetStackHighWaterMark(ConnectionMonitor::get_instance()->monitor));
  Logger.Info_f(F("Free heap: %d"), ESP.getFreeHeap());
}
