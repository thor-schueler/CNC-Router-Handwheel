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

/*
SPIClass *hspi = NULL;

void Lcd_Writ_Bus(unsigned char d)
{
  hspi->transfer(d);
}

void Lcd_Write_Com(unsigned char VH)  
{   
  digitalWrite(RS, LOW);
  Lcd_Writ_Bus(VH);
}

void Lcd_Write_Data(unsigned char VH)
{
  digitalWrite(RS, HIGH);
  Lcd_Writ_Bus(VH);
}

void Lcd_Write_Com_Data(unsigned char com,unsigned char dat)
{
  Lcd_Write_Com(com);
  Lcd_Write_Data(dat);
}

void Address_set(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)
{
  Lcd_Write_Com(0x2a);    // Send Column Address Set Command 
	Lcd_Write_Data(x1>>8);  // This command is used to define the area of the frame memory that the MCU can access. This command makes no change on 
	Lcd_Write_Data(x1);     // the other driver status. The values of SC [15:0] (first and second data bytes) and EC [15:0] (third and fourth data bytes) 
	Lcd_Write_Data(x2>>8);  // are referred when RAMWR command is applied. Each value 
	Lcd_Write_Data(x2);     // represents one column line in the Frame Memory.

  Lcd_Write_Com(0x2b);    // Send Page Address Set Command
	Lcd_Write_Data(y1>>8);  // This command is used to define the area of the frame memory that the MCU can access. This command makes no change on 
	Lcd_Write_Data(y1);     // the other driver status. The values of SP [15:0] (first and second data bytes) and EP [15:0] (third and fourth data bytes) 
	Lcd_Write_Data(y2>>8);  // are referred when RAMWR command is applied. Each value 
	Lcd_Write_Data(y2);     // represents one Page line in the Frame Memory.  

	Lcd_Write_Com(0x2c); 	  // Send Memory Write Command. This command transfers image data from the host processor to the ILI9488’s frame memory 
                          // starting at the pixel location specified by Column Address Set (2Ah) and Page Address Set (2Bh) commands.
}

void SPI_Init(void)
{
  hspi = new SPIClass(HSPI);
  hspi->begin();
  hspi->setBitOrder(MSBFIRST);
}
*/

/**
 * Initializes the LCD panel controller
 */
/*
void Lcd_Init(void)
{
  digitalWrite(RESET, HIGH);
  vTaskDelay(5); 
  digitalWrite(RESET, LOW);
  vTaskDelay(5);
  digitalWrite(RESET, HIGH);
  vTaskDelay(5);

  digitalWrite(hspi->pinSS(), LOW); 
    //
    // pull chip select low the select the lcd display
    //
  
  Lcd_Write_Com(0xF7);    // Send Adjust Control 3 command 
  Lcd_Write_Data(0xA9);   // First parameter - constant
  Lcd_Write_Data(0x51);   // Second parameter - constant
  Lcd_Write_Data(0x2C);   // Third parameter - constant
  Lcd_Write_Data(0x82);   // Fourth parameter - DSI write DCS command, use loose packet RGB 666

  Lcd_Write_Com(0xC0);    // Send Power Control 1 command
  Lcd_Write_Data(0x11);   // First parameter - Set the VREG1OUT voltage for positive gamma 1.25 x 3.70 = 4.6250
  Lcd_Write_Data(0x09);   // Second parameter - Set the VREG2OUT voltage for negative gammas -1.25 x 3.30 = -4.1250 

  Lcd_Write_Com(0xC1);    // Send Power Control 2  command  
  Lcd_Write_Data(0x41);   // First parameter - Set the factor used in the step-up circuits.
                          //    DDVDH = VCI x 2
                          //    DDVDL = -(VCI x 2)
                          //    VCL = -VCI
                          //    VGH = VCI x 6
                          //    VGL = -VCI x 4

  Lcd_Write_Com(0xC5);    // Send VCOM Control command  
  Lcd_Write_Data(0x00);   // First parameter - NV memory is not programmed
  Lcd_Write_Data(0x0A);   // Second parameter - Used to set the factor to generate VCOM voltage from the reference voltage VREG2OUT.  VCOM = -1.75 
  Lcd_Write_Data(0x80);   // Third parameter - Select the Vcom value from VCM_REG [7:0] or NV memory. 1: VCOM value from VCM_REG [7:0].
 
  Lcd_Write_Com(0xB1);    // Send Frame Rate Control (In Normal Mode/Full Colors)
  Lcd_Write_Data(0xB0);   // First parameter - 
                          //    Set division ratio for internal clocks when Normal mode: 00 - Fosc
                          //    Set the frame frequency of full color normal mode: CNT = 17, Frame Rate 60.76 
  Lcd_Write_Data(0x11);   // Second parameter - Is used to set 1H (line) period of the Normal mode at the MCU interface: 17 clocks

  Lcd_Write_Com(0xB4);    // Send Display Inversion Control command
  Lcd_Write_Data(0x02);   // First Parameter - set the Display Inversion mode: 2 dot inversion
  
  Lcd_Write_Com(0xB6);    // Send Display Function Control command
  Lcd_Write_Data(0x02);   // First parameter -
                          //    0000 0010
                          //    0           Select the display data path (memory or direct to shift register) when the RGB interface is used. Bypass - Memory
                          //     0          RCM RGB interface selection (refer to the RGB interface section). DE Mode
                          //      0         Select the interface to access the GRAM. When RM = 0, the driver will write display data to the GRAM via the system 
                          //                interface, and the driver will write display data to the GRAM via the RGB interface when RM = 1.
                          //       0        Select the display operation mode: Internal system clock  
                          //         00     Set the scan mode in a non-display area: Normal scan
                          //           10   Determine source/VCOM output in a non-display area in the partial display mode: AGND   
  Lcd_Write_Data(0x22);   // Second parameter - 
                          //     0010 0010  
                          //     00         Set the direction of scan by the gate driver: G1 -> G480
                          //       1        Select the shift direction of outputs from the source driver: S960 -> S1        
                          //        0       Set the gate driver pin arrangement in combination with the GS bit (RB6h) to select the optimal scan mode for the module: G1->G2->G3->G4 ………………… G477->G478->G479->G480 
                          //          0010  Set the scan cycle when the PTG selects interval scan in a non-display area drive period. The scan cycle is defined 
                          //                by n frame periods, where n is an odd number from 3 to 31. The polarity of liquid crystal drive voltage from the gate driver is 
                          //                inverted in the same timing as the interval scan cycle: 5 frames (84ms)
  Lcd_Write_Com(0xB7);    // Send Entry Mode Set command   
  Lcd_Write_Data(0xC6);   // First parameter - 
                          //    1100 0110
                          //    1100        Set the data format when 16bbp (R, G, B) to 18 bbp (R, G, B) is stored in the internal GRAM. See ILI9488 datasheet 
                          //         0      The ILI9488 driver enters the Deep Standby Mode when the DSTB is set to high (= 1). In the Deep Standby mode, 
                          //                both internal logic power and SRAM power are turned off, the display data are stored in the Frame Memory, and the 
                          //                instructions are not saved. Rewrite Frame Memory content and instructions after exiting the Deep Standby Mode.
                          //          11    Set the output level of the gate driver G1 ~ G480 as follows: Normal display  
                          //            0   Low voltage detection control: Enable
  Lcd_Write_Com(0xBE);    // Send HS Lanes Control command
  Lcd_Write_Data(0x00);   // First parameter - Type 1 
  Lcd_Write_Data(0x04);   // Second parameter - ESD protection: on
 
  Lcd_Write_Com(0xE9);    // Send Set Image Function command
  Lcd_Write_Data(0x00);   // First parameter -  Enable 24-bits Data Bus; users can use DB23~DB0 as 24-bits data input: off
 
  Lcd_Write_Com(0x36);    // Send Memory Access Control command
  Lcd_Write_Data(0x08);   // First parameter - see datasheet 
                          //    0000 1000   
                          //    0         MY - Row Address Order
                          //     0        MX - Column Access Order
                          //      0       MV - Row/Column Exchange
                          //       0      ML - Vertical Refresh Order
                          //         1    BGR- RBG-BGR Order
                          //          0   MH - Horizontal Refresh Order
  Lcd_Write_Com(0x3A);    // Send Interface Pixel Format command
  Lcd_Write_Data(0x66);   // First parameter - 
                          //    0110 0110
                          //    0110      RGB Interface Format 18bits/pixel
                          //         0110 MCU Interface Format 18bits/pixel
  Lcd_Write_Com(0xE0);    // Send PGAMCTRL(Positive Gamma Control) command
  Lcd_Write_Data(0x00);   // Parameters 1 - 15 - Set the gray scale voltage to adjust the gamma characteristics of the TFT panel.
  Lcd_Write_Data(0x07); 
  Lcd_Write_Data(0x10); 
  Lcd_Write_Data(0x09); 
  Lcd_Write_Data(0x17); 
  Lcd_Write_Data(0x0B); 
  Lcd_Write_Data(0x41); 
  Lcd_Write_Data(0x89); 
  Lcd_Write_Data(0x4B); 
  Lcd_Write_Data(0x0A); 
  Lcd_Write_Data(0x0C); 
  Lcd_Write_Data(0x0E); 
  Lcd_Write_Data(0x18); 
  Lcd_Write_Data(0x1B); 
  Lcd_Write_Data(0x0F); 

  Lcd_Write_Com(0xE1);    // Send NGAMCTRL(Negative Gamma Control) command 
  Lcd_Write_Data(0x00);   // Parameters 1 - 15 - Set the gray scale voltage to adjust the gamma characteristics of the TFT panel.
  Lcd_Write_Data(0x17); 
  Lcd_Write_Data(0x1A); 
  Lcd_Write_Data(0x04); 
  Lcd_Write_Data(0x0E); 
  Lcd_Write_Data(0x06); 
  Lcd_Write_Data(0x2F); 
  Lcd_Write_Data(0x45); 
  Lcd_Write_Data(0x43); 
  Lcd_Write_Data(0x02); 
  Lcd_Write_Data(0x0A); 
  Lcd_Write_Data(0x09); 
  Lcd_Write_Data(0x32); 
  Lcd_Write_Data(0x36); 
  Lcd_Write_Data(0x0F); 

  Lcd_Write_Com(0x11);    //Send Sleep OUT command 			
  Lcd_Write_Com(0x29);    // Send Display On command 

  digitalWrite(hspi->pinSS(), HIGH);
}

void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c)                   
{	
  unsigned int i,j;
  digitalWrite(hspi->pinSS(), LOW);
  Lcd_Write_Com(0x02c); //write_memory_start
  //digitalWrite(RS,HIGH);
  l=l+x;
  Address_set(x,y,l,y);
  j=l*2;
  for(i=1;i<=j;i++)
  {
    Lcd_Write_Data((c>>8)&0xF8);
    Lcd_Write_Data((c>>3)&0xFC);
    Lcd_Write_Data(c<<3);
  }
  digitalWrite(hspi->pinSS(),HIGH);   
}

void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c)                   
{	
  unsigned int i,j;
  digitalWrite(hspi->pinSS(),LOW);
  Lcd_Write_Com(0x02c); //write_memory_start
  //digitalWrite(RS,HIGH);
  l=l+y;
  Address_set(x,y,x,l);
  j=l*2;
  for(i=1;i<=j;i++)
  { 
    Lcd_Write_Data((c>>8)&0xF8);
    Lcd_Write_Data((c>>3)&0xFC);
    Lcd_Write_Data(c<<3);
  }
  digitalWrite(hspi->pinSS(),HIGH);   
}

void Rect(unsigned int x,unsigned int y,unsigned int w,unsigned int h,unsigned int c)
{
  H_line(x  , y  , w, c);
  H_line(x  , y+h, w, c);
  V_line(x  , y  , h, c);
  V_line(x+w, y  , h, c);
}

void Rectf(unsigned int x,unsigned int y,unsigned int w,unsigned int h,unsigned int c)
{
  unsigned int i;
  for(i=0;i<h;i++)
  {
    H_line(x  , y  , w, c);
    H_line(x  , y+i, w, c);
  }
}

int RGB(int r,int g,int b)
{
  return r << 16 | g << 8 | b;
}

void LCD_Clear(unsigned int j)                   
{	
  unsigned int i,m;
  digitalWrite(hspi->pinSS(),LOW);
  Address_set(0,0,320,480);
  for(i=0;i<320;i++)
    for(m=0;m<480;m++)
    {
      Lcd_Write_Data((j>>8)&0xF8);
      Lcd_Write_Data((j>>3)&0xFC);
      Lcd_Write_Data(j<<3);
    }
  digitalWrite(hspi->pinSS(),HIGH);   
}
*/



/**
 * @brief Performs system setup activities, including connecting to WIFI, setting time, obtaining the IoTHub info 
 * from DPS if necessary and connecting the IoTHub. Use this method to also register various delegates and command
 * handlers.
 * 
 */
void setup()
{
  int wifi_try_counter = 0;

  //Logger.Info_f(F("Copyright 2022, Avanade, Firmware Version: %s"), FIRMWARE_VERSION);
  //Logger.Info_f(F("Loop task stack size: %i"), getArduinoLoopTaskStackSize());
  //Logger.Info_f(F("Loop task stack high water mark: %i"), uxTaskGetStackHighWaterMark(NULL));

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
  
  Logger.Info("... startup\n");
  
  // initialize SPI
  //SPI_Init();
  //Logger.Info("... SPI Init Done");

  // intialize auxiliary pins
  //pinMode(hspi->pinSS(),OUTPUT);
  //pinMode(RS,OUTPUT);
  //pinMode(RESET,OUTPUT);
  //digitalWrite(hspi->pinSS(), HIGH);
  //digitalWrite(RS, HIGH);
  //digitalWrite(RESET, HIGH);
  //Logger.Info("... Pins configured");
  
  // intialize TFT LCD panel
  //Lcd_Init();
  display = new DISPLAY_SPI();
  //display->set_rotation(3);
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
  //LCD_Clear(0xf800);
  //LCD_Clear(0x07E0);
  //LCD_Clear(0x001F);
  //LCD_Clear(0x0); 
  //for(int i=0;i<500;i++)
  //{
  //  Rect(random(300),random(300),random(300),random(300),random(65535)); // rectangle at x, y, with, hight, color
  //}

  display->fill_screen(0xf800);
  display->fill_screen(0x07E0);
  display->fill_screen(0x001F);
  display->fill_screen(0x0);
  for(int i=0;i<500;i++)
  {
    display->set_draw_color(random(65535));
    display->draw_rectangle(
      random(display->get_display_width()), 
      random(display->get_display_height()),
      random(display->get_display_width()),
      random(display->get_display_height())
    );
  }
  vTaskDelay(100);
}
