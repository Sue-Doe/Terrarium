#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>



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

    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0 ) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    ssize_t valread = read(new_socket, buffer, BUFFER_SIZE);
    std::cout << "Received: " << buffer << std::endl;
    send(new_socket, buffer, valread, 0);
    std::cout << "echo message sent" << std::endl;
    close(new_socket);
    close(server_fd);
    return 0;
}

std::string getClientAddress() {
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    bind(sock_fd, (sockaddr*)&address, sizeof(address));

    char buffer[1024];

    sockaddr_in sender_address;
    socklen_t sender_len = sizeof(sender_address);

    int bytes = recvfrom(
        sock_fd,
        buffer,
        sizeof(buffer) - 1,
        0,
        (sockaddr*)&sender_address,
        &sender_len
    );

    buffer[bytes] = '\0';

    close(sock_fd);
    return buffer;
}

int main() {
    tcp_server();
}