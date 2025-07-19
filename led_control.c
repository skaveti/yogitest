#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pigpio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>

#include "ssd1306.h"

#define CLK 17
#define DT  22
#define SW  27

#define I2C_BUS 1

bool running = false;
float current_temperature = 0.0f;
pthread_mutex_t lock;

static int clk_pin, dt_pin, sw_pin;
static uint8_t prev_state;

volatile int encoder_count = 0;
volatile uint8_t state = 0;

void sm_button_pressed() {
    printf("Button pressed!\n");
}

void encoder_callback(int gpio, int level, uint32_t tick) {
    static const uint8_t transition_table[4][4] = {
        {0, 1, 3, 0},  // from state 0
        {1, 1, 2, 0},  // from state 1
        {2, 1, 2, 3},  // from state 2
        {3, 0, 2, 3}   // from state 3
    };

    int clk = gpioRead(CLK);
    int dt  = gpioRead(DT);
    uint8_t input = (clk << 1) | dt;

    uint8_t next_state = transition_table[state][input];
    if (state == 0 && next_state == 1) encoder_count++;
    else if (state == 0 && next_state == 3) encoder_count--;

    state = next_state;
}


void button_callback(int gpio, int level, uint32_t tick) {
    if (level == 0) sm_button_pressed();
}

int encoder_init(int clk, int dt, int sw) {
    clk_pin = clk; dt_pin = dt; sw_pin = sw;

    gpioSetMode(clk, PI_INPUT); gpioSetPullUpDown(clk, PI_PUD_UP);
    gpioSetMode(dt,  PI_INPUT); gpioSetPullUpDown(dt,  PI_PUD_UP);
    gpioSetMode(sw,  PI_INPUT); gpioSetPullUpDown(sw,  PI_PUD_UP);

    prev_state = (gpioRead(clk) << 1) | gpioRead(dt);

    // gpioSetAlertFunc(clk, encoder_callback);
    // gpioSetAlertFunc(dt, encoder_callback);
    gpioSetAlertFunc(sw,  button_callback);

    return 0;
}

float read_temp(const char *device_path) {
    FILE *fp = fopen(device_path, "r");
    if (!fp) {
        perror("Failed to open device file");
        return -1000.0;
    }

    char buf[256];
    char *temp_ptr = NULL;
    float temp_c = -1000.0;

    while (fgets(buf, sizeof(buf), fp)) {
        if ((temp_ptr = strstr(buf, "t="))) {
            temp_c = atof(temp_ptr + 2) / 1000.0;
            break;
        }
    }

    fclose(fp);
    return temp_c;
}

void* temperature_sensor_loop(void* arg) {
    const char *device_file = "/sys/bus/w1/devices//28-3c9fe3811386/w1_slave";

    bool keep_running = true;

    while (keep_running) {
        float temp = read_temp(device_file);
        if (temp > -100.0)
            printf("Temperature: %.2f °C\n", temp);
        else
            printf("Sensor read error.\n");

        sleep(1);
        pthread_mutex_lock(&lock);
        current_temperature = temp;
        keep_running = running;
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}


#define LED_GPIO 16  // Change this to your GPIO pin

#define PORT 8080
#define BUF_SIZE 1024

int main() {
    if (gpioInitialise() < 0) {
        fprintf(stderr, "pigpio initialization failed\n");
        return 1;
    }

    printf("Starting ...\n");
    running = true;

    encoder_init(CLK, DT, SW);

    int handle = i2cOpen(I2C_BUS, SSD1306_I2C_ADDR, 0);
    if (handle < 0) {
        printf("I2C open failed\n");
        return 1;
    }

    ssd1306_startup(handle);

    /*
    int result = ssd1306_oled_default_config(handle);
    if (result != 0) {
        printf("I2C initialization failed\n");
        return 1;
    }
    ssd1306_oled_clear_screen(handle);
    ssd1306_oled_set_line(handle, 0, 0xFF);
    */

    pthread_t t1;
    pthread_mutex_init(&lock, NULL);
    pthread_create(&t1, NULL, temperature_sensor_loop, NULL);

    gpioSetMode(LED_GPIO, PI_OUTPUT);

    printf("Turning LED ON\n");
    gpioWrite(LED_GPIO, 1);
    sleep(1);

    printf("Starting TCP server\n");

    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUF_SIZE] = {0};
    socklen_t addr_len = sizeof(client_addr);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed");
        return 5;
    }

    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return 2;
    }

    // Listen
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        return 3;
    }

    printf("Server listening on port %d\n", PORT);

    // Accept connection
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
        perror("Accept failed");
        close(server_fd);
        return 4;
    }

    read(client_fd, buffer, BUF_SIZE);
    printf("Received: %s\n", buffer);

    pthread_mutex_lock(&lock);
    running = false;
    pthread_mutex_unlock(&lock);

    send(client_fd, "Hello from server!", 19, 0);
    printf("Reply sent\n");

    close(client_fd);
    close(server_fd);

    pthread_join(t1, NULL);
    printf("Final temperature = %f\n", current_temperature);

    printf("Turning LED OFF\n");
    gpioWrite(LED_GPIO, 0);

    // ssd1306_oled_clear_screen(handle);
    i2cClose(handle);
    gpioTerminate();

    return 0;
}
