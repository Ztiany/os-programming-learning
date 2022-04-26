/*
 ============================================================================
 
 Author      : Ztiany
 Description : TCP Client Factory.

 ============================================================================
 */

#ifndef GEEKTIME_TCP_CLIENT_H
#define GEEKTIME_TCP_CLIENT_H

/**创建一个 TCP 客户端，并连接到指定的地址。*/
int tcp_client(const char *ip, int port);

#endif //GEEKTIME_TCP_CLIENT_H
