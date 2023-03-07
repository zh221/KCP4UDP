#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "udpz.h"

int main(int argc, char *argv[])
{
    // int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    // struct sockaddr_in addr_a;
    // bzero(&addr_a, sizeof(addr_a));
    // addr_a.sin_family = AF_INET;
    // addr_a.sin_addr.s_addr = inet_addr("169.254.115.229");
    // addr_a.sin_port = htons(9000);

    // if (bind(sockfd, (const struct sockaddr*)&addr_a, sizeof(addr_a)) < 0) {
    //     std::cout << "Bind socket failed." << std::endl;
    //     return -1;
    // }

    UdpZ udpz;
    udpz.InitServer("127.0.0.1", 9000, 100);

    while (true) {
        char *msgBuff = nullptr;
        // struct sockaddr_in peer;
        // socklen_t peerLen = sizeof(peer);
        // bzero(&peer, peerLen);
        // recvfrom(sockfd, in_buff, sizeof(in_buff) - 1, 0, 
        //          (struct sockaddr*)&peer, &peerLen);
        int msgLen = 0;
        int ret = udpz.RecvMsg(msgBuff, msgLen);
        std::cout << ret << std::endl;
        if (ret > 0) {
            std::cout << msgBuff << std::endl;
        }
    }


    return 0;
}