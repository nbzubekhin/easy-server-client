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

#include "tcp_udp_srv_cli.h"

Client::Client()
    : m_sockfd(0),
      m_ip(""),
      m_errorMsg(""),
      m_isConnected(false),
      m_threadHandler(nullptr) {}

Client::~Client() {
  if (m_threadHandler != nullptr) {
    m_threadHandler->detach();
    delete m_threadHandler;
    m_threadHandler = nullptr;
  }
}

bool Client::operator==(const Client &other) {
  if ((this->m_sockfd == other.m_sockfd) && (this->m_ip == other.m_ip)) {
    return true;
  }
  return false;
}

void Client::setFileDescriptor(int sockfd) { m_sockfd = sockfd; }
int Client::getFileDescriptor() const { return m_sockfd; }

void Client::setIp(const std::string &ip) { m_ip = ip; }
std::string Client::getIp() const { return m_ip; }

void Client::setErrorMessage(const std::string &msg) { m_errorMsg = msg; }
std::string Client::getInfoMessage() const { return m_errorMsg; }

void Client::setConnected() { m_isConnected = true; }

void Client::setDisconnected() { m_isConnected = false; }
bool Client::isConnected() { return m_isConnected; }

void Client::setThreadHandler(std::function<void(void)> func) {
  m_threadHandler = new std::thread(func);
}

pipe_ret_t TcpClient::connectTo(const std::string &address, int port) {
  m_sockfd = 0;
  pipe_ret_t ret;

  m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (m_sockfd == -1) {	 // socket failed
    ret.success = false;
    ret.msg = strerror(errno);
    return ret;
  }

  int inetSuccess = inet_aton(address.c_str(), &m_server.sin_addr);

  if (!inetSuccess) {  // inet_addr failed to parse address
    // if hostname is not in IP strings and dots format, try resolve it
    struct hostent *host;
    struct in_addr **addrList;
    if ((host = gethostbyname(address.c_str())) == NULL) {
      ret.success = false;
      ret.msg = "Failed to resolve hostname";
      return ret;
    }
    addrList = (struct in_addr **)host->h_addr_list;
    m_server.sin_addr = *addrList[0];
  }
  m_server.sin_family = AF_INET;
  m_server.sin_port = htons(port);

  int connectRet =
      connect(m_sockfd, (struct sockaddr *)&m_server, sizeof(m_server));
  if (connectRet == -1) {
    ret.success = false;
    ret.msg = strerror(errno);
    return ret;
  }
  m_receiveTask = new std::thread(&TcpClient::ReceiveTask, this);
  ret.success = true;
  return ret;
}

pipe_ret_t TcpClient::sendMsg(const char *msg, size_t size) {
  pipe_ret_t ret;
  int numBytesSent = send(m_sockfd, msg, size, 0);
  if (numBytesSent < 0) {  // send failed
    ret.success = false;
    ret.msg = strerror(errno);
    return ret;
  }
  if ((uint)numBytesSent < size) {  // not all bytes were sent
    ret.success = false;
    char msg[100];
    sprintf(msg, "Only %d bytes out of %lu was sent to client", numBytesSent,
	    size);
    ret.msg = msg;
    return ret;
  }
  ret.success = true;
  return ret;
}

void TcpClient::subscribe(const client_observer_t &observer) {
  m_subscibers.push_back(observer);
}

void TcpClient::unsubscribeAll() { m_subscibers.clear(); }

/*
 * Publish incoming client message to observer.
 * Observers get only messages that originated
 * from clients with IP address identical to
 * the specific observer requested IP
 */
void TcpClient::publishServerMsg(const char *msg, size_t msgSize) {
  for (uint i = 0; i < m_subscibers.size(); i++) {
    if (m_subscibers[i].incoming_packet_func != NULL) {
      (*m_subscibers[i].incoming_packet_func)(msg, msgSize);
    }
  }
}

/*
 * Publish client disconnection to observer.
 * Observers get only notify about clients
 * with IP address identical to the specific
 * observer requested IP
 */
void TcpClient::publishServerDisconnected(const pipe_ret_t &ret) {
  for (uint i = 0; i < m_subscibers.size(); i++) {
    if (m_subscibers[i].disconnected_func != NULL) {
      (*m_subscibers[i].disconnected_func)(ret);
    }
  }
}

/*
 * Receive server packets, and notify user
 */
void TcpClient::ReceiveTask() {
  while (!stop) {
    char msg[MAX_PACKET_SIZE];
    int numOfBytesReceived = recv(m_sockfd, msg, MAX_PACKET_SIZE, 0);
    if (numOfBytesReceived < 1) {
      pipe_ret_t ret;
      ret.success = false;
      stop = true;
      if (numOfBytesReceived == 0) {  // server closed connection
	ret.msg = "Server closed connection";
      } else {
	ret.msg = strerror(errno);
      }
      publishServerDisconnected(ret);
      finish();
      break;
    } else {
      publishServerMsg(msg, numOfBytesReceived);
    }
  }
}

pipe_ret_t TcpClient::finish() {
  stop = true;
  terminateReceiveThread();
  pipe_ret_t ret;
  if (close(m_sockfd) == -1) {	// close failed
    ret.success = false;
    ret.msg = strerror(errno);
    return ret;
  }
  ret.success = true;
  return ret;
}

void TcpClient::terminateReceiveThread() {
  if (m_receiveTask != nullptr) {
    m_receiveTask->detach();
    delete m_receiveTask;
    m_receiveTask = nullptr;
  }
}

TcpClient::~TcpClient() { terminateReceiveThread(); }

void TcpServer::subscribe(const server_observer_t &observer) {
    m_subscibers.push_back(observer);
}

void TcpServer::unsubscribeAll() {
    m_subscibers.clear();
}

void TcpServer::printClients() {
    for (uint i = 0; i < m_clients.size(); i++) {
        std::string connected = m_clients[i].isConnected() ? "True" : "False";
        std::cout << "-----------------\n" <<
                  "IP address: " << m_clients[i].getIp() << std::endl <<
                  "Connected?: " << connected << std::endl <<
                  "Socket FD: " << m_clients[i].getFileDescriptor() << std::endl <<
                  "Message: " << m_clients[i].getInfoMessage().c_str() << std::endl;
    }
}

///
/// Receive client packets, and notify user
///
void TcpServer::receiveTask(/*TcpServer *context*/) {

    Client * client = &m_clients.back();

    while(client->isConnected()) {
        char msg[MAX_PACKET_SIZE];
        int numOfBytesReceived = recv(client->getFileDescriptor(), msg, MAX_PACKET_SIZE, 0);
        if(numOfBytesReceived < 1) {
            client->setDisconnected();
            if (numOfBytesReceived == 0) { //client closed connection
                client->setErrorMessage("Client closed connection");
                //printf("client closed");
            } else {
                client->setErrorMessage(strerror(errno));
            }
            close(client->getFileDescriptor());
            publishClientDisconnected(*client);
            deleteClient(*client);
            break;
        } else {
            publishClientMsg(*client, msg, numOfBytesReceived);
        }
    }
}

///
/// Erase client from clients vector.
/// If client isn't in the vector, return false. Return
/// true if it is.
///
bool TcpServer::deleteClient(Client & client) {
    int clientIndex = -1;
    for (uint i=0; i<m_clients.size(); i++) {
        if (m_clients[i] == client) {
            clientIndex = i;
            break;
        }
    }
    if (clientIndex > -1) {
        m_clients.erase(m_clients.begin() + clientIndex);
        return true;
    }
    return false;
}

///
/// Publish incoming client message to observer.
/// Observers get only messages that originated
/// from clients with IP address identical to
/// the specific observer requested IP
///
void TcpServer::publishClientMsg(const Client & client, const char * msg, size_t msgSize) {
    for (uint i=0; i<m_subscibers.size(); i++) {
        if (m_subscibers[i].wantedIp == client.getIp() || m_subscibers[i].wantedIp.empty()) {
            if (m_subscibers[i].incoming_packet_func != NULL) {
                (*m_subscibers[i].incoming_packet_func)(client, msg, msgSize);
            }
        }
    }
}

///
/// Publish client disconnection to observer.
/// Observers get only notify about clients
/// with IP address identical to the specific
/// observer requested IP
///
void TcpServer::publishClientDisconnected(const Client & client) {
    for (uint i=0; i<m_subscibers.size(); i++) {
        if (m_subscibers[i].wantedIp == client.getIp()) {
            if (m_subscibers[i].disconnected_func != NULL) {
                (*m_subscibers[i].disconnected_func)(client);
            }
        }
    }
}

///
/// Bind port and start listening
/// Return tcp_ret_t
///
pipe_ret_t TcpServer::start(int port) {
    m_sockfd = 0;
    m_clients.reserve(10);
    m_subscibers.reserve(10);
    pipe_ret_t ret;

    m_sockfd = socket(AF_INET,SOCK_STREAM,0);
    if (m_sockfd == -1) { //socket failed
        ret.success = false;
        ret.msg = strerror(errno);
        return ret;
    }
    /// set socket for reuse (otherwise might have to wait 4 minutes every time socket is closed)
    int option = 1;
    setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    memset(&m_serverAddress, 0, sizeof(m_serverAddress));
    m_serverAddress.sin_family = AF_INET;
    m_serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    m_serverAddress.sin_port = htons(port);

    int bindSuccess = bind(m_sockfd, (struct sockaddr *)&m_serverAddress, sizeof(m_serverAddress));
    if (bindSuccess == -1) { // bind failed
        ret.success = false;
        ret.msg = strerror(errno);
        return ret;
    }
    const int clientsQueueSize = 5;
    int listenSuccess = listen(m_sockfd, clientsQueueSize);
    if (listenSuccess == -1) { // listen failed
        ret.success = false;
        ret.msg = strerror(errno);
        return ret;
    }
    ret.success = true;
    return ret;
}

///
/// Accept and handle new client socket. To handle multiple clients, user must
/// call this function in a loop to enable the acceptance of more than one.
/// If timeout argument equal 0, this function is executed in blocking mode.
/// If timeout argument is > 0 then this function is executed in non-blocking
/// mode (async) and will quit after timeout seconds if no client tried to connect.
/// Return accepted client
///
Client TcpServer::acceptClient(uint timeout) {
    socklen_t sosize  = sizeof(m_clientAddress);
    Client newClient;

    if (timeout > 0) {
        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        FD_ZERO(&m_fds);
        FD_SET(m_sockfd, &m_fds);
        int selectRet = select(m_sockfd + 1, &m_fds, NULL, NULL, &tv);
        if (selectRet == -1) { // select failed
            newClient.setErrorMessage(strerror(errno));
            return newClient;
        } else if (selectRet == 0) { // timeout
            newClient.setErrorMessage("Timeout waiting for client");
            return newClient;
        } else if (!FD_ISSET(m_sockfd, &m_fds)) { // no new client
            newClient.setErrorMessage("File descriptor is not set");
            return newClient;
        }
    }

    int file_descriptor = accept(m_sockfd, (struct sockaddr*)&m_clientAddress, &sosize);
    if (file_descriptor == -1) { // accept failed
        newClient.setErrorMessage(strerror(errno));
        return newClient;
    }

    newClient.setFileDescriptor(file_descriptor);
    newClient.setConnected();
    newClient.setIp(inet_ntoa(m_clientAddress.sin_addr));
    m_clients.push_back(newClient);
    m_clients.back().setThreadHandler(std::bind(&TcpServer::receiveTask, this));

    return newClient;
}

///
/// Send message to all connected clients.
/// Return true if message was sent successfully to all clients
///
pipe_ret_t TcpServer::sendToAllClients(const char * msg, size_t size) {
    pipe_ret_t ret;
    for (uint i=0; i<m_clients.size(); i++) {
        ret = sendToClient(m_clients[i], msg, size);
        if (!ret.success) {
            return ret;
        }
    }
    ret.success = true;
    return ret;
}

///
/// Send message to specific client (determined by client IP address).
/// Return true if message was sent successfully
///
pipe_ret_t TcpServer::sendToClient(const Client & client, const char * msg, size_t size){
    pipe_ret_t ret;
    int numBytesSent = send(client.getFileDescriptor(), (char *)msg, size, 0);
    if (numBytesSent < 0) { // send failed
        ret.success = false;
        ret.msg = strerror(errno);
        return ret;
    }
    if ((uint)numBytesSent < size) { // not all bytes were sent
        ret.success = false;
        char msg[100];
        sprintf(msg, "Only %d bytes out of %lu was sent to client", numBytesSent, size);
        ret.msg = msg;
        return ret;
    }
    ret.success = true;
    return ret;
}

///
/// Close server and clients resources.
/// Return true is success, false otherwise
///
pipe_ret_t TcpServer::finish() {
    pipe_ret_t ret;
    for (uint i=0; i<m_clients.size(); i++) {
        m_clients[i].setDisconnected();
        if (close(m_clients[i].getFileDescriptor()) == -1) { // close failed
            ret.success = false;
            ret.msg = strerror(errno);
            return ret;
        }
    }
    if (close(m_sockfd) == -1) { // close failed
        ret.success = false;
        ret.msg = strerror(errno);
        return ret;
    }
    m_clients.clear();
    ret.success = true;
    return ret;
}

TCP_UDP_SRV_CLI::TCP_UDP_SRV_CLI() { m_running = true; }
TCP_UDP_SRV_CLI::~TCP_UDP_SRV_CLI() { m_running = false; }
