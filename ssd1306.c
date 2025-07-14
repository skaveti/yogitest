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

#include "ssd1306.h"
#include "font.h"

static uint8_t data_buf[1024];
static uint8_t max_lines = 0;
static uint8_t max_columns = 0;
static uint8_t global_x = 0;
static uint8_t global_y = 0;
