#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;
const char* SERVER_HOST = "192.168.0.16";

int sendTCPMessage(){
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_HOST, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address / address not supported" << std::endl;
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Invalid address/Address not supported" << std::endl;
        return -1;
    }

    const char* hello = "hello from client";
    send(sock, hello, strlen(hello), 0);
    std::cout << "Hello message sent" << std::endl;
    ssize_t valread = read(sock, buffer, BUFFER_SIZE);
    
    if (valread > 0) {
        buffer[valread] = '\0';
        std::cout << "Received: " << buffer << std::endl;
    } else if (valread == 0) {
        std::cout << "Server closed the connection" << std::endl;
    } else {
        perror("read");
    }

    close(sock);
    return 0;

}

int main() {
    sendTCPMessage();
}
