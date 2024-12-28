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
    w_area_x1 = 200;
    w_area_y1 = 140;
    w_area_x2 = 480;
    w_area_y2 = 320;
    w_area_cursor_x = w_area_x1;
    w_area_cursor_y = w_area_y1;
}

/**
 * @brief Initializes the display
 */
void DISPLAY_Wheel::init()
{
    DISPLAY_SPI::init();
    draw_background(lcars, lcars_size);
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
    while(w_area_cursor_y > w_area_y2 - text_size*8)
    {
        // scroll display area to make space
        // set partial display window to work area
        set_addr_window(w_area_x1, w_area_y1, w_area_x2, w_area_y2);
        vert_scroll(140, 180, 180-text_size*8);
        w_area_cursor_y = w_area_cursor_y - text_size*8;
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

void DISPLAY_Wheel::windowScroll(int16_t x, int16_t y, int16_t w, int16_t h, int16_t dx, int16_t dy, uint8_t *bufh, uint8_t *bufl)
{
    uint32_t cnth = 0;
    uint32_t cntl = 0;
    if (dx) 
        if(true)
        {
            // even though the ILI9488 does support vertical scrolling (horizontal in landscape) via hardware
            // it only scrolls the entire screen height. So we still need to implement software scrolling...
            // first we read the data....
            cnth = read_GRAM_RGB(x, y, bufh, w, h);
            // to scroll by 1 pixel to the right, we need to process each row in the read buffer
            // and move the pixel bytes over by one and then blank our the first pixel....
            // to affect the scrolling distance dx, we need to iterate until we reach dx...
            for(uint16_t i=1; i<=dx; i++)
            {
              for(uint16_t row=0; row<h; row++)
              {
                uint8_t *rowStart = &bufh[row*w*3];                              // position the pointer at the start of the row.
                memmove(rowStart + i*3, rowStart + (i - 1) * 3, (w - i) * 3);   // move the bytes over appropriately
                rowStart[(i-1)*3] = 0x0;                                        // Red component 
                rowStart[(i-1)*3 + 1] = 0x0;                                    // Green component 
                rowStart[(i-1)*3 + 2] = 0x0;                                    // Blue component }
                                                                                // Set the first pixel to black (0x0, 0x0, 0x0)
              }
              set_addr_window(x, y, x+w-1, y+h-1);                              // Set the scroll region data window
              CS_ACTIVE;
              writeCmd8(CC);
              CD_DATA;
              spi->transferBytes(bufh, nullptr, cnth);                            // transfer the updated buffer into the window.
              CS_IDLE;
            }
        }
        else
        {
            // in portrait mode we have to use software scrolling to move in the x direction
            // first we read the data....
            cnth = read_GRAM_RGB(x, y, bufh, w, h);
            for(uint16_t i=1; i<= dx; i++)
            {
              for(uint16_t row=0; row<h; ++row) 
              { 
                uint8_t *rowStart = &bufh[row*w*3];  
                memmove(rowStart + i*3, rowStart + (i-1) * 3 , (w - i) * 3); // Shift each row to the left by one pixel (3 components)

                rowStart[(i-1)*3] = 0x0; // Red component 
                rowStart[(i-1)*3 + 1] = 0x0; // Green component 
                rowStart[(i-1)*3 + 2] = 0x0; // Blue component }
                  // Set the first pixel to black (0x0, 0x0, 0x0) 
              }
              set_addr_window(x, y, x+w-1, y+h-1);
              CS_ACTIVE;
              writeCmd8(CC);
              CD_DATA;
              spi->transferBytes(bufh, nullptr, cnth);
              CS_IDLE;
            }
        }
    if (dy) 
        if(rotation == 1 || rotation == 3)
        {
            // if we scroll vertically and rotation is landscape, we need to scroll in software
            // first we read the data....
            cnth = read_GRAM_RGB(x, y, bufh, w, h);
            for(uint16_t i=1; i<= dy; i++)
            {
              set_addr_window(x, y, x + w-1, y+h-1);
              CS_ACTIVE;
              writeCmd8(CC);
              CD_DATA;
              spi->transferBytes(bufh + i*3*w, nullptr, cnth-3*i*w);
              for(int m=3*(i-1)*w; m<3*i*w; m++) bufh[m] = 0x0;
              spi->transferBytes(bufh, nullptr, 3*i*w);
                    // each dy means we have to move the start over by 3* the with of the area
                    // conversely, the size to transfer reduces by dy*3*width
                    // but now the last row needs to be blanked....
              CS_IDLE;
            }
        }
        else
        {
          // in portrait mode we can use hardware scroll to move in hte y direction

            
        }
}

/**
 * @brief Tests the display by going through a routine of drawing various
 * shapes and information
 */
void DISPLAY_Wheel::test()
{
    draw_background(lcars, lcars_size);

    // Define the scroll window in the rotated coordinate system 
    uint16_t startX = 50; // X start in rotated view 
    uint16_t startY = 75; // Y start in rotated view 
    uint16_t width = 280; // Width of the scrolling area in rotated view 
    uint16_t height = 180;

    //uint16_t scrollbuf[480];                  //my biggest shield is 320x480


    //Logger.Info("... setting address window");

    //set_addr_window(startX, startY, startX + width - 1, startY + height - 1);

    //Logger.Info("... setup scroll");

    // Total display width (in rotated view) 
    //uint16_t topFixedArea = startY; 
    //uint16_t scrollArea = height; 
    //uint16_t bottomFixedArea = 480 - topFixedArea - scrollArea;

    //CS_ACTIVE;
    //writeCmd8(0x33);
    //writeData16(0);
    //writeData16(scrollArea);
    //writeData16(320 - scrollArea);

    //writeCmd8(0x37);
    //writeData16(0);

    //Logger.Info("... scrolling");

    //for(int scroll=0; scroll < height; scroll++){
    //    writeCmd8(0x37);
    //    writeData16(scroll);
    //    vTaskDelay(50);
    //}
    //Logger.Info("... reset display");
    //writeCmd8(0x13);
    //Logger.Info("...Done");
    //CS_IDLE;
    return;  

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