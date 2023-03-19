#include "udpz.h"

int main(int argc, char *argv[])
{
    UdpTransceiver<std::string> server;
    if (server.InitServer("127.0.0.1", 9000, 100) != 0) {
        return 0;
    }
    std::string userData;
    int size = 0;
    int ret = -1;
    while (true) {
        ret = server.RecvMsg(userData, size);
        if (ret != 0) {
            std::cout << "Receive data size: " << ret << std::endl;
            std::cout << "Receive data content: " << userData << std::endl;
        }
    }
    return 0;
}