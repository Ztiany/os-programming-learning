/*
 ============================================================================
 
 Author      : Ztiany
 Description : TCP Server Factory.

 ============================================================================
 */

#ifndef GEEKTIME_TCP_SERVER_H
#define GEEKTIME_TCP_SERVER_H

/** 创建一个 TCP Server，并接收一个 TCP Client，返回 Client 的 fd。*/
int tcp_server_accept_one(int port);

#endif //GEEKTIME_TCP_SERVER_H
