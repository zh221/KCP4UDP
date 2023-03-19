// #ifndef KCP4UDP_H
// #define KCP4UDP_H

// #include <iostream>
// #include <string>
// #include <memory>
// #include <thread>
// #include <cstdlib>
// #include <ctime>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/time.h>
// #include <sys/wait.h>

// #include "ikcp.c"
// #include "udpz.h"

// #define KCP_RECVBUF_MAXLEN 65535
// #define KCP_SENDBUF_MAXLEN 65535

// /* get system time */
// static inline void Timeofday(long *sec, long *usec)
// {
// 	#if defined(__unix)
// 	struct timeval time;
// 	gettimeofday(&time, NULL);
// 	if (sec) *sec = time.tv_sec;
// 	if (usec) *usec = time.tv_usec;
// 	#else
// 	static long mode = 0, addsec = 0;
// 	BOOL retval;
// 	static IINT64 freq = 1;
// 	IINT64 qpc;
// 	if (mode == 0) {
// 		retval = QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
// 		freq = (freq == 0)? 1 : freq;
// 		retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
// 		addsec = (long)time(NULL);
// 		addsec = addsec - (long)((qpc / freq) & 0x7fffffff);
// 		mode = 1;
// 	}
// 	retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
// 	retval = retval * 2;
// 	if (sec) *sec = (long)(qpc / freq) + addsec;
// 	if (usec) *usec = (long)((qpc % freq) * 1000000 / freq);
// 	#endif
// }

// static inline IUINT32 Iclock()
// {
//     long s, u;
//     IINT64 value;
//     Timeofday(&s, &u);
//     value = ((IINT64)s) * 1000 + (u / 1000);
//     return (IUINT32)(value & 0xfffffffful);
// }

// static inline void Isleep(unsigned long millisecond)
// {
// 	#ifdef __unix 	/* usleep( time * 1000 ); */
// 	struct timespec ts;
// 	ts.tv_sec = (time_t)(millisecond / 1000);
// 	ts.tv_nsec = (long)((millisecond % 1000) * 1000000);
// 	/*nanosleep(&ts, NULL);*/
// 	usleep((millisecond << 10) - (millisecond << 4) - (millisecond << 3));
// 	#elif defined(_WIN32)
// 	Sleep(millisecond);
// 	#endif
// }

// int OutputPkg(const char *data, int size, ikcpcb *kcpcb, void *user)
// {
//     UdpZ *udpz = static_cast<UdpZ*>(user);
//     if (size > 0) {
//         return udpz->SnedMsg(std::string(data));
//     }
//     return -1;
// }

// class Kcp4Udp
// {
// private:
//     ikcpcb *_kcpcb = nullptr;
//     UdpZ _udpz;
//     char _recvBuf[KCP_RECVBUF_MAXLEN];
//     char _sendBuf[KCP_SENDBUF_MAXLEN];
// public:
//     ~Kcp4Udp()
//     {
//         if (this->_kcpcb) {
//             ikcp_release(this->_kcpcb);
//         }
//     }
//     int KcpInit(uint32_t conv, std::string localIPv4, uint16_t localPort, flag udpServEnabled)
//     {
//         this->_kcpcb = ikcp_create(conv, &this->_udpz);
//         this->_kcpcb->output = OutputPkg;
//         ikcp_wndsize(this->_kcpcb, 128, 128);
//         ikcp_nodelay(this->_kcpcb, 0, 10, 0, 0);
//         this->_kcpcb->rx_minrto = 10;
//         this->_kcpcb->fastresend = 1;

//         if (udpServEnabled) {
//             this->_udpz.InitServer(localIPv4, localPort);
//         }
//         return 0;
//     }
//     int KcpRun()
//     {
//         return 0;
//     }
//     int KcpSend(const std::string destIPv4, const uint16_t destPort, const char *msgBuf, const int msgLen)
//     {
//         ikcp_send(this->_kcpcb, msgBuf, msgLen);
//         return 0;
//     }
//     int KcpRecv(std::string &senderIPv4, const uint16_t &senderPort, char *msgBuf, int &msgLen)
//     {
//         ikcp_recv(this->_kcpcb, msgBuf, msgLen);
//         return 0;
//     }
// // private:
//     void KcpCoreLoop()
//     {
//         uint32_t current = Iclock();
//         uint32_t slap = current + 20;
//         uint32_t index = 0;
//         uint32_t next = 0;
//         int count = 0;
//         int maxRTT = 0;
//         int ret = -1;
//         while (true) {
//             Isleep(1);
//             current = Iclock();
//             ikcp_update(this->_kcpcb, Iclock());

//             for (; current >= slap; slap += 20) {
//                 ((uint32_t*)this->_sendBuf)[0] = index++;
//                 ((uint32_t*)this->_sendBuf)[1] = current;

//                 ikcp_send(this->_kcpcb, this->_sendBuf, 8);
//             }

//             while (true) {
//                 ret = this->_udpz.RecvMsg(this->_recvBuf, KCP_RECVBUF_MAXLEN);
//                 if (ret < 0) break;
//                 ikcp_input(this->_kcpcb, this->_recvBuf, KCP_RECVBUF_MAXLEN);   
//             }
//         }
//     }
// };

// #endif