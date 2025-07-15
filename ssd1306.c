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
    uint8_t buf[2] = {0x00, cmd}; // Command mode
    i2cWriteDevice(handle, (char *)buf, 2);
}

void write_data(int handle, uint8_t *data, int len) {
    uint8_t *buf = malloc(len + 1);
    buf[0] = 0x40; // Data mode
    memcpy(buf + 1, data, len);
    i2cWriteDevice(handle, (char *)buf, len + 1);
    free(buf);
}

