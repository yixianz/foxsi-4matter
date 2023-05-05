#include "Subsystem.h"
#include "Ticker.h"
#include <iostream>
#include <queue>
#include <vector>
#include <map>
#include <string>

int main() {
    
    // for gse-based testing; flip these if running on formatter processor
    std::string local_ip = "192.168.1.108";
    std::string ground_ip = "192.168.1.8";

    // number ports for local receiving, remote TCP, and remote UDP.
    unsigned short local_ground_port = 9999;
    unsigned short remote_ground_port = 9999;
    unsigned short remote_subsys_port = 10000;

    // create the Boost io_context to handle async events
    boost::asio::io_context context;

    // durations of each state (currently UNUSED)
    std::map<STATE_ORDER, double> durations{
        {CMD_SEND, 1.0},
        {DATA_REQ, 1.0},
        {DATA_RECV, 1.0},
        {IDLE, 1.0}
    };

    // currently UNUSED (not even sure what I wanted to do with this...)
    std::map<std::string, std::string> flags{
        {"flag0", "no"}
    };

    PepperMill frmtr(
        local_ip,               // IP address of this computer
        ground_ip,              // IP of the remote TCP computer (to listen to)
        ground_ip,              // IP of the remote UDP computer (to send to)
        local_ground_port,      // port number on this computer to listen for TCP on
        remote_ground_port,     // port number on the remote TCP computer
        remote_subsys_port,     // port number on the remote UDP computer (to send to)
        context
    );

    frmtr.recv_tcp_fwd_udp();   // need to give the io_context something to do, else it dies immediately 
    context.run();

    return 0;
}
