#pragma once

/// MIT License
/// Copyright (c) 2020
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
/// Description:
/// This file contains implementation of easy TCP/UDP server and client.

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream> /// delete from here
#include <cstring>
#include <functional>
#include <thread>
#include <vector>

#include "common.h"

#define MAX_PACKET_SIZE 4096

struct pipe_ret_t {
  bool success;
  std::string msg;
  pipe_ret_t() {
    success = false;
    msg = "";
  }
};

class Client {
 private:
  int m_sockfd;
  std::string m_ip;
  std::string m_errorMsg;
  bool m_isConnected;
  std::thread *m_threadHandler;

 public:
  Client();
  ~Client();
  bool operator==(const Client &);

  void setFileDescriptor(int);
  int getFileDescriptor() const;

  void setIp(const std::string &);
  std::string getIp() const;

  void setErrorMessage(const std::string &);
  std::string getInfoMessage() const;

  void setConnected();
  void setDisconnected();
  bool isConnected();

  void setThreadHandler(std::function<void(void)> func);
};

typedef void(incoming_packet_func)(const char *msg, size_t size);
typedef incoming_packet_func *incoming_packet_func_t;
typedef void(disconnected_func)(const pipe_ret_t &ret);
typedef disconnected_func *disconnected_func_t;

struct client_observer_t {
  std::string wantedIp;
  incoming_packet_func_t incoming_packet_func;
  disconnected_func_t disconnected_func;

  client_observer_t() {
    wantedIp = "";
    incoming_packet_func = NULL;
    disconnected_func = NULL;
  }
};

typedef void(incoming_packet_func_srv)(const Client &client, const char *msg,
				       size_t size);
typedef incoming_packet_func_srv *incoming_packet_func_srv_t;
typedef void(disconnected_func_srv)(const Client &client);
typedef disconnected_func_srv *disconnected_func_srv_t;

struct server_observer_t {
  std::string wantedIp;
  incoming_packet_func_srv_t incoming_packet_func;
  disconnected_func_srv_t disconnected_func;

  server_observer_t() {
    wantedIp = "";
    incoming_packet_func = NULL;
    disconnected_func = NULL;
  }
};

class TcpClient {
 private:
  int m_sockfd = 0;
  bool stop = false;
  struct sockaddr_in m_server;
  std::vector<client_observer_t> m_subscibers;
  std::thread *m_receiveTask = nullptr;

  void publishServerMsg(const char *msg, size_t msgSize);
  void publishServerDisconnected(const pipe_ret_t &ret);
  void ReceiveTask();
  void terminateReceiveThread();

 public:
  ~TcpClient();
  pipe_ret_t connectTo(const std::string &address, int port);
  pipe_ret_t sendMsg(const char *msg, size_t size);

  void subscribe(const client_observer_t &observer);
  void unsubscribeAll();
  void publish(const char *msg, size_t msgSize);

  pipe_ret_t finish();
};

class TcpServer {
 private:
  int m_sockfd;
  struct sockaddr_in m_serverAddress;
  struct sockaddr_in m_clientAddress;
  fd_set m_fds;
  std::vector<Client> m_clients;
  std::vector<server_observer_t> m_subscibers;
  std::thread *threadHandle;

  void publishClientMsg(const Client &client, const char *msg, size_t msgSize);
  void publishClientDisconnected(const Client &client);
  void receiveTask(/*void * context*/);

 public:
  pipe_ret_t start(int port);
  Client acceptClient(uint timeout);
  bool deleteClient(Client &client);
  void subscribe(const server_observer_t &observer);
  void unsubscribeAll();
  pipe_ret_t sendToAllClients(const char *msg, size_t size);
  pipe_ret_t sendToClient(const Client &client, const char *msg, size_t size);
  pipe_ret_t finish();
  void printClients();
};

//////////////////////////////////////////////////////
class TCP_UDP_SRV_CLI {
 public:
  TCP_UDP_SRV_CLI();
  ~TCP_UDP_SRV_CLI();

 private:
  bool m_running;
};
