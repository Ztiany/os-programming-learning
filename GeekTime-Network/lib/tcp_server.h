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

/** 创建一个 TCP Server，并开始监听，返回 Server 的 fd。*/
int tcp_server_listen(int port);

/** 创建一个非阻塞 TCP Server，并开始监听，返回 Server 的 fd。*/
int tcp_server_listen_non_blocking(int port);

#endif //GEEKTIME_TCP_SERVER_H
