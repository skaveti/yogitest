CC=gcc
CFLAGS=-Wall -lpigpio -lrt -lpthread
TARGET=led_control

all: $(TARGET)

$(TARGET): led_control.c ssd1306.c ssd1306.h font.h
	$(CC) -std=gnu17 -o $(TARGET) led_control.c ssd1306.c $(CFLAGS)

clean:
	rm -f $(TARGET)


