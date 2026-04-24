#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>
#include <cstdlib>
#include <gpiod.h>

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;

constexpr const char* CHIP_NAME = "/dev/gpiochip0";
constexpr unsigned int GPIO_PIN = 17; // BCM GPIO 17 = physical pin 11

int ledVal = 0;

gpiod_chip* chip = nullptr;
gpiod_line_request* request = nullptr;

int setupLED() {
    chip = gpiod_chip_open(CHIP_NAME);

    if (chip == nullptr) {
        std::cerr << "Failed to open GPIO chip\n";
        return 1;
    }

    gpiod_line_settings* settings = gpiod_line_settings_new();

    if (settings == nullptr) {
        std::cerr << "Failed to create line settings\n";
        gpiod_chip_close(chip);
        return 1;
    }

    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);

    gpiod_line_config* lineConfig = gpiod_line_config_new();

    if (lineConfig == nullptr) {
        std::cerr << "Failed to create line config\n";
        gpiod_line_settings_free(settings);
        gpiod_chip_close(chip);
        return 1;
    }

    gpiod_line_config_add_line_settings(lineConfig, &GPIO_PIN, 1, settings);

    gpiod_request_config* requestConfig = gpiod_request_config_new();

    if (requestConfig == nullptr) {
        std::cerr << "Failed to create request config\n";
        gpiod_line_config_free(lineConfig);
        gpiod_line_settings_free(settings);
        gpiod_chip_close(chip);
        return 1;
    }

    gpiod_request_config_set_consumer(requestConfig, "led-control");

    request = gpiod_chip_request_lines(chip, requestConfig, lineConfig);

    gpiod_request_config_free(requestConfig);
    gpiod_line_config_free(lineConfig);
    gpiod_line_settings_free(settings);

    if (request == nullptr) {
        std::cerr << "Failed to request GPIO line\n";
        gpiod_chip_close(chip);
        return 1;
    }

    std::cout << "GPIO setup complete on BCM GPIO " << GPIO_PIN << std::endl;

    return 0;
}

int toggleLED() {
    if (request == nullptr) {
        std::cerr << "GPIO request is not initialized\n";
        return 1;
    }

    ledVal = !ledVal;

    gpiod_line_value value;

    if (ledVal == 1) {
        value = GPIOD_LINE_VALUE_ACTIVE;
    } else {
        value = GPIOD_LINE_VALUE_INACTIVE;
    }

    int result = gpiod_line_request_set_value(request, GPIO_PIN, value);

    if (result < 0) {
        std::cerr << "Failed to set GPIO value\n";
        return 1;
    }

    std::cout << "LED value set to: " << ledVal << std::endl;

    return 0;
}

void cleanupLED() {
    if (request != nullptr) {
        gpiod_line_request_release(request);
        request = nullptr;
    }

    if (chip != nullptr) {
        gpiod_chip_close(chip);
        chip = nullptr;
    }
}

int tcp_server() {
    int server_fd;
    int new_socket;

    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));

    int opt = 1;
    socklen_t addrlen = sizeof(address);

    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        perror("socket failed");
        return 1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt SO_REUSEADDR");
        close(server_fd);
        return 1;
    }

#ifdef SO_REUSEPORT
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt SO_REUSEPORT");
        close(server_fd);
        return 1;
    }
#endif

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        return 1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        new_socket = accept(server_fd, (sockaddr*)&address, &addrlen);

        if (new_socket < 0) {
            perror("accept failed");
            continue;
        }

        std::memset(buffer, 0, BUFFER_SIZE);

        ssize_t valread = read(new_socket, buffer, BUFFER_SIZE - 1);

        if (valread > 0) {
            buffer[valread] = '\0';

            std::cout << "Received: [" << buffer << "]" << std::endl;

            if (std::strncmp(buffer, "LED", 3) == 0) {
                std::cout << "Toggling LED" << std::endl;
                toggleLED();
            }

            send(new_socket, buffer, valread, 0);
            std::cout << "Echo message sent" << std::endl;
        } else if (valread == 0) {
            std::cout << "Client disconnected without sending data" << std::endl;
        } else {
            perror("read failed");
        }

        close(new_socket);
    }

    close(server_fd);
    return 0;
}

int main() {
    if (setupLED() != 0) {
        return 1;
    }

    int result = tcp_server();

    cleanupLED();

    return result;
}