#include <stdio.h>
#include <string.h>
#include <pigpio.h>
#include <unistd.h>
#include <arpa/inet.h>

#define LED_GPIO 16  // Change this to your GPIO pin

#define PORT 8080
#define BUF_SIZE 1024

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

    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUF_SIZE] = {0};
    socklen_t addr_len = sizeof(client_addr);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Accept connection
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    read(client_fd, buffer, BUF_SIZE);
    printf("Received: %s\n", buffer);

    send(client_fd, "Hello from server!", 19, 0);
    printf("Reply sent\n");

    close(client_fd);
    close(server_fd);

    printf("Turning LED OFF\n");
    gpioWrite(LED_GPIO, 0);

    gpioTerminate();
    return 0;
}
