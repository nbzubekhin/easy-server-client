#include "../src/tcp_udp_srv_cli.h"
#include "unit_tests_common.h"
#include <exception>

TEST(TcpUdpServerClient, CreationTcpUdpServerClientobject) {
    try {
        TCP_UDP_SRV_CLI tcp_server;
    } catch (std::exception &e) {
        FAIL() << "An error occured while creating TCP_UDP_SRV_CLI object: "
               << e.what();
    }
}
