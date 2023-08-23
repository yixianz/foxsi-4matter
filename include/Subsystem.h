#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H

#include "AbstractState.h"
#include "UDPInterface.h"
#include "TCPInterface.h"
#include "UARTInterface.h"
#include "RingBufferInterface.h"
#include "Fragmenter.h"
#include "Commanding.h"
#include "Parameters.h"
// #include <boost/asio.hpp>
#include <vector>
#include <map>
#include <queue>

/**
 * @brief Manager for network operations.
 * 
 * This class manages transport-layer services (UDP and TCP) for the Formatter software. The class wraps some basic socket input-output functionality (provided by `boost::asio`) and manages internal buffering and forwarding of received data. Currently, assumes network topology in which one remote TCP endpoint sends messages which are filtered and forwarded to another remote UDP endpoint. 
 * @todo This should be modified in the future (multiple remote UDP endpoints), and instantiated based on foxsi4-commands/systems.json.
 */
class TransportLayerMachine {
    public:
        /**
         * @brief the local machine's UDP socket object.
         */
        boost::asio::ip::udp::socket local_udp_sock;
        /**
         * @brief the local machine's TCP socket object.
         */
        boost::asio::ip::tcp::socket local_tcp_sock;
        /**
         * @brief a remote machine's UDP endpoint.
         */
        boost::asio::ip::udp::endpoint remote_udp_endpoint;
        /**
         * @brief a remote machine's TCP endpoint.
         */
        boost::asio::ip::tcp::endpoint remote_tcp_endpoint;

        /**
         * @brief a rudimentary buffer for data to downlink (send to UDP endpoint).
         * @todo replace with a buffer of structured messages, include sender info and length.
         */
        std::vector<uint8_t> downlink_buff;
        /**
         * @brief a rudimentary buffer for uplinked command data (to send to TCP endpoint).
         * @todo replace with a buffer of structured messages, include target system info and length.
         */
        std::vector<uint8_t> uplink_buff;
        /**
         * @brief a rudimentary buffer for uplinked command data (to send to TCP endpoint).
         * @todo replace with a buffer of structured messages, include target system info and length. Clean up handoff between uplink_buff and command_pipe.
         */
        std::vector<uint8_t> command_pipe;
        /**
         * @deprecated currently unused.
         */
        std::queue<uint8_t> ground_pipe;

        /**
         * @brief instance of `CommandDeck`, storing command and system data used to decode and forward uplinked commands.
         */
        CommandDeck commands;
        
        /**
         * @brief map from `System::hex` codes for each onboard system to `RingBufferInterface` objects for each system.
         * Used to lookup the ring buffer parameters for remote memory access to each system.
         */
        std::unordered_map<uint8_t, RingBufferInterface> ring_buffers;

        /**
         * @brief instance of `Fragmenter` used to slice downlink data stream into appropriated-sized blocks.
         * Downlink interface prescribes a Maximum Transmission Unit (MTU) that limits that total buffer size that can be transmitted as one packet.
         */
        Fragmenter fragmenter;

        /**
         * @deprecated currently unused. And probably unsafe.
         */
        STATE_ORDER active_state;
        /**
         * @deprecated currently unused. And probably unsafe.
         */
        SUBSYSTEM_ORDER active_subsys;

    public:
        /**
         * @brief Default constructor. 
         * Creates an empty `CommandDeck` and assigns `TransportLayerMachine::*socket` and `TransportLayerMachine::*endpoint` to the default values prescribed in `Parameters.h`.
         * 
         * @param context A reference to a `boost::asio::io_context` used to run asynchronous work.
         */
        TransportLayerMachine(boost::asio::io_context& context);
        /**
         * @brief Construct a new Transport Layer Machine from `std::string` IP address and `unsigned short` port number pairs.
         * 
         * @param local_ip The IP address of the local machine.
         * @param remote_tcp_ip The IP address of a remote machine that communicates using TCP.
         * @param remote_udp_ip The IP address of a remote machine that communicates using UDP.
         * @param local_port The port number of the local machine (used for both UDP and TCP communication).
         * @param remote_tcp_port The port number of a remote machine that communicates using TCP.
         * @param remote_udp_port The port number of a remote machine that communicates using UDP.
         * @param context A reference to a `boost::asio::io_context` context.
         */
        TransportLayerMachine(
            std::string local_ip,
            std::string remote_tcp_ip,
            std::string remote_udp_ip,
            unsigned short local_port,
            unsigned short remote_tcp_port,
            unsigned short remote_udp_port,
            boost::asio::io_context& context
        );

        /**
         * @brief Construct a new Transport Layer Machine object from predefined `boost::asio` endpoint objects.
         * 
         * @param local_udp_end The local machine's UDP endpoint.
         * @param local_tcp_end The local machine's TCP endpoint.
         * @param remote_udp_end A remote machine's UDP endpoint.
         * @param remote_tcp_end A remote machine's TCP endpoint.
         * @param context A reference to a `boost::asio::io_context` used for running asynchronous work.
         */
        TransportLayerMachine(
            boost::asio::ip::udp::endpoint local_udp_end,
            boost::asio::ip::tcp::endpoint local_tcp_end,
            boost::asio::ip::udp::endpoint remote_udp_end,
            boost::asio::ip::tcp::endpoint remote_tcp_end,
            boost::asio::io_context& context
        );

        /**
         * @brief Intended as interface for `Metronome`.
         * 
         * @todo currently undefined.
         * 
         * @param new_subsys Next entry in `SUBSYSTEM_ORDER` enum to advance to. Consider replacing with for `Subsystem::hex`.
         * @param new_state Next entry in `STATE_ORDER` enum to advance to.
         */
        void update(SUBSYSTEM_ORDER new_subsys, STATE_ORDER new_state);

        /**
         * @brief replaces `TransportLayerMachine::commands` with the provided `CommandDeck`.
         * @todo give a more descriptive name like `set_commands`
         * @param new_commands new `CommandDeck` to use when parsing uplinked command messages.
         */
        void add_commands(CommandDeck& new_commands);
        /**
         * @brief replaces `TransportLayerMachine::ring_buffers` with a new interface map.
         * @todo give a more descriptive name like `set_ring_buffer_interface`
         * @param new_ring_buffers new mapping from applicable `System::hex` values to system-specific `RingBufferInterface` objects.
         */
        void add_ring_buffer_interface(std::unordered_map<uint8_t, RingBufferInterface> new_ring_buffers);
        /**
         * @brief replaces `TransportLayerMachine::fragmenter` with a new one.
         * @todo give a more descriptive name like `set_fragmenter`
         * @param new_fragmenter new `Fragmenter` object to use to decimate downlink data stream.
         */
        void add_fragmenter(Fragmenter new_fragmenter);
        /**
         * @brief replaces `TransportLayerMachine::fragmenter` with new one constructed in-place from provided values.
         * 
         * @param fragment_size See Fragmenter constructor.
         * @param header_size See Fragmenter constructor.
         */
        void add_fragmenter(size_t fragment_size, size_t header_size);

        /**
         * @deprecated currently unused.
         */
        void handle_recv();
        /**
         * @brief asynchronously forwards any received TCP packets over UDP.
         */
        void recv_tcp_fwd_udp();
        /**
         * @brief asynchronously forwards any received UDP packets over TCP.
         */
        void recv_udp_fwd_tcp();
        /**
         * @brief parses and acts on a command string received over UDP.
         * 
         * A command string is at least a pair of bytes `<system><command code>` where both the `system` and `command code` values are defined in [foxsi4-commands](https://github.com/foxsi/foxsi4-commands). This method checks a received UDP packet is a valid command string, then delegates handling of the command to `TransportLayerMachine::handle_cmd()`.
         */
        void recv_udp_fwd_tcp_cmd();
        /**
         * @brief asynchronously sends data stored in `TransportLayerMachine::uplink_buffer` to  `TransportLayerMachine::remote_tcp_endpoint`.
         */
        void send_tcp();
        /**
         * @brief asynchronously filters, then sends data stored in `TransportLayerMachine::downlink_buffer` to  `TransportLayerMachine::remote_udp_endpoint`.
         */
        void send_udp(const boost::system::error_code& err, std::size_t byte_count);

        /**
         * @brief convenience method to receive and print UDP packets.
         */
        void print_udp_basic();

        /**
         * @brief parses and acts on or sends a command string in `TransportLayerMachine::uplink_buff`. 
         * 
         * The uplinked command string will be checked against `TransportLayerMachine::commands`. If it is generic, it will be sent asynchronously to the appropriate system. If it is a frame read command (remote ring buffer access), a synchronous remote read-loop is executed and full remote ring buffer data is **printed**. Frame read command status is decided by `TransportLayerMachine::check_frame_read_cmd`.
         * 
         * @todo don't just print remote ring buffer data. Put it in a downlink queue somewhere.
         * @todo support uplink commands with arguments.
         * @todo correctly identify remote read cases where the remote ring buffer will wrap around. Then do multiple reads.
         */
        void handle_cmd();

        /**
         * @brief the target functionality of this method is currently implemented inside `TransportLayerMachine::handle_cmd`. 
         * @todo consider factoring that functionality out of `TransportLayerMachine::handle_cmd`.
         */
        void handle_remote_buffer_transaction();

        /**
         * @brief Extract the data field from a SpaceWire reply sent by `sys`.
         * @todo specify name to SpaceWire e.g. `get_spw_reply_data` or something.
         * @param spw_reply the full, raw reply data (presumed via an SPMU-001).
         * @param sys the `System` sending the reply.
         * @return std::vector<uint8_t> 
         */
        std::vector<uint8_t> get_reply_data(std::vector<uint8_t> spw_reply, System& sys);
        /**
         * @brief Extract the data field from a SpaceWire reply sent by `sys`.
         * @todo specify name to SpaceWire e.g. `get_spw_reply_data` or something.
         * @param spw_reply the full, raw reply data (presumed via an SPMU-001).
         * @param sys the hex code for the system sending the reply.
         * @return std::vector<uint8_t> 
         */
        std::vector<uint8_t> get_reply_data(std::vector<uint8_t> spw_reply, uint8_t sys);

        /**
         * @brief checks if a provided command (lookup in `TransportLayerMachine::commands`) will try to query a remote ring buffer.
         * @todo the frame read commands are just hard-coded defaults currently. Either add an identifying field to `foxsi4-commands` or define these constants in `Parameters.h`.
         * @param sys 1-byte hex code ID for command's remote system.
         * @param cmd 1-byte hex code ID for command.
         * @return true if the command will read continuous data from a remote ring buffer.
         * @return false if the command is generic.
         */
        bool check_frame_read_cmd(uint8_t sys, uint8_t cmd);

};

#endif