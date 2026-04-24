#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>

#include <iostream>

#include <unistd.h>

#include <gpiod.h>

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;

int tcp_server() {

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt SO_REUSEPORT");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server listening on port " << PORT << std::endl;
    while (true) {

        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0 ) {
            perror("listen");
            exit(EXIT_FAILURE);
        }
    
        ssize_t valread = read(new_socket, buffer, BUFFER_SIZE);
        std::cout << "Received: " << buffer << std::endl;

        if (std::strcmp(buffer, "penis") == 0) {
            turnOnLED();
        }
        send(new_socket, buffer, valread, 0);
        std::cout << "echo message sent" << std::endl;
        close(new_socket);

    }
    close(server_fd);
    return 0;
}




int ledVal = 0;

int turnOnLED() {

    const char* chipName = "/dev/gpiochip0";
    const unsigned int gpioPin = 17;
    gpiod_chip* chip = gpiod_chip_open(chipName);

    if (chip == nullptr) {
        std::cerr << "Failed to open GPIO\n";
        return 1;
    }

    gpiod_line* line = gpiod_chip_get_line(chip, gpioPin);

    if (line == nullptr) {
        std::cerr << "Failed to get GPIO line\n";
        gpiod_chip_close(chip);
        return 1;
    }

    if (gpiod_line_request_output(line, "led-control", 0) < 0) {
        std::cerr << "Failed to request GPIO line as output\n";
        gpiod_chip_close(chip);
        return 1;
    }
    gpiod_line_set_value(line, ledVal);
    ledVal++;
    ledVal = ledVal % 2;
    gpiod_line_release(line);
    gpiod_chip_close(chip);
    return 0;
}


int main() {
    tcp_server();
} 