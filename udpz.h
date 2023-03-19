#ifndef _UdpTransceiver_H
#define _UdpTransceiver_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <cstring>

#include "unistd.h"
#include "sys/types.h"
#include "sys/socket.h" 
#include "netinet/in.h"
#include "arpa/inet.h"

#define MSG_DATA_PIECE_LEN 6388

using namespace std::chrono_literals;

struct MsgPiece
{
    uint64_t conv;
    uint64_t timestamp;
    uint32_t totalSize;
    uint16_t totalPiece;
    uint16_t currPiece;
    uint8_t data[MSG_DATA_PIECE_LEN];
};

const unsigned int MSG_SUB_PKG_LEN = sizeof(MsgPiece);

template <typename DataType>
class UdpTransceiver
{
private:
    int _sockfd = -1;
    struct sockaddr_in _localAddr;
    struct sockaddr_in _remoteAddr;
    int _recvTimeout;
    char *_recvBuf = nullptr;
public:
    ~UdpTransceiver();
    int InitServer(const std::string &localIP, const int &localPort, uint64_t timeout = 50);
    int InitClient(const std::string &remoteIP, const int &remotePort);
    int SendMsg(const DataType &userData, int size);
    int RecvMsg(DataType &userData, int &size);
private:
    uint64_t Timestamp();
};

// public:

template <typename DataType>
UdpTransceiver<DataType>::~UdpTransceiver()
{
    if (this->_sockfd != -1) {
        close(this->_sockfd);
    }
}

static bool CheckIPAndPort(const std::string &ip, const int port)
{
    if (ip.empty()) return false;
    std::vector<std::string> ipSegments;
    int i ,j;
    i = j = 0;
    while (i < ip.size()) {
        j = i;
        while (j < ip.size() && ip[j] != '.') {
            if (ip[j] < '0' || ip[j] > '9') return false;
            j++;
        }
        ipSegments.emplace_back(ip.substr(i, j - i));
        i = j + 1;
    }
    std::stringstream ss;
    int value = 0;
    for (const std::string &item : ipSegments) {
        if (item[0] == '0' && item.size() != 1) return false;
        ss << item; ss >> value;
        if (value < 0 || value > 255) return false;
    }
    if (port < 0 || port > 65535) return false;
    return true;
}

template <typename DataType>
int UdpTransceiver<DataType>::InitServer(const std::string &localIP, const int &localPort, uint64_t timeout)
{
    if (CheckIPAndPort(localIP, localPort) == false) {
        std::cout << "Failed to initialize server." << std::endl;
        return -1;
    }
    this->_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (this->_sockfd < 0) {
        std::cout << "Create sockfd failed." << std::endl;
        return -1;
    }

    memset(&this->_localAddr, 0, sizeof(this->_localAddr));
    this->_localAddr.sin_family = AF_INET;
    this->_localAddr.sin_addr.s_addr = inet_addr(localIP.c_str());
    this->_localAddr.sin_port = htons(localPort);

    if (bind(this->_sockfd, (const struct sockaddr*)&this->_localAddr, sizeof(this->_localAddr)) < 0) {
        std::cout << "Bind socket failed." << std::endl;
        return -1;
    }
    this->_recvTimeout = timeout;
    return 0;
}

template <typename DataType>
int UdpTransceiver<DataType>::InitClient(const std::string &remoteIP, const int &remotePort)
{
    if (CheckIPAndPort(remoteIP, remotePort) == false) {
        std::cout << "Failed to initialize client." << std::endl;
        return -1;
    }
    this->_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (this->_sockfd < 0) {
        std::cout << "Create sockfd failed." << std::endl;
        return -1;
    }
    
    memset(&this->_remoteAddr, 0, sizeof(this->_remoteAddr));
    this->_remoteAddr.sin_family = AF_INET;
    this->_remoteAddr.sin_addr.s_addr = inet_addr(remoteIP.c_str());
    this->_remoteAddr.sin_port = htons(remotePort);
    if (connect(this->_sockfd, (struct sockaddr*)&this->_remoteAddr, sizeof(this->_remoteAddr)) < 0) {
        std::cout << "Failed to connect server." << std::endl;
        return -1;
    }
    return 0;   
}

template <typename DataType>
int UdpTransceiver<DataType>::SendMsg(const DataType &userData, int size)
{
    if (size <= 0) {
        std::cout << "Failed to send empty data." << std::endl;
    }
    MsgPiece msgPiece;
    msgPiece.totalSize = size;
    msgPiece.totalPiece = msgPiece.totalSize / MSG_DATA_PIECE_LEN;
    int tailSize = msgPiece.totalSize % MSG_DATA_PIECE_LEN;
    if (tailSize) {
        msgPiece.totalPiece++;
    }

    msgPiece.conv = 0x01;
    int ret = -1;
    int i, sizeCount;
    sizeCount = 0;
    const uint8_t *data = reinterpret_cast<const uint8_t *>(&userData);
    for (i = 0; i < msgPiece.totalPiece - 1; i++)
    {
        msgPiece.currPiece = i;
        msgPiece.timestamp = this->Timestamp();
        memset(msgPiece.data, 0, MSG_DATA_PIECE_LEN);
        memcpy(msgPiece.data, &data + i * MSG_DATA_PIECE_LEN, MSG_DATA_PIECE_LEN);

        ret = send(this->_sockfd, &msgPiece, MSG_SUB_PKG_LEN, 0);
        if (ret != MSG_SUB_PKG_LEN) {
            std::cout << "Failed to send data." << std::endl;
            return -1;
        }
        sizeCount += MSG_DATA_PIECE_LEN;
    }

    memset(msgPiece.data, 0, MSG_DATA_PIECE_LEN);
    memcpy(msgPiece.data, data + i * MSG_DATA_PIECE_LEN, tailSize);
    msgPiece.currPiece = i;
    msgPiece.timestamp = this->Timestamp();

    ret = send(this->_sockfd, &msgPiece, MSG_SUB_PKG_LEN, 0);
    if (ret != MSG_SUB_PKG_LEN) {
        std::cout << "Failed to send data." << std::endl;
        return -1;
    }
    sizeCount += tailSize;
    return sizeCount;
}

template <typename DataType>
int UdpTransceiver<DataType>::RecvMsg(DataType &userData, int &size)
{    
    MsgPiece msgPiece;
    int ret = -1;
    while (ret < 0) {
        ret = recv(this->_sockfd, &msgPiece, MSG_SUB_PKG_LEN, 0);
    }
    std::cout << "Receive first data piece package." << std::endl;

    int tailSize = msgPiece.totalSize % MSG_DATA_PIECE_LEN;
    uint8_t *data = new uint8_t(msgPiece.totalSize);
    
    if (msgPiece.totalSize == tailSize) {
        memcpy(data, msgPiece.data, tailSize);
        memcpy(&userData, data, tailSize);
        std::cout << "data: " << data << std::endl;
        return msgPiece.totalSize;
    }
    memcpy(data, msgPiece.data, MSG_DATA_PIECE_LEN);

    uint64_t conv = msgPiece.conv;
    uint64_t lastTimestamp = msgPiece.timestamp;
    uint64_t newTimestamp = msgPiece.timestamp;
    std::cout << "MSG length: [" << msgPiece.totalSize << "] Piece amounts: ["<< msgPiece.totalPiece << "]" << std::endl;

    int counter = 1;
    int offset = 0;
    while (counter <  msgPiece.totalPiece) {
        ret = recv(this->_sockfd, &msgPiece, MSG_SUB_PKG_LEN, 0);
        if (ret == MSG_SUB_PKG_LEN && msgPiece.conv == conv) {
            offset = msgPiece.currPiece * MSG_DATA_PIECE_LEN;
            if (msgPiece.currPiece == msgPiece.totalPiece - 1) {
                memcpy(data + offset, msgPiece.data, tailSize);
                break;
            }
            memcpy(data + offset, msgPiece.data, MSG_DATA_PIECE_LEN);
            counter++;
            lastTimestamp = msgPiece.timestamp;
            continue;
        }
        newTimestamp = Timestamp();
        if (newTimestamp - lastTimestamp > this->_recvTimeout) {
            return -1;
        }
    }

    size = msgPiece.totalSize;
    memcpy(&userData, (DataType *)data, size);
    std::cout << "data: " << data << std::endl;
    return size;
}

// private:

template <typename DataType>
uint64_t UdpTransceiver<DataType>::Timestamp()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>
           (std::chrono::system_clock().now().time_since_epoch()).count();
}

#endif