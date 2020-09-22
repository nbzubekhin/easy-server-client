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
/// C++ program contains implementation of easy TCP/UDP server.

#include <getopt.h>

#include <cstdlib>
#include <exception>
#include <iostream>

#include "tcp_udp_srv_cli.h"

using std::cerr;
using std::cout;
using std::endl;

namespace {
char *app_name = NULL;
constexpr char MESSAGE_OPTIONS_HELP[] =
    "  Options:\n"
    "   -h --help                       Print this help\n"
    "   -t --type-protocol   <tcp|udp>  Assign of the used protocol (TCP or "
    "UDP)\n"
    "   -n --number-of-port  <integer>  Assign of the used number of port\n"
    "   -v --verbose                    Enable logs to output to screen (by "
    "default disabled)\n\n";

inline void print_help() {
  cout << "Usage: " << app_name << " [OPTIONS]" << endl;
  cout << MESSAGE_OPTIONS_HELP;
}

int number_of_port = 9000;
std::string type_protocol = "tcp";
TcpServer server;
server_observer_t observer1, observer2;

void onIncomingMsg1(const Client &client, const char *msg, size_t size) {
  std::string msgStr = msg;
  // print the message content
  std::cout << "Observer1 got client msg: " << msgStr << std::endl;
  // if client sent the string "quit", close server
  // else if client sent "print" print the server clients
  // else just print the client message
  if (msgStr.find("quit") != std::string::npos) {
    std::cout << "Closing server..." << std::endl;
    pipe_ret_t finishRet = server.finish();
    if (finishRet.success) {
      std::cout << "Server closed." << std::endl;
    } else {
      std::cout << "Failed closing server: " << finishRet.msg << std::endl;
    }
  } else if (msgStr.find("print") != std::string::npos) {
    server.printClients();
  } else {
    std::string replyMsg = "server got this msg: " + msgStr;
    server.sendToAllClients(replyMsg.c_str(), replyMsg.length());
  }
}

// observer callback. will be called for every new message received by clients
// with the requested IP address
void onIncomingMsg2(const Client &client, const char *msg, size_t size) {
  std::string msgStr = msg;
  // print client message
  std::cout << "Observer2 got client msg: " << msgStr << std::endl;

  // reply back to client
  std::string replyMsg = "server got this msg: " + msgStr;
  server.sendToClient(client, msg, size);
}

// observer callback. will be called when client disconnects
void onClientDisconnected(const Client &client) {
  std::cout << "Client: " << client.getIp()
	    << " disconnected: " << client.getInfoMessage() << std::endl;
}

}  // namespace

int main(int argc, char *argv[]) {
  int c;
  app_name = argv[0];

  for (;;) {
    int option_index = 0;
    static struct option long_options[] = {
	{"type-protocol", required_argument, 0, 't'},
	{"number-of-port", required_argument, 0, 'n'},
	{"verbose", no_argument, 0, 'v'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}};

    c = getopt_long(argc, argv, "vhn:t:", long_options, &option_index);
    if (c == -1) {
      break;
    }

    switch (c) {
      case 'v':
	cout << "option verbose" << endl;
	break;

      case 't':
	cout << "option 'type-protocol' with value " << optarg << endl;
	type_protocol = optarg;
	break;

      case 'n':
	cout << "option 'number-of-port' with value " << optarg << endl;
	try {
	  number_of_port = std::stoi(optarg);
	} catch (std::invalid_argument const &e) {
	  logger::instance().log("Bad input: std::invalid_argument thrown: " +
				 std::string(e.what()));
	  return EXIT_FAILURE;
	} catch (std::out_of_range const &e) {
	  logger::instance().log(
	      "Integer overflow: std::out_of_range thrown: " +
	      std::string(e.what()));
	  return EXIT_FAILURE;
	}
	break;

      case 'h':
	(void)print_help();
	return EXIT_SUCCESS;

      default:
	cerr << "?? getopt returned not defined character code" << endl;
	(void)print_help();
    }
  }

  /// Start server on defined port
  pipe_ret_t startRet = server.start(number_of_port);
  if (startRet.success) {
    logger::instance().log("Server setup succeeded");
  } else {
    logger::instance().log("Server setup failed: " + startRet.msg);
    return EXIT_FAILURE;
  }

  // configure and register observer1
  observer1.incoming_packet_func = onIncomingMsg1;
  observer1.disconnected_func = onClientDisconnected;
  observer1.wantedIp = "127.0.0.1";
  server.subscribe(observer1);

  // configure and register observer2
  observer2.incoming_packet_func = onIncomingMsg2;
  observer1.disconnected_func = nullptr;  // don't care about disconnection
  observer2.wantedIp = "";		  // use empty string instead to receive
					  // messages from any IP address
  server.subscribe(observer2);

  // receive clients
  for (;;) {
    Client client = server.acceptClient(0);
    if (client.isConnected()) {
      std::cout << "Got client with IP: " << client.getIp() << std::endl;
      server.printClients();
    } else {
      std::cout << "Accepting client failed: " << client.getInfoMessage()
		<< std::endl;
    }
    sleep(1);
  }

  return EXIT_SUCCESS;
}
