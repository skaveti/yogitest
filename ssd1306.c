#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pigpio.h>

#include "ssd1306.h"
#include "font.h"

static uint8_t data_buf[1024];
static uint8_t max_lines = 0;
static uint8_t max_columns = 0;
static uint8_t global_x = 0;
static uint8_t global_y = 0;

void write_cmd(int handle, uint8_t cmd) {
    uint8_t buf[2] = {SSD1306_COMM_CONTROL_BYTE, cmd}; // Command mode
    i2cWriteDevice(handle, (char *)buf, 2);
}

void write_data(int handle, uint8_t *data, int len) {
    uint8_t *buf = malloc(len + 1);
    buf[0] = SSD1306_DATA_CONTROL_BYTE; // Data mode
    memcpy(buf + 1, data, len);
    i2cWriteDevice(handle, (char *)buf, len + 1);
    free(buf);
}

int write_buffer(int handle, uint8_t *buffer, int len) {
    return i2cWriteDevice(handle, (char *)buffer, len);
}

int ssd1306_oled_default_config(int handle)
{
    uint8_t oled_lines; 
    uint8_t oled_columns;
      
    oled_lines = SSD1306_HEIGHT;
    oled_columns = SSD1306_WIDTH;
    
    printf("Number of lines / columns = %d, %d\n", oled_lines, oled_columns);
        
    max_lines = oled_lines;
    max_columns = oled_columns;
    global_x = 0;
    global_y = 0;
    
    uint16_t i = 0;
    data_buf[i++] = SSD1306_COMM_CONTROL_BYTE;  //command control byte
    data_buf[i++] = SSD1306_COMM_DISPLAY_OFF;   //display off
    data_buf[i++] = SSD1306_COMM_DISP_NORM;     //Set Normal Display (default)
    data_buf[i++] = SSD1306_COMM_CLK_SET;       //SETDISPLAYCLOCKDIV
    data_buf[i++] = 0x80;                       // the suggested ratio 0x80
    data_buf[i++] = SSD1306_COMM_MULTIPLEX;     //SSD1306_SETMULTIPLEX
    data_buf[i++] = oled_lines - 1;             // height is 32 or 64 (always -1)
    data_buf[i++] = SSD1306_COMM_VERT_OFFSET;   //SETDISPLAYOFFSET
    data_buf[i++] = 0;                          //no offset
    data_buf[i++] = SSD1306_COMM_START_LINE;    //SETSTARTLINE
    data_buf[i++] = SSD1306_COMM_CHARGE_PUMP;   //CHARGEPUMP
    data_buf[i++] = 0x14;                       //turn on charge pump
    data_buf[i++] = SSD1306_COMM_MEMORY_MODE;   //MEMORYMODE
    data_buf[i++] = SSD1306_HORI_MODE;          // page mode
    data_buf[i++] = SSD1306_COMM_HORIZ_NORM;    //SEGREMAP  Mirror screen horizontally (A0)
    // data_buf[i++] = SSD1306_COMM_HORIZ_FLIP;    //SEGREMAP  Mirror screen horizontally (A0)
    data_buf[i++] = SSD1306_COMM_SCAN_NORM;     //COMSCANDEC Rotate screen vertically (C0)
    data_buf[i++] = SSD1306_COMM_COM_PIN;       //HARDWARE PIN 
    if (oled_lines == 32)
        data_buf[i++] = 0x02;                       // for 32 lines
    else
        data_buf[i++] = 0x12;                       // for 64 lines or 48 lines
    data_buf[i++] = SSD1306_COMM_CONTRAST;      //SETCONTRAST
    data_buf[i++] = 0x7f;                       // default contract value
    data_buf[i++] = SSD1306_COMM_PRECHARGE;     //SETPRECHARGE
    data_buf[i++] = 0xf1;                       // default precharge value
    data_buf[i++] = SSD1306_COMM_DESELECT_LV;   //SETVCOMDETECT                
    data_buf[i++] = 0x40;                       // default deselect value
    data_buf[i++] = SSD1306_COMM_RESUME_RAM;    //DISPLAYALLON_RESUME
    data_buf[i++] = SSD1306_COMM_DISP_NORM;     //NORMALDISPLAY
    data_buf[i++] = SSD1306_COMM_DISPLAY_ON;    //DISPLAY ON             
    data_buf[i++] = SSD1306_COMM_DISABLE_SCROLL;//Stop scroll
    
    return write_buffer(handle, data_buf, i);
}

int ssd1306_oled_clear_line(int handle, uint8_t row)
{
    uint8_t i;
    if (row >= (max_lines / 8))
        return 1;
        
    ssd1306_oled_set_XY(handle, 0, row);
    data_buf[0] = SSD1306_DATA_CONTROL_BYTE;
    for (i = 0; i < max_columns; i++)
        data_buf[i+1] = 0x00;
        
    return write_buffer(handle, data_buf, 1 + max_columns);
}

int ssd1306_oled_set_line(int handle, uint8_t row, uint8_t value)
{
    uint8_t i;
    if (row >= (max_lines / 8))
        return 1;
        
    ssd1306_oled_set_XY(handle, 0, row);
    data_buf[0] = SSD1306_DATA_CONTROL_BYTE;
    for (i = 0; i < max_columns; i++)
        data_buf[i+1] = value;
        
    return write_buffer(handle, data_buf, 1 + max_columns);
}

int ssd1306_oled_clear_screen(int handle)
{
    int rc = 0;
    int status = 0;
    uint8_t i;
    
    for (i = 0; i < (max_lines / 8); i++)
    {
        rc = ssd1306_oled_clear_line(handle, i);
        if (rc != 0)
            status = rc;
    }
    
    return status;
}

int ssd1306_oled_set_X(int handle, uint8_t x)
{
    if (x >= max_columns)
        return 1;

    global_x = x;
    
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    data_buf[1] = SSD1306_COMM_LOW_COLUMN | (x & 0x0f);
    data_buf[2] = SSD1306_COMM_HIGH_COLUMN | ((x >> 4) & 0x0f);
    
    return write_buffer(handle, data_buf, 3);
}

int ssd1306_oled_set_Y(int handle, uint8_t y)
{
    if (y >= (max_lines / 8))
        return 1;

    global_y = y;
    
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    data_buf[1] = SSD1306_COMM_PAGE_NUMBER | (y & 0x0f);

    return write_buffer(handle, data_buf, 2);
}

int ssd1306_oled_set_XY(int handle, uint8_t x, uint8_t y)
{
    if (x >= max_columns || y >= (max_lines / 8))
        return 1;

    global_x = x;
    global_y = y;
    
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    data_buf[1] = SSD1306_COMM_PAGE_NUMBER | (y & 0x0f);

    data_buf[2] = SSD1306_COMM_LOW_COLUMN | (x & 0x0f);
    
    data_buf[3] = SSD1306_COMM_HIGH_COLUMN | ((x >> 4) & 0x0f);
    
    return write_buffer(handle, data_buf, 4);
}




