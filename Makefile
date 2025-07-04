CC=gcc
CFLAGS=-Wall -lpigpio -lrt -lpthread
TARGET=led_control

all: $(TARGET)

$(TARGET): led_control.c
	$(CC) -o $(TARGET) led_control.c $(CFLAGS)

clean:
	rm -f $(TARGET)


