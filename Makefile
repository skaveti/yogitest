CC=gcc
CFLAGS=-Wall -lpigpio -lrt -lpthread
TARGET=led_control

all: $(TARGET)

$(TARGET): led_control.c
	$(CC) -std=gnu17 -o $(TARGET) led_control.c $(CFLAGS)

clean:
	rm -f $(TARGET)


