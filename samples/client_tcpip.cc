/// Copyright (c) 2020 by Nestor B. Zubekhin.
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
/// C++ program contains implementation of easy TCP/UDP client.

#include <getopt.h>

#include <cstdlib>
#include <exception>
#include <iostream>

#include "tcp_udp_srv_cli.h"

using std::cerr;
using std::cout;
using std::endl;

namespace {
char *         app_name = NULL;
constexpr char MESSAGE_OPTIONS_HELP[] =
    "  Options:\n"
    "   -h --help                            Print this help\n"
    "   -t --type-protocol   <tcp|udp>       Assign of the used protocol (TCP or UDP)\n"
    "   -n --number-of-port  <integer>       Assign of the used number of port\n"
    "   -a --ip-address      <IPv4 address>  Assign of IP address for server screen (by default is: 127.0.0.1)\n\n";

inline void print_help() {
    cout << "Usage: " << app_name << " [OPTIONS]" << endl;
    cout << MESSAGE_OPTIONS_HELP;
    exit(EXIT_FAILURE);
}
}  // namespace

int main(int argc, char *argv[]) {
    int c;
    app_name = argv[0];

    for (;;) {
        int                  option_index   = 0;
        static struct option long_options[] = {{"type-protocol", required_argument, 0, 't'},
                                               {"number-of-port", required_argument, 0, 'n'},
                                               {"ip-address", required_argument, 0, 'a'},
                                               {"help", no_argument, 0, 'h'},
                                               {0, 0, 0, 0}};

        c = getopt_long(argc, argv, "ha:n:t:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'a':
                cout << "option 'ip-address' with value " << optarg << endl;
                break;

            case 't':
                cout << "option 'type-protocol' with value " << optarg << endl;
                break;

            case 'n':
                cout << "option 'number-of-port' with value " << optarg << endl;
                break;

            case 'h':
                (void)print_help();
                break;

            default:
                cerr << "?? getopt returned not defined character code" << endl;
                (void)print_help();
        }
    }

    try {
        TCP_UDP_SRV_CLI tcp_client;
    } catch (std::exception &e) {
        std::cerr << "An error occured while creating TCP_UDP_SRV_CLI object: " << e.what();
    }

    return EXIT_SUCCESS;
}
