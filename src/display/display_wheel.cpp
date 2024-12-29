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
    w_area_x1 = 210;
    w_area_y1 = 140;
    w_area_x2 = 480;
    w_area_y2 = 316;
    w_area_cursor_x = w_area_x1;
    w_area_cursor_y = w_area_y1;
}

/**
 * @brief Initializes the display
 */
void DISPLAY_Wheel::init()
{
    uint32_t buffer_size = (w_area_x2-w_area_x1)*(w_area_y2-w_area_y1)*3/2;
    DISPLAY_SPI::init();
    draw_background(lcars, lcars_size);
    
    Logger.Info(F("Attempting allocation of screen scrolling memory buffer..."));
    Logger.Info_f(F("....Largest free block: %d"), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    buf1 = (uint8_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_8BIT);
    if(buf1 == nullptr) Logger.Error(F("....Allocation of upper scroll buffer (buf1) did not succeed"));
    else Logger.Info(F("....Allocation of upper scroll buffer (buf1) successful"));

    buf2 = (uint8_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_8BIT);
    if(buf1 == nullptr) Logger.Error(F("....Allocation of lower scroll buffer (buf1) did not succeed"));
    else Logger.Info(F("....Allocation of lower scroll buffer (buf1) successful"));
    Logger.Info_f(F("....Free heap: %d"), ESP.getFreeHeap());
    Logger.Info_f(F("....Largest free block: %d"), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    Logger.Info(F("Done."));
}

/**
 * @brief Print a string in the working area. Advances the cursor to keep track of position
 * @param s - String to print
 */
void DISPLAY_Wheel::w_area_print(String s, bool newline)
{
    set_text_color(0xffff);
    set_text_back_color(0x0);
    set_text_size(1);
    Logger.Info_f(F("printing %d"), w_area_cursor_y);
    while(w_area_cursor_y > w_area_y2 - text_size*16)
    {
        Logger.Info(F("scroll"));
        // scroll display area to make space
        window_scroll(w_area_x1, w_area_y1, w_area_x2-w_area_x1, w_area_y2-w_area_y1, 
            0, text_size*16, buf1, buf2, text_size*16);
        w_area_cursor_y = w_area_cursor_y - text_size*16;
        Logger.Info_f(F("printing %d"), w_area_cursor_y);
    }

    print_string(s, w_area_cursor_x, w_area_cursor_y);
    w_area_cursor_x = get_text_X_cursor();
    w_area_cursor_y = get_text_Y_cursor();
    if(newline)
    {
        print_string("\n", w_area_cursor_x, w_area_cursor_y);
        w_area_cursor_x = w_area_x1;
        w_area_cursor_y = get_text_Y_cursor();
    }

}


/**
 * @brief Implements scrolling for partial screen, both horizontally and vertically
 * @param x - the top left of the scrolling area x coordinate
 * @param y - the top left of the scrolling area y coordinate
 * @param w - the width of the scroll area
 * @param h - the height of the scorll area
 * @param dx - the ammount to scroll into the x direction
 * @param dy - the ammount to scroll tino the y direction
 * @param bufh - the upper page buffer. Expected to be w*h*3/2
 * @param bufl - the lower page buffer. Expected to be w*h*3/2
 * @param inc - the scroll imcrement. Defaults to 1. 
 * @remarks Since memory on the ESP32 is limited, the page buffer is split into an upper and lower area
 * to accomodate an scrolling window of about 280*180 (or a bit larger). The buffers needs to be allocated to 
 * be each w*h/2*3 (since each pixel is represented by 3 bytes). Additionally, it is important that the dy 
 * parameter is divisible by two and that dy/2 is divisible by inc. 
 */
void DISPLAY_Wheel::window_scroll(int16_t x, int16_t y, int16_t w, int16_t h, int16_t dx, int16_t dy, uint8_t *bufh, uint8_t *bufl, uint8_t inc)
{
    uint32_t cnth = 0;
    uint32_t cntl = 0;
    uint16_t bh = h/2;
    if (dx) 
    {
      // even though the ILI9488 does support vertical scrolling (horizontal in landscape) via hardware
      // it only scrolls the entire screen height. So we still need to implement software scrolling...
      // first we read the data....
      cnth = read_GRAM_RGB(x, y, bufh, w, bh);
      cntl = read_GRAM_RGB(x, y + bh, bufl, w, bh);
      
      // to scroll by 1 pixel to the right, we need to process each row in the read buffer
      // and move the pixel bytes over by one and then blank our the first pixel....
      // to affect the scrolling distance dx, we need to iterate until we reach dx...
      for(uint16_t i=1; i<=dx; i+=inc)
      {
        for(uint16_t row=0; row<bh; row++)
        {
          uint8_t *rowStarth = &bufh[row*w*3];
          uint8_t *rowStartl = &bufl[row*w*3];                            // position the pointer at the start of the row.
          memmove(rowStarth + (i+inc-1)*3, rowStarth + (i-1)*3, (w - (i+inc-1)) * 3); 
          memmove(rowStartl + (i+inc-1)*3, rowStartl + (i-1)*3, (w - (i+inc-1)) * 3); 
                                                                          // move the bytes over appropriately
          for(uint8_t k=1; k<=inc; k++)
          {                                                                 
            rowStarth[((i-1)+(k-1))*3]     = 0x0;               // Red component 
            rowStarth[((i-1)+(k-1))*3 + 1] = 0x0;               // Green component 
            rowStarth[((i-1)+(k-1))*3 + 2] = 0x0;               // Blue component 
            rowStartl[((i-1)+(k-1))*3]     = 0x0;               // Red component 
            rowStartl[((i-1)+(k-1))*3 + 1] = 0x0;               // Green component 
            rowStartl[((i-1)+(k-1))*3 + 2] = 0x0;               // Blue component 
                                                                // Set the first pixel to black (0x0, 0x0, 0x0)
          }
        }
        set_addr_window(x, y, x+w-1, y+h-1);                              // Set the scroll region data window
        CS_ACTIVE;
        writeCmd8(CC);
        CD_DATA;
        spi->transferBytes(bufh, nullptr, cnth);                            // transfer the updated buffer into the window.
        spi->transferBytes(bufl, nullptr, cntl);
        CS_IDLE;
      }
    }
    if (dy) 
    {
      // if we scroll vertically and rotation is landscape, we need to scroll in software
      // first we read the data....
      cnth = read_GRAM_RGB(x, y, bufh, w, bh);
      cntl = read_GRAM_RGB(x, y + bh, bufl, w, bh);
      set_addr_window(x, y, x + w-1, y+h-1);
      CS_ACTIVE;
      writeCmd8(CC);
      CD_DATA;

      for(uint16_t i=inc; i<=dy; i+=inc)
      {
        if(i<=bh)
        {
          spi->transferBytes(bufh + i*3*w, nullptr, cnth-3*i*w);
          spi->transferBytes(bufl, nullptr, cntl);
          memset(bufh + 3*(i-inc)*w, 0x0, inc*w*3);
          spi->transferBytes(bufh, nullptr, 3*i*w);
                // each dy means we have to move the start over by 3* the with of the area
                // conversely, the size to transfer reduces by dy*3*width
                // but now the last row needs to be blanked....
        }
        else
        {
          // now bufh has been fully processes and we need to shif processing to bufl
          spi->transferBytes(bufl + (i-bh)*3*w, nullptr, cntl-3*(i-bh)*w);
          spi->transferBytes(bufh, nullptr, cnth);
          memset(bufl + 3*(i-bh-inc)*w, 0x0, inc*w*3);
          spi->transferBytes(bufl, nullptr, 3*(i-bh)*w);
        }
      }
      CS_IDLE;
    }
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
  fill_rect(w_area_x1, w_area_y1, w, h+4, 0x0);
  set_text_back_color(0x0);
  set_text_color(0xf800);
  set_text_size(5);
  print_string("The End", w_area_x1+20, w_area_y1+20);

  write_axis(X); vTaskDelay(1000);
  write_axis(Y); vTaskDelay(1000);
  write_axis(Z); vTaskDelay(1000);

  write_feed(Feed::NANO); vTaskDelay(1000);
  write_feed(Feed::MICRO); vTaskDelay(1000);
  write_feed(Feed::MILLI); vTaskDelay(1000);
  write_feed(Feed::FULL); vTaskDelay(1000);

  float val = 310.703;
  for(int i=0; i<500; i++)
  {
    write_x(val);
    val += 0.001;
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
  fill_rect(w_area_x1, w_area_y1, w, h, 0x0);
}

/**
 * @brief Writes the currently selected Axis into the display
 * @param axis - The value for the axis to print
 */
void DISPLAY_Wheel::write_axis(Axis axis)
{
    set_text_back_color(RGB_to_565(177,0,254));
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
    set_text_back_color(RGB_to_565(177,0,254));
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
    set_text_back_color(RGB_to_565(177,0,254));
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
    set_text_back_color(RGB_to_565(177,0,254));
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
    set_text_back_color(RGB_to_565(177,0,254));
    set_text_color(0xffffff);
    set_text_size(2);
    print_number_float(z, 3, 348, 65, '.', 7, ' ');
}