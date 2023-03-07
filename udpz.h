#ifndef UDPZ_H
#define UDPZ_H

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "unistd.h"
#include "sys/types.h"
#include "sys/socket.h"
#include <sys/time.h>
#include "netinet/in.h"
#include "arpa/inet.h"

#define MSG_DAT_PIECE_LEN 6388

struct MsgPiece
{
    unsigned short conv;
    unsigned short totalSize;
    unsigned short totalPiece;
    unsigned short currPiece;
    unsigned int timeSlab;
    char data[MSG_DAT_PIECE_LEN];
};

const unsigned int MSG_SUB_PKG_LEN = sizeof(MsgPiece);

class UdpZ
{
public:
    ~UdpZ()
    {
        if (this->_sockfd != -1) {
            close(this->_sockfd);
        }
        if (this->_recvBuf)
        {
            free(this->_recvBuf);
        }
    }

    int InitServer(const std::string &hostIP, const int hostPort, int timeout)
    {
        this->_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (this->_sockfd < 0) {
            std::cout << "Create sockfd failed." << std::endl;
            return -1;
        }

        memset(&this->_localAddr, 0, sizeof(this->_localAddr));
        this->_localAddr.sin_family = AF_INET;
        this->_localAddr.sin_addr.s_addr = inet_addr(hostIP.c_str());
        this->_localAddr.sin_port = htons(hostPort);

        if (bind(this->_sockfd, (const struct sockaddr*)&this->_localAddr, sizeof(this->_localAddr)) < 0) {
            std::cout << "Bind socket failed." << std::endl;
            return -1;
        }
        this->_recvTimeout = timeout;
        return 0;
    }

    int InitClient(const std::string &servIP, const int servPort)
    {
        this->_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (this->_sockfd < 0) {
            std::cout << "Create sockfd failed." << std::endl;
            return -1;
        }
        memset(&this->_remoteAddr, 0, sizeof(this->_remoteAddr));
        this->_remoteAddr.sin_family = AF_INET;
        this->_remoteAddr.sin_addr.s_addr = inet_addr(servIP.c_str());
        this->_remoteAddr.sin_port = htons(servPort);
        return 0;
    }

    int SendMsg(const char *msgData, int msgLen)
    {
        int totalSize = msgLen;
        int totalPiece = msgLen / MSG_DAT_PIECE_LEN;
        if (msgLen % MSG_DAT_PIECE_LEN) {
            totalPiece += 1;
        }

        int ret = -1;
        MsgPiece msgPiece;
        msgPiece.conv = 0x01;
        msgPiece.totalSize = msgLen;
        msgPiece.totalPiece = totalPiece;
        const char *pkgBuf = reinterpret_cast<const char*>(&msgPiece);
        
        int i;
        for (i = 0; i < totalPiece - 1; i++)
        {
            msgPiece.currPiece = i;
            msgPiece.timeSlab = TimeSlab();
            memset(msgPiece.data, 0, MSG_DAT_PIECE_LEN);
            memcpy(msgPiece.data, msgData + i * MSG_DAT_PIECE_LEN, MSG_DAT_PIECE_LEN);

            ret = sendto(this->_sockfd, pkgBuf, MSG_SUB_PKG_LEN, 0, (const sockaddr*)&this->_remoteAddr,
                         sizeof(this->_remoteAddr));
            if (ret < 0) {
                return -1;
            }
        }

        int tailLen = msgLen - i * MSG_DAT_PIECE_LEN;
        memset(msgPiece.data, 0, MSG_DAT_PIECE_LEN);
        memcpy(msgPiece.data, msgData + (totalPiece - 1) * MSG_DAT_PIECE_LEN, tailLen);
        msgPiece.currPiece = totalPiece - 1;
        msgPiece.timeSlab = TimeSlab();

        ret = sendto(this->_sockfd, pkgBuf, MSG_SUB_PKG_LEN, 0, (const sockaddr*)&this->_remoteAddr,
                     sizeof(this->_remoteAddr));
        if (ret < 0) {
            return -1;
        }
        return msgLen;
    }

    int RecvMsg(char *msgData, int &msgLen)
    {
        int ret = -1;
        socklen_t remoteAddrLen = sizeof(this->_remoteAddr);
        memset(&this->_remoteAddr, 0, remoteAddrLen);

        char buf[MSG_SUB_PKG_LEN] = {0};
        while (ret < 0) {
            ret = recvfrom(this->_sockfd, buf, sizeof(buf), 0,
                           (struct sockaddr*)&this->_remoteAddr, &remoteAddrLen);
        }
        std::cout << "Receive first data piece package." << std::endl;

        MsgPiece *msgPiece = reinterpret_cast<MsgPiece *>(buf);
        std::vector<MsgPiece> msgPieceList(msgPiece->totalPiece);

        msgPieceList[msgPiece->currPiece] = *msgPiece;
        unsigned short conv = msgPiece->conv;
        unsigned int lastTimeSlab = msgPiece->timeSlab;
        unsigned int newTimeSlab = msgPiece->timeSlab;
        msgLen = msgPiece->totalPiece;
        std::cout << "MSG length: [" << msgLen << "] Piece amounts: ["<< msgPiece->totalPiece << "]" << std::endl;

        int counter = 1;
        while (counter < msgPieceList.size()) {
            memset(buf, 0, MSG_SUB_PKG_LEN);
            ret = recvfrom(this->_sockfd, buf, sizeof(buf), 0,
                           (struct sockaddr*)&this->_remoteAddr, &remoteAddrLen);
            
            if (ret > 0) {
                msgPiece = reinterpret_cast<MsgPiece *>(buf);
                if (msgPiece->conv == conv)
                {
                    msgPieceList[msgPiece->currPiece] = *msgPiece;
                    counter++;
                    lastTimeSlab = msgPiece->timeSlab;
                }
            }
            newTimeSlab = TimeSlab();
            if (lastTimeSlab - newTimeSlab > this->_recvTimeout) {
                return -1;
            }
        }

        this->_recvBuf = (char *)malloc(msgLen);
        if (this->_recvBuf == nullptr) {
            return -1;
        }

        int i;
        for (i = 0; i < msgPieceList.size() - 1; i++) {
            memcpy(this->_recvBuf + i * MSG_DAT_PIECE_LEN, msgPieceList[i].data, MSG_DAT_PIECE_LEN);
        }
        int tailLen = msgLen - i * MSG_DAT_PIECE_LEN;
        memcpy(this->_recvBuf + i * MSG_DAT_PIECE_LEN, msgPieceList[i].data, tailLen);

        return msgLen;
    }
private:
    inline void Timeofday(long *sec, long *usec)
    {
        #if defined(__unix)
        struct timeval time;
        gettimeofday(&time, NULL);
        if (sec) *sec = time.tv_sec;
        if (usec) *usec = time.tv_usec;
        #else
        static long mode = 0;
        static long addSec = 0;
        BOOL retval;
        static unsigned long freq = 1;
        unsigned long qpc;
        if (mode == 0) {
            retval = QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
            freq = (freq == 0)? 1 : freq;
            retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
            addSec = (long)time(NULL);
            addSec = addSec - (long)((qpc / freq) & 0x7fffffff);
            mode = 1;
        }
        retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
        retval = retval * 2;
        if (sec) *sec = (long)(qpc / freq) + addSec;
        if (usec) *usec = (long)((qpc % freq) * 1000000 / freq);
        #endif
    }

    unsigned int TimeSlab()
    {
        long s, u;
        unsigned int long value;
        Timeofday(&s, &u);
        value = ((unsigned int long)s) * 1000 + (u / 1000);
        return ( unsigned int)(value & 0xfffffffful);
    }

private:
    int _sockfd = -1;
    struct sockaddr_in _localAddr;
    struct sockaddr_in _remoteAddr;
    int _recvTimeout;
    char *_recvBuf = nullptr;
};

#endif