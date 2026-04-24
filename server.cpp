#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>

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
    std::string buffer = getClientAddress();
    std::cout << "Received: " << buffer << "\n";
}