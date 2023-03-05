#ifndef UDPZ_H
#define UDPZ_H

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "unistd.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "netinet/in.h"
#include "arpa/inet.h"

class UdpZ
{
public:
    ~UdpZ()
    {
        if (this->_sockfd != -1) {
            close(this->_sockfd);
        }
    }
    int InitServer(const std::string &hostIP, const int hostPort)
    {
        this->_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (this->_sockfd < 0) {
            std::cout << "Create sockfd failed." << std::endl;
            return -1;
        }
        bzero(&this->_localAddr, sizeof(this->_localAddr));
        this->_localAddr.sin_family = AF_INET;
        this->_localAddr.sin_addr.s_addr = inet_addr(hostIP.c_str());
        this->_localAddr.sin_port = htons(hostPort);

        if (bind(this->_sockfd, (const struct sockaddr*)&this->_localAddr, sizeof(this->_localAddr)) < 0) {
            std::cout << "Bind socket failed." << std::endl;
            return -1;
        }
        return 0;
    }
    int InitClient(const char *servIP, const int servPort)
    {
        this->_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (this->_sockfd < 0) {
            std::cout << "Create sockfd failed." << std::endl;
            return -1;
        }
        bzero(&this->_remoteAddr, sizeof(this->_remoteAddr));
        this->_remoteAddr.sin_family = AF_INET;
        this->_remoteAddr.sin_addr.s_addr = inet_addr("169.254.115.229");
        this->_remoteAddr.sin_port = htons(9000);
        return 0;
    }
    int SnedMsg(const std::string &msgStr)
    {
        int ret = sendto(this->_sockfd, msgStr.c_str(), msgStr.size(), 0,
                         (const struct sockaddr*)&this->_remoteAddr, sizeof(this->_remoteAddr));
        std::cout << "send : " << ret << std::endl;
        return 0;
    }
    int RecvMsg(char *msgBuf, const int bufLen)
    {
        char buf[1024] = {0};
        socklen_t remoteAddrLen = sizeof(this->_remoteAddr);
        bzero(&this->_remoteAddr, remoteAddrLen);
        
        int ret = recvfrom(this->_sockfd, buf, sizeof(buf) - 1, 0, 
                 (struct sockaddr*)&this->_remoteAddr, &remoteAddrLen);
        if (ret > 0) std::cout<< buf << std::endl; 
        
        if (bufLen >= sizeof(buf)) {
            memcpy(msgBuf, buf, strlen(buf));
        }
        return 0;
        
    }
private:
    int _sockfd = -1;
    struct sockaddr_in _localAddr;
    struct sockaddr_in _remoteAddr;
};

#endif