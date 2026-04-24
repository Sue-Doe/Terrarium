#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>

std::string getLocalipAddress() {
    struct ifaddrs* interfaces = nullptr;
    getifaddrs(&interfaces);

    std::string ipAddress = "";
    struct ifaddrs* iface = interfaces;
    while (iface != nullptr ) {
        
        if (iface->ifa_addr == nullptr) {
            continue;
        }

        if (iface->ifa_addr->sa_family  == AF_INET) {
            char ip[INET6_ADDRSTRLEN];
            sockaddr_in* addr = (sockaddr_in*)iface->ifa_addr;

            inet_ntop(AF_INET, &(addr->sin_addr), ip, INET_ADDRSTRLEN);

            std::string defaultIP = "127.0.0.1";
            if (ip != defaultIP) {
                ipAddress = ip;
            }
            
        }
        iface = iface->ifa_next;
    }

    freeifaddrs(interfaces);
    return ipAddress;
}




int main() {
    std::string hello = getLocalipAddress();
    std::cout << hello;
    // int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    // sockaddr_in server_address;
    // std::memset(&server_address, 0, sizeof(server_address));

    // server_address.sin_family = AF_INET;
    // server_address.sin_port = htons(8080);
    // inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    // const char* message = "Hello using UDP!";

    // sendto(
    //     sock_fd,
    //     message,
    //     std::strlen(message),
    //     0,
    //     (sockaddr*)&server_address,
    //     sizeof(server_address)
    // );

    // close(sock_fd);
}