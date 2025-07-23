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
static uint8_t max_pages = 0;
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

void enable(int handle, uint8_t on)
{
    write_cmd(handle, SSD1306_COMM_DISPLAY_OFF | on);
}

void ssd1306_startup(int handle)
{
    uint8_t oled_lines;
    uint8_t oled_columns;

    oled_lines = SSD1306_HEIGHT;
    oled_columns = SSD1306_WIDTH;

    printf("Number of lines / columns = %d, %d\n", oled_lines, oled_columns);

    max_lines = oled_lines;
    max_pages = max_lines / 8;
    max_columns = oled_columns;
    global_x = 0;
    global_y = 0;

    // Init sequence (partial)
    uint8_t init_cmds_full[] = {
        0xAE, 0xD5, 0x80, 0xA8, 0x3F,
        0xD3, 0x00, 0x40, 0x8D, 0x14,
        0x20, 0x00, 0xA1, 0xC8, 0xDA,
        0x12, 0x81, 0xCF, 0xD9, 0xF1,
        0xDB, 0x40, 0xA4, 0xA6, 0xAF
    };

    // Init sequence (partial)
    uint8_t init_cmds[] = {
        0xA8, 0x3F, 0xD3, 0x00, 0x40,
        0xA1, 0xC8, 0xDA, 0x12, 0x81,
        0x7F, 0xA4, 0xA6, 0xD5, 0x80,
        0x8D, 0x14, 0x20, 0x00, 0xAF
    };

    for (int i = 0; i < sizeof(init_cmds); i++) {
        write_cmd(handle, init_cmds[i]);
    }
}

void ssd1306_fill_page(int handle, uint8_t page, uint8_t value)
{
    uint8_t fill_cmds[] = {
        SSD1306_COMM_SET_COL_ADDR, 0x00, 0x7F,
        SSD1306_COMM_SET_PAGE_ADDR, page, page
    };

    for (int i = 0; i < sizeof(fill_cmds); i++) {
        write_cmd(handle, fill_cmds[i]);
    }

    for (int i = 0; i < max_columns; i++)
        data_buf[i] = value;
    write_data(handle, data_buf, max_columns);
}

void ssd1306_clear(int handle)
{
    uint8_t fill_cmds[] = {
        SSD1306_COMM_SET_COL_ADDR, 0x00, 0x7F,
        SSD1306_COMM_SET_PAGE_ADDR, 0x00, 0x07
    };

    for (int i = 0; i < sizeof(fill_cmds); i++) {
        write_cmd(handle, fill_cmds[i]);
    }

    for (int i = 0; i < max_pages*max_columns; i++)
        data_buf[i] = 0x00;
    for (int i = 0; i < 2*max_columns; i++)
        data_buf[i] = 0xFF;
    write_data(handle, data_buf, max_pages*max_columns);
}

/*
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
*/
