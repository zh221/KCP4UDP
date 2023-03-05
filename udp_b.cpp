#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "udpz.h"

int main(int argc, char *argv[])
{
    // int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    // struct sockaddr_in addr_b;
    // bzero(&addr_b, sizeof(addr_b));
    // addr_b.sin_family = AF_INET;
    // addr_b.sin_addr.s_addr = inet_addr("169.254.115.229");
    // addr_b.sin_port = htons(9000);

    UdpZ udpz;
    udpz.InitClient("169.254.115.229", 9000);

    unsigned int counter = 0;
    while (counter != 1000)
    {
        std::string msgStr = "hello a : " + std::to_string(counter);
        // int ret = sendto(sockfd, msgStr.c_str(), msgStr.size(), 0, \
        //        (const struct sockaddr*)&addr_b, sizeof(addr_b));
        //  std::cout << "send : " << ret << std::endl;

        udpz.SnedMsg(msgStr);
        sleep(1);
        counter++;
    }

    return 0;
}