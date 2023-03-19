// #include <iostream>
// #include <thread>
// #include <functional>
// #include "kcp4udp.h"

// int main(int argc, char *argv[])
// {
//     Kcp4Udp kcp4udp;
//     kcp4udp.KcpInit(0x16, "127.0.0.1", 9000, true);
//     // std::thread kcpUpdateThd(std::mem_fn(&Kcp4Udp::KcpCoreLoop), kcp4udp);
//     kcp4udp.KcpCoreLoop();
//     return 0;
// }