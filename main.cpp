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
#include "src/display/display_spi.h"

#define TELEMETRY_FREQUENCY_MILLISECS 120000
#define AP_ENABLE_PIN 5

/* 
 * This program is a demo of clearing screen to display black, white, red, green, blue.
 * when using the BREAKOUT BOARD only and using these hardware spi lines to the LCD,
 * the SDA pin and SCK pin is defined by the system and can't be modified.
 * if you don't need to control the LED pin,you can set it to 3.3V and set the pin definition to -1.
 * other pins can be defined by youself,for example
 * pin usage as follow:
 *                   CS  DC/RS  RESET  SDI/MOSI  SCK   LED    VCC     GND    
 * ESP32             15   25     26       13     14    3.3V   3.3V    GND
*/

#define LED   -1            
#define RS    25       
#define RESET 26


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

DISPLAY_SPI *display = NULL;

//void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c)                   
//{	
//  unsigned int i,j;
//  digitalWrite(hspi->pinSS(), LOW);
//  Lcd_Write_Com(0x02c); //write_memory_start
//  l=l+x;
//  Address_set(x,y,l,y);
//  j=l*2;
//  for(i=1;i<=j;i++)
//  {
//    Lcd_Write_Data((c>>8)&0xF8);
//    Lcd_Write_Data((c>>3)&0xFC);
//    Lcd_Write_Data(c<<3);
//  }
//  digitalWrite(hspi->pinSS(),HIGH);   
//}

//void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c)                   
//{	
//  unsigned int i,j;
//  digitalWrite(hspi->pinSS(),LOW);
//  Lcd_Write_Com(0x02c); //write_memory_start
//  l=l+y;
//  Address_set(x,y,x,l);
//  j=l*2;
//  for(i=1;i<=j;i++)
//  { 
//    Lcd_Write_Data((c>>8)&0xF8);
//    Lcd_Write_Data((c>>3)&0xFC);
//    Lcd_Write_Data(c<<3);
//  }
//  digitalWrite(hspi->pinSS(),HIGH);   
//}


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
  
  Logger.Info("... Startup\n");
  display = new DISPLAY_SPI();
  display->set_rotation(3);
  display->init();
  Logger.Info("... LCD Init done");
}

/**
 * @brief Main loop. Use this loop to execute recurring tasks. In this sample, we will periodically send telemetry
 * and query and update the device twin.  
 * 
 */
void loop()
{
  display->fill_screen(0xf800);
  display->fill_screen(0x07E0);
  display->fill_screen(0x001F);
  display->fill_screen(0x0);
  for(int i=0;i<50;i++)
  {
    display->set_draw_color(random(65535));
    display->draw_rectangle(
      random(display->get_width()), 
      random(display->get_height()),
      random(display->get_width()),
      random(display->get_height())
    );
    vTaskDelay(100);
  }
  display->fill_screen(0x0);
  for(int i=0;i<50;i++)
  {
    display->set_draw_color(random(65535));
    display->draw_round_rectangle(
      random(display->get_width()), 
      random(display->get_height()),
      random(display->get_width()),
      random(display->get_height()),
      random(10)
    );
    vTaskDelay(100);
  }
  display->fill_screen(0x0);
  for(int i=0;i<50;i++)
  {
    display->set_draw_color(random(65535));
    display->draw_triangle(
      random(display->get_width()), 
      random(display->get_height()),
      random(display->get_width()),
      random(display->get_height()), 
      random(display->get_width()),
      random(display->get_height())
    );
  }
  display->fill_screen(0x0);
  for(int i=0;i<50;i++)
  {
    display->set_draw_color(random(65535));
    display->draw_circle(
      random(display->get_width()), 
      random(display->get_height()),
      random(display->get_width())
    );
  }
  display->fill_screen(0x0);
  display->set_text_back_color(0x0);
  display->set_text_color(0xf800);
  display->set_text_size(6);
  display->print_string("The End", 20, 200);
  vTaskDelay(1000);
}
