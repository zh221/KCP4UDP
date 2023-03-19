#include "udpz.h"

int main(int argc, char *argv[])
{
    std::string s = "hello 123.";
    uint8_t *data = new uint8_t[s.size() + 1];
    memcpy(data, &s, s.size());
    std::cout << data << std::endl;
    // UdpTransceiver<std::string> client;
    // if (client.InitClient("127.0.0.1", 9000) != 0) {
    //     return 0;
    // }
    
    // std::string userData = "hello udp.";
    // int ret = 0;
    // while (true) {
    //     ret = client.SendMsg(userData, userData.size());
    //     if (ret < 0) {
    //         std::cout << "Failed to send data." << std::endl;
    //     }
    //     std::cout << "Send data size: " << ret << std::endl;
    // }
    return 0;
}