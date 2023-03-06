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

#define MSG_DAT_PIECE_LEN 6388
#define MSG_TIMEOUT 30

struct MsgPiece
{
    unsigned short conv;
    unsigned short totalSize;
    unsigned short totalPiece;
    unsigned short currPiece;
    unsigned int timeSlab;
    data[MSG_DAT_PIECE_LEN];
}

const unsigned int MSG_SUB_PKG_LEN = sizeof(MsgPiece);

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
    int DetachSend(const char *msgData, int msgLen)
    {
        int totalSize = msgLen;
        int totalPiece = msgLen / MSG_DAT_PIECE_LEN;
        if (msgLen % MSG_DAT_PIECE_LEN) {
            totalPiece += 1;
        }
        int ret = -1;
        char buf[MSG_SUB_PKG_LEN];
        MsgPiece msgPiece;
        for (int i = 0; i < totalPiece - 1; i++)
        {
            memcpy(&msgPiece.data, msgData + i * MSG_DAT_PIECE_LEN, MSG_DAT_PIECE_LEN);
            msgPiece.conv = 0x01;
            msgPiece.totalSize = msgLen;
            msgPiece.totalPiece = totalPiece;
            msgPiece.currPiece = i;
            msgPiece.timeSlab = TimeSlab();
            bzero(buf, MSG_SUB_PKG_LEN);
            memcpy(buf, &msgPiece, MSG_SUB_PKG_LEN);
            ret = sendto(this->_sockfd, buf, MSG_SUB_PKG_LEN, (const sockaddr*)&this->_remoteAddr,
                         sizeof(this->_remoteAddr));
            if (ret < 0) {
                return -1;
            }
        }
        int len = msgLen - MSG_DAT_PIECE_LEN * (totalPiece - 1);
        memcpy(&msgPiece.data, msgData + (totalPiece - 1) * MSG_DAT_PIECE_LEN, len);
        msgPiece.conv = 0x01;
        msgPiece.totalSize = msgLen;
        msgPiece.totalPiece = totalPiece;
        msgPiece.currPiece = totalPiece - 1;
        msgPiece.timeSlab = TimeSlab();
        bzero(buf, MSG_SUB_PKG_LEN);
        memcpy(buf, &msgPiece, MSG_SUB_PKG_LEN);
        ret = sendto(this->_sockfd, buf, MSG_SUB_PKG_LEN, (const sockaddr*)&this->_remoteAddr,
                     sizeof(this->_remoteAddr));
        if (ret < 0) {
            return -1;
        }
        return msgLen;
    }
    int AssembleRecv(char *msgData, int &msgLen)
    {
        int ret = -1;
        socklen_t remoteAddrLen = sizeof(this->_remoteAddr);
        char buf[MSG_SUB_PKG_LEN] = {0};
        while (ret < 0) {
            ret = recvfrom(this->_sockfd, buf, sizeof(buf), (sockaddr *)&this->_remoteAddr, 0, 
                           (struct sockaddr*)&this->_remoteAddr, &remoteAddrLen);
        }
        MsgPiece *msgPiece = static_cast<MsgPiece *>(buf);
        unsigned short conv = msgPiece->conv
        std::vector<MsgPiece> msgPieceList(msgPiece->totalPiece);
        msgPieceList[msgPiece->currPiece] = *msgPiece;
        unsigned int lastTimeSlab = msgPiece->timeSlab;
        unsigned int newTimeSlab = msgPiece->timeSlab;
        int counter = 1;
        while (counter < msgPieceList.size()) {
            ret = -1;
            bzero(buf, MSG_SUB_PKG_LEN);
            ret = recvfrom(this->_sockfd, buf, sizeof(buf), (sockaddr *)&this->_remoteAddr, 0, 
                           (struct sockaddr*)&this->_remoteAddr, &remoteAddrLen);
            if (ret > 0) {
                msgPiece = static_cast<MsgPiece *>(buf);
                if (msgPiece->conv == conv)
                {
                    msgPieceList[msgPiece->currPiece] = *msgPiece;
                    counter++;
                    lastTimeSlab = msgPiece->timeSlab;
                }
            }
            newTimeSlab = TimeSlab();
            if (lastTimeSlab - newTimeSlab > MSG_TIMEOUT) {
                return -1;
            }
        }

        return msgLen;
    }
private:
    unsigned int TimeSlab()
    {
        // ...
        return 0;
    }
private:
    int _sockfd = -1;
    struct sockaddr_in _localAddr;
    struct sockaddr_in _remoteAddr;
};

#endif