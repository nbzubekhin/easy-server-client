#include "../src/tcp_udp_srv_cli.h"
#include "unit_tests_common.h"

TEST(TcpIPServer, CreationTcpIPServerObject) {
    try {
        TcpServer server;
    } catch (std::exception &e) {
        FAIL() << "An error occured while creating TcpServer: "
               << e.what();
    }
}

TEST(TcpIPClient, CreationTcpIPClientObject) {
    try {
        TcpClient client;
    } catch (std::exception &e) {
        FAIL() << "An error occured while creating TcpClient object: "
               << e.what();
    }
}
