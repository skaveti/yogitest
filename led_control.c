#include <stdio.h>
#include <pigpio.h>
#include <unistd.h>

#define LED_GPIO 16  // Change this to your GPIO pin

int main() {
    if (gpioInitialise() < 0) {
        fprintf(stderr, "pigpio initialization failed\n");
        return 1;
    }

    printf("Starting");

    gpioSetMode(LED_GPIO, PI_OUTPUT);

    printf("Turning LED ON\n");
    gpioWrite(LED_GPIO, 1);
    sleep(1);

    printf("Turning LED OFF\n");
    gpioWrite(LED_GPIO, 0);

    gpioTerminate();
    return 0;
}
