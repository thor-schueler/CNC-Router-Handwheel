// Copyright (c) Thor Schueler. All rights reserved.
// SPDX-License-Identifier: MIT
// IMPORTANT: LIBRARY MUST BE SPECIFICALLY CONFIGURED FOR EITHER TFT SHIELD
// OR BREAKOUT BOARD USAGE.

#include <SPI.h>
#include "display_wheel.h"
#include "../logging/SerialLogger.h"


/**
 * @brief Generates a new instance of the DISPLAY_SPI class. 
 * @details initializes the SPI and LCD pins including CS, RS, RESET 
 */
DISPLAY_Wheel::DISPLAY_Wheel()
{
    draw_background(lcars, lcars_size);
    w_area_x1 = 200;
    w_area_y1 = 140;
    w_area_x2 = 480;
    w_area_y2 = 320;
}

/**
 * @brief Tests the display by going through a routine of drawing various
 * shapes and information
 */
void DISPLAY_Wheel::test()
{
  int w = w_area_x2 - w_area_x1;
  int h = w_area_y2 - w_area_y1;

  draw_background(lcars, lcars_size);
  fill_rect(w_area_x1, w_area_y1, w, h, 0xf800); vTaskDelay(500);
  fill_rect(w_area_x1, w_area_y1, w, h, 0x07E0); vTaskDelay(500);
  fill_rect(w_area_x1, w_area_y1, w, h, 0x001F); vTaskDelay(500);
  fill_rect(w_area_x1, w_area_y1, w, h, 0x0);
  for(int i=0;i<50;i++)
  {
    set_draw_color(random(65535));
    draw_rectangle(
      w_area_x1 + random(w), 
      w_area_y1 + random(h),
      w_area_x1 + random(w),
      w_area_y1 + random(h)
    );
    vTaskDelay(100);
  }
  fill_rect(w_area_x1, w_area_y1, w, h, 0x0);
  for(int i=0;i<50;i++)
  {
    int x1 = w_area_x1 + random(w);
    int y1 = w_area_y1 + random(h);
    int x2 = w_area_x1 + random(w);
    int y2 = w_area_y1 + random(h);
    int r = (x2>x1?x2-x1:x1-x2)>(y2>y1?y2-y1:y1-y2) ? random((y2>y1?y2-y1:y1-y2)/4) : random((x2>x1?x2-x1:x1-x2)/4);
    set_draw_color(random(65535));
    draw_round_rectangle(x1, y1, x2, y2, r); 
    vTaskDelay(100);
  }
  fill_rect(w_area_x1, w_area_y1, w, h, 0x0);
  for(int i=0;i<50;i++)
  {
    set_draw_color(random(65535));
    draw_triangle(
      w_area_x1 + random(w), 
      w_area_y1 + random(h),
      w_area_x1 + random(w), 
      w_area_y1 + random(h), 
      w_area_x1 + random(w), 
      w_area_y1 + random(h)
    );
  }
  fill_rect(w_area_x1, w_area_y1, w, h, 0x0);
  for(int i=0;i<50;i++)
  {
    uint16_t r = w>h ? random(h/2) : random(w/2);
    set_draw_color(random(65535));
    draw_circle(
      w_area_x1 + r + random(w), 
      w_area_y1 + r + random(h),
      r
    );
  }
  fill_rect(w_area_x1, w_area_y1, w, h, 0x0);
  set_text_back_color(0x0);
  set_text_color(0xf800);
  set_text_size(6);
  print_string("The End", w_area_x1+20, w_area_y1+20);

  write_axis(X); vTaskDelay(1000);
  write_axis(Y); vTaskDelay(1000);
  write_axis(Z); vTaskDelay(1000);

  write_feed(Feed::NANO); vTaskDelay(1000);
  write_feed(Feed::MICRO); vTaskDelay(1000);
  write_feed(Feed::MILLI); vTaskDelay(1000);
  write_feed(Feed::FULL); vTaskDelay(1000);

  float val = 310.703;
  for(int i=0; i<1000; i++)
  {
    write_x(val);
    val += 0.001;
    vTaskDelay(5);
  }
  val = 720.089;
  for(int i=0; i<100; i++)
  {
    write_y(val);
    val += 0.1;
    vTaskDelay(10);
  }
  write_z(30.009);
  vTaskDelay(5000);
}

/**
 * @brief Writes the currently selected Axis into the display
 * @param axis - The value for the axis to print
 */
void DISPLAY_Wheel::write_axis(Axis axis)
{
    //fill_rect(80, 150, 105, 40, RGB_to_565(44,0,63));
    set_text_back_color(RGB_to_565(44,0,63));
    set_text_color(0xffff);
    set_text_size(4);
    switch(axis)
    {
        case X: print_string("X", 155, 158); break;
        case Y: print_string("Y", 155, 158); break;
        case Z: print_string("Z", 155, 158); break;
    }

}

/**
 * @brief Writes the current feed selection to the display
 * @param feed - The value for the feed selection to write
 */
void DISPLAY_Wheel::write_feed(float feed)
{
    set_text_back_color(RGB_to_565(44,0,63));
    set_text_color(0xffffff);
    set_text_size(3);
    print_number_float(feed, 3, 90, 210, '.', 5, ' ');
}

/**
 * @brief Writes the current x position to the display
 * @param x - The value for the x position to write
 */
void DISPLAY_Wheel::write_x(float x)
{
    set_text_back_color(RGB_to_565(44,0,63));
    set_text_color(0xffffff);
    set_text_size(2);
    print_number_float(x, 3, 128, 65, '.', 7, ' ');
}

/**
 * @brief Writes the current y position to the display
 * @param y - The value for the y position to write
 */
void DISPLAY_Wheel::write_y(float y)
{
    set_text_back_color(RGB_to_565(44,0,63));
    set_text_color(0xffffff);
    set_text_size(2);
    print_number_float(y, 3, 238, 65, '.', 7, ' ');
}

/**
 * @brief Writes the current z position to the display
 * @param z - The value for the z position to write
 */
void DISPLAY_Wheel::write_z(float z)
{
    set_text_back_color(RGB_to_565(44,0,63));
    set_text_color(0xffffff);
    set_text_size(2);
    print_number_float(z, 3, 348, 65, '.', 7, ' ');
}