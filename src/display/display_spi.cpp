
// Copyright (c) Thor Schueler. All rights reserved.
// SPDX-License-Identifier: MIT
// IMPORTANT: LIBRARY MUST BE SPECIFICALLY CONFIGURED FOR EITHER TFT SHIELD
// OR BREAKOUT BOARD USAGE.

#include <SPI.h>
#include "pins_arduino.h"
#include "wiring_private.h"
#include "display_spi.h"
#include "lcd_spi_registers.h"
#include "mcu_spi_magic.h"

#define TFTLCD_DELAY16  0xFFFF
#define TFTLCD_DELAY8   0x7F
#define MAX_REG_NUM     24


static uint8_t display_buffer[WIDTH * HEIGHT] = {0};

/**
 * @brief Generates a new instance of the DISPLAY_SPI class. 
 * @details initializes the SPI and LCD pins including CS, RS, RESET 
 */
DISPLAY_SPI::DISPLAY_SPI()
{
	pinMode(CS, OUTPUT);	  // Enable outputs
	pinMode(RS, OUTPUT);
	pinMode(RESET, OUTPUT);
	if(LED >= 0)
	{
		pinMode(LED, OUTPUT);
		digitalWrite(LED, HIGH);
	}
	digitalWrite(RESET, HIGH);

	spi = new SPIClass(HSPI);
  	spi->begin();
  	spi->setBitOrder(MSBFIRST);
	spi->setDataMode(SPI_MODE0);

	xoffset = 0;
	yoffset = 0;
	rotation = 0;
	setWriteDir();
}

#pragma region public methods
/**
 * @brief Pass 8-bit (each) R,G,B, get back 16-bit packed color
 * @param r - Red color value
 * @param g - Green color value
 * @param b - Blue color value
 * @returns 16-bit packed color value
 */
uint16_t DISPLAY_SPI::RGB_to_565(uint8_t r, uint8_t g, uint8_t b)
{
	return ((r& 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
}

/**
 * @brief Draws a bitmap on the display
 * @param x - X coordinate of the upper left corner
 * @param y - Y coordinate of the upper left corner
 * @param width - Width of the bitmap
 * @param height - Height of the bitmap
 * @param BMP - Pointer to the bitmap data
 * @param mode - Bitmap mode (normal or inverse)
 */
void DISPLAY_SPI::draw_bitmap(uint8_t x,uint8_t y,uint8_t width, uint8_t height, uint8_t *BMP, uint8_t mode)
{
  uint8_t i,j,k;
  uint8_t tmp;
  for(i=0;i<(height+7)/8;i++)
  {
		for(j=0;j<width;j++)
		{
		    if(mode)
			{
				tmp = pgm_read_byte(&BMP[i*width+j]);
			}
			else
			{
				tmp = ~(pgm_read_byte(&BMP[i*width+j]));
			}
			for(k=0;k<8;k++)
			{
				if(tmp&0x01)
				{
					draw_pixel(x+j, y+i*8+k,1);
				}
				else
				{
					draw_pixel(x+j, y+i*8+k,0);
				}
				tmp>>=1;
			}
		}
   } 
}

/**
 * @brief Draws a pixel of a certain color at a certain location
 * @param x - x coordinate of the pixel
 * @param y - y coordinate of the pixel
 * @param color - the color of the pixel to set
 */
void DISPLAY_SPI::draw_pixel(int16_t x, int16_t y, uint16_t color)
{
	if((x < 0) || (y < 0) || (x > get_width()) || (y > get_height()))
	{
		return;
	}
	set_addr_window(x, y, x, y);
	CS_ACTIVE;
	writeCmd8(CC);
	writeData18(color);
	CS_IDLE;
}

/**
 * @brief Fill area from x to x+w, y to y+h
 * @param x - x Coordinate
 * @param y - y Coordinate
 * @param w - width
 * @param h - height
 * @param color - color
 */
void DISPLAY_SPI::fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	int16_t end;
	if (w < 0) 
	{
        w = -w;
        x -= w;
    }                           //+ve w
    end = x + w;
    if (x < 0)
    {
        x = 0;
    }
    if (end > get_width())
    {
        end = get_width();
    }
    w = end - x;
    if (h < 0) 
	{
        h = -h;
        y -= h;
    }                           //+ve h
    end = y + h;
    if (y < 0)
    {
        y = 0;
    }
    if (end > get_height())
    {
        end = get_height();
    }
    h = end - y;
    set_addr_window(x, y, x + w - 1, y + h - 1);
	CS_ACTIVE;
	writeCmd8(CC);		
	if (h > w) 
	{
        end = h;
        h = w;
        w = end;
    }
	while (h-- > 0) 
	{
		end = w;
		do 
		{
			writeData18(color);
		} while (--end != 0);
	}
	CS_IDLE;
}

/**
 * @brief Gets teh display height
 * @returns The display height
 */
int16_t DISPLAY_SPI::get_height() const
{
	return height;
}

/**
 * @brief Gets the current display rotation
 * @returns the rotation: 
 * 		0  :  0 degree 
 *		1  :  90 degree
 *		2  :  180 degree
 *		3  :  270 degree
 */
uint8_t DISPLAY_SPI::get_rotation(void) const
{
	return rotation;
}

/**
 * @brief Gets the display width
 * @returns The display width
 */
int16_t DISPLAY_SPI::get_width() const
{
	return width;
}

/**
 * @brief Initializes the display
 */
void DISPLAY_SPI::init()
{
	reset();
	toggle_backlight(true);
	start_display();
}

/**
 * @brief Inverts the display
 * @param invert - True to invert, false to revert inversion
 */
void DISPLAY_SPI::invert_display(boolean invert)
{
	CS_ACTIVE;
	uint8_t val = VL^invert;
	writeCmd8(val ? 0x21 : 0x20);
	CS_IDLE;
}

/**
 * @brief Push color table for 16 bits to controller
 * @param block - the color table
 * @param n - the number of colors in the table
 * @param first - true to first send an initialization command
 * @param flags - flags
 */
void DISPLAY_SPI::push_color_table(uint16_t * block, int16_t n, bool first, uint8_t flags)
{
	uint16_t color;
    uint8_t h, l;
	bool isconst = flags & 1;
    CS_ACTIVE;
    if (first) 
	{  
		writeCmd8(CC);		
    }
    while (n-- > 0) 
	{
        if (isconst) 
		{
			color = pgm_read_word(block++);		
        } 
		else 
		{
			color = (*block++);			

		}
		writeData18(color);
	}
    CS_IDLE;
}

/**
 * @brief Push color table for 8 bits to controller
 * @param block - the color table
 * @param n - the number of colors in the table
 * @param first - true to first send an initialization command
 * @param flags - flags
 */
void DISPLAY_SPI::push_color_table(uint8_t * block, int16_t n, bool first, uint8_t flags)
{
	uint16_t color;
    uint8_t h, l;
	bool isconst = flags & 1;
	bool isbigend = (flags & 2) != 0;
    CS_ACTIVE;
    if (first) 
	{
		writeCmd8(CC);		
    }
    while (n-- > 0) 
	{
        if (isconst) 
		{
            h = pgm_read_byte(block++);
            l = pgm_read_byte(block++);
        } 
		else 
		{
		    h = (*block++);
            l = (*block++);
		}
        color = (isbigend) ? (h << 8 | l) :  (l << 8 | h);
		
			writeData18(color);
	}
    CS_IDLE;
}

/**
 * @brief Reset the display
 */
void DISPLAY_SPI::reset()
{
	CS_IDLE;
	RD_IDLE;
    WR_IDLE;

    digitalWrite(RESET, LOW);
    delay(2);
    digitalWrite(RESET, HIGH);
  
  	CS_ACTIVE;
  	CD_COMMAND;
  	write8(0x00);
  	for(uint8_t i=0; i<3; i++)
  	{
  		WR_STROBE;
  	}
  	CS_IDLE;
}

/**
 * @brief Set display rotation
 * @param r - The Rotation to set. 
 * 					 0 - 0 degree
 * 					 1 - 90 degree
 * 					 2 - 180 degree
 * 					 3 - 270 degree
 */
void DISPLAY_SPI::set_rotation(uint8_t r)
{
    rotation = r & 3;           // just perform the operation ourselves on the protected variables
    width = (rotation & 1) ? WIDTH : HEIGHT;
    height = (rotation & 1) ? HEIGHT : WIDTH;
	CS_ACTIVE;

	uint8_t val;
	switch (rotation) 
	{			
		case 0:
		    val = ILI9488_MADCTL_MX | ILI9488_MADCTL_MY | ILI9488_MADCTL_BGR ; //0 degree 
		    break;
		case 1:
		    val = ILI9488_MADCTL_MV | ILI9488_MADCTL_MY | ILI9488_MADCTL_BGR ; //90 degree 
		    break;
		case 2:
		    val = ILI9488_MADCTL_ML | ILI9488_MADCTL_BGR; //180 degree 
		    break;
		case 3:
		    val = ILI9488_MADCTL_MX | ILI9488_MADCTL_ML | ILI9488_MADCTL_MV | ILI9488_MADCTL_BGR; //270 degree
		    break;
	}
	writeCmdData8(MD, val); 
 	set_addr_window(0, 0, width - 1, height - 1);
	vert_scroll(0, HEIGHT, 0);
	CS_IDLE;
}

/**
 * @brief Toggles the backlight on or off if an LED Pin is connected
 * @param state - true to turn the backlight on, false to turn it off. 
 */
void DISPLAY_SPI::toggle_backlight(boolean state)
{
	if(LED >= 0)
	{
		if(state)
		{
			digitalWrite(LED, HIGH);
		}
		else
		{
			digitalWrite(LED, LOW);
		}
	}
}

/**
 * @brief Scrolls the display vertically
 * @param top - the top of the scroll 
 * @param scrollines - the number of the lines to scroll
 * @param offset - offset from the top
 */
void DISPLAY_SPI::vert_scroll(int16_t top, int16_t scrollines, int16_t offset)
{
    int16_t bfa;
    int16_t vsp;
    int16_t sea = top;
	bfa = HEIGHT - top - scrollines; 
    if (offset <= -scrollines || offset >= scrollines)
    {
		offset = 0; //valid scroll
    }
	vsp = top + offset; // vertical start position
    if (offset < 0)
    {
        vsp += scrollines;          //keep in unsigned range
    }
    sea = top + scrollines - 1;

  	uint8_t d[6];           // for multi-byte parameters
  	d[0] = top >> 8;        //TFA
  	d[1] = top;
  	d[2] = scrollines >> 8; //VSA
  	d[3] = scrollines;
  	d[4] = bfa >> 8;        //BFA
  	d[5] = bfa;
	push_command(SC1, d, 6);
	d[0] = vsp >> 8;        //VSP
  	d[1] = vsp;
	push_command(SC2, d, 2);
	if (offset == 0) 
	{
		push_command(0x13, NULL, 0);
	}
}

#pragma endregion

#pragma region protected methods
/**
 * @brief Read graphics RAM data
 * @param x - x Coordinate to start reading from
 * @param y - y Coordinate to start reading from
 * @param block - Pointer to word array to write the data
 * @param w - Width of the area to read
 * @param h - height of the area to read
 * @returns The number of words read
 */
int16_t DISPLAY_SPI::read_GRAM(int16_t x, int16_t y, uint16_t *block, int16_t w, int16_t h)
{
	uint16_t ret, dummy;
    int16_t n = w * h;
	int16_t cnt = 0;
    uint8_t r, g, b, tmp;
    set_addr_window(x, y, x + w - 1, y + h - 1);
    while (n > 0) 
	{
        CS_ACTIVE;
		writeCmd16(RC);
        setReadDir();

		read8(r);
        while (n) 
		{
			if(R24BIT == 1)
			{
        		read8(r);
         		read8(g);
        		read8(b);
            	ret = RGB_to_565(r, g, b);
			}
			else if(R24BIT == 0)
			{
				read16(ret);
			}
            *block++ = ret;
            n--;
			cnt++;
        }
        CS_IDLE;
        setWriteDir();
    }
	return cnt;
}


/**
 * @brief Read the value from LCD register
 * @param reg - the register to read
 * @param index - the number of words to read
 */
uint16_t DISPLAY_SPI::read_reg(uint16_t reg, int8_t index)
{
	uint16_t ret,high;
    uint8_t low;
	CS_ACTIVE;
    writeCmd16(reg);
    setReadDir();
    delay(1); 
	do 
	{ 
		read16(ret);
	} while (--index >= 0);  
    CS_IDLE;
    setWriteDir();
    return ret;
}

/**
 * @brief Sets the LCD address window 
 * @param x1 - Upper left x
 * @param y1 - Upper left y
 * @param x2 - Lower right x
 * @param y2 - Lower right y
 */
void DISPLAY_SPI::set_addr_window(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	CS_ACTIVE;
	uint8_t x_buf[] = {x1>>8,x1&0xFF,x2>>8,x2&0xFF};
	uint8_t y_buf[] = {y1>>8,y1&0xFF,y2>>8,y2&0xFF};
	push_command(XC, x_buf, 4);
	push_command(YC, y_buf, 4);
	CS_IDLE;		
}

/**
 * @brief Starts the display, initializes registers
 */
void DISPLAY_SPI::start_display()
{
	reset();
	delay(200);
		
	static const uint8_t ILI9488_IPF[] PROGMEM ={0x3A,1,0x66};
	init_table8(ILI9488_IPF, sizeof(ILI9488_IPF));

	XC=ILI9488_COLADDRSET,YC=ILI9488_PAGEADDRSET,CC=ILI9488_MEMORYWRITE,RC=HX8357_RAMRD,SC1=0x33,SC2=0x37,MD=ILI9488_MADCTL,VL=0,R24BIT=1;
	static const uint8_t ILI9488_regValues[] PROGMEM = 
	{
		0xF7, 4, 0xA9, 0x51, 0x2C, 0x82,
		0xC0, 2, 0x11, 0x09,
		0xC1, 1, 0x41,
		0xC5, 3, 0x00, 0x0A, 0x80,
		0xB1, 2, 0xB0, 0x11,
		0xB4, 1, 0x02,
		0xB6, 2, 0x02, 0x22,
		0xB7, 1, 0xC6,
		0xBE, 2, 0x00, 0x04,
		0xE9, 1, 0x00,
		0x36, 1, 0x08,
		0x3A, 1, 0x66,
		0xE0, 15, 0x00, 0x07, 0x10, 0x09, 0x17, 0x0B, 0x41, 0x89, 0x4B, 0x0A, 0x0C, 0x0E, 0x18, 0x1B, 0x0F,
		0xE1, 15, 0x00, 0x17, 0x1A, 0x04, 0x0E, 0x06, 0x2F, 0x45, 0x43, 0x02, 0x0A, 0x09, 0x32, 0x36, 0x0F,
		0x11, 0,
		//TFTLCD_DELAY8, 120,
		0x29, 0
	};
	init_table8(ILI9488_regValues, sizeof(ILI9488_regValues));

	//set_rotation(rotation); 
	//invert_display(false);
}

/**
 * @brief Writes the display buffer contents to the display
 * @remarks The display buffer is stored in the static display_buffer array
 */
void DISPLAY_SPI::write_display_buffer()
{
	uint8_t i, n;	
	CS_ACTIVE;
	for(i=0; i<HEIGHT; i++)  
	{  
		writeCmd8(YC+i);    
		writeCmd8(0x02); 
		writeCmd8(XC); 
		for(n=0; n<WIDTH; n++)
		{
			writeData8(display_buffer[i*WIDTH+n]); 
		}
	} 
	CS_IDLE;
}
#pragma endregion

#pragma region private methods
/**
 * @brief Pushes initialization data and commands to the display controller
 * @details This method uses byte data. The first byte is a command, the second the number of parameters, followed by all the parameters, then next command etc.....
 * @param table - Pointer to table of byte data
 * @param size - The number of bytes in the table 
 */
void DISPLAY_SPI:: init_table8(const void *table, int16_t size)
{
	uint8_t i;
    uint8_t *p = (uint8_t *) table, dat[MAX_REG_NUM]; 
    while (size > 0) 
	{
        uint8_t cmd = pgm_read_byte(p++);
        uint8_t len = pgm_read_byte(p++);
        if (cmd == TFTLCD_DELAY8) 
		{
            delay(len);
            len = 0;
        } 
		else 
		{
            for (i = 0; i < len; i++)
            {
                dat[i] = pgm_read_byte(p++);
            }
			push_command(cmd,dat,len);
        }
        size -= len + 2;
    }
}

/**
 * @brief Pushes initialization data and commands to the display controller
 * @details This method uses word data. The first word is a command, the second the number of parameters, followed by all the parameters, then next command etc.....
 * @param table - Pointer to table of word data
 * @param size - The number of words in the table 
 */
void DISPLAY_SPI:: init_table16(const void *table, int16_t size)
{
    uint16_t *p = (uint16_t *) table;
    while (size > 0) 
	{
        uint16_t cmd = pgm_read_word(p++);
        uint16_t d = pgm_read_word(p++);
        if (cmd == TFTLCD_DELAY16)
        {
            delay(d);
        }
        else 
		{
			write_cmd_data(cmd, d);
		}
        size -= 2 * sizeof(int16_t);
    }
}

/**
 * @brief Writes command and data block to the display controller
 * @param cmd - The command to write
 * @param data - The block of data to write
 * @param data_size - Size of the data block
 */
void DISPLAY_SPI::push_command(uint8_t cmd, uint8_t *data, int8_t data_size)
{
  	CS_ACTIVE;
	writeCmd16(cmd);
	while (data_size-- > 0) 
	{
        uint8_t u8 = *data++;
        writeData8(u8); 
    }
    CS_IDLE;
}

/**
 * @brief Read data from the SPI bus
 * @return the data read from the bus
 */
uint8_t DISPLAY_SPI::spi_read()
{
	return spi->transfer(0xFF);
}

/**
 * @brief Performs a write on the SPI bus.
 * @param data - data to write
 */
void DISPLAY_SPI::spi_write(uint8_t data)
{
	spi->transfer(data);
}

/**
 * @brief Writes a command to the display controller.
 * @param cmd - Command to write
 */
void DISPLAY_SPI::write_cmd(uint16_t cmd)
{
	CS_ACTIVE;
	writeCmd16(cmd);
	CS_IDLE;
}

/**
 * @brief Writes data to the display controller.
 * @param data - Data to write
 */
void DISPLAY_SPI::write_data(uint16_t data)
{
	CS_ACTIVE;
	writeData16(data);
	CS_IDLE;
}

/**
 * @brief Writes command and data combination to the display controller.
 * @param cmd - Command to write
 * @param data - Data to write
 */
void DISPLAY_SPI::write_cmd_data(uint16_t cmd, uint16_t data)
{
	CS_ACTIVE;
	writeCmdData16(cmd,data);
	CS_IDLE;
}
#pragma endregion



