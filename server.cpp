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

int ledVal = 0;

int turnOnLED() {
    const char *chipName = "/dev/gpiochip0";
    const unsigned int gpioPin = 17;
    gpiod_chip *chip = gpiod_chip_open(chipName);

    if (chip == nullptr)
    {
        std::cerr << "Failed to open GPIO chip\n";
        return 1;
    }

    gpiod_line_settings *settings = gpiod_line_settings_new();
    if (settings == nullptr) {
        std::cerr << "Failed to create line settings\n";\
        gpiod_chip_close(chip);
        return 1;
    }
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);

    if (ledVal == 1) {
        gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_ACTIVE);
    }
    else {
        gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);
    }

    gpiod_line_config *lineConfig = gpiod_line_config_new();

    if (lineConfig == nullptr) {
        std::cerr << "Failed to create line config\n";
        gpiod_line_settings_free(settings);
        gpiod_chip_close(chip);
        return 1;
    }

    gpiod_line_config_add_line_settings(lineConfig, &gpioPin, 1, settings);
    gpiod_request_config *requestConfig = gpiod_request_config_new();

    if (requestConfig == nullptr){
        std::cerr << "Failed to create request config\n";
        gpiod_line_config_free(lineConfig);
        gpiod_line_settings_free(settings);
        gpiod_chip_close(chip); 
        return 1;
    }

    gpiod_request_config_set_consumer(requestConfig, "led-control");

    gpiod_line_request *request = gpiod_chip_request_lines(chip, requestConfig, lineConfig);

    if (request == nullptr) {
        std::cerr << "Failed to request GPIO line\n";
        gpiod_request_config_free(requestConfig);
        gpiod_line_config_free(lineConfig);
        gpiod_line_settings_free(settings);
        gpiod_chip_close(chip);
        return 1;
    }

    std::cout << "LED value set to: " << ledVal << std::endl;
    ledVal++;
    ledVal = ledVal % 2;

    gpiod_line_request_release(request);
    gpiod_request_config_free(requestConfig);
    gpiod_line_config_free(lineConfig);
    gpiod_line_settings_free(settings);
    gpiod_chip_close(chip);
    return 0;
}

int tcp_server()
{

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt SO_REUSEPORT");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server listening on port " << PORT << std::endl;
    while (true)
    {

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        ssize_t valread = read(new_socket, buffer, BUFFER_SIZE);
        std::cout << "Received: " << buffer << std::endl;

        if (std::strcmp(buffer, "penis") == 0)
        {
            std::cout << "turning On led" << std::endl;
            turnOnLED();
        }
        send(new_socket, buffer, valread, 0);
        std::cout << "echo message sent" << std::endl;
        close(new_socket);
    }
    close(server_fd);
    return 0;
}

int main()
{
    
    tcp_server();
}