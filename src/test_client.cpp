#include "queue_with_lock.hpp"
#include "message.hpp"
#include "client.hpp"

#include <chrono>
#include <cstdint>
#include <string>
#include <thread>

class my_client : public simple_messaging::client {
public:
	virtual void process_incoming_messages() override {
		if (is_connected()) {
			if (!get_incoming_messages().empty()) {
				auto msg = get_incoming_messages().pop().msg;

				switch (msg.header.id) {
				case simple_messaging::MessageType::ServerAccept: {
					std::cout << "Server Accepted Connection" << std::endl;
				}
				break;
				case simple_messaging::MessageType::ServerPing:	{
					std::chrono::system_clock::time_point time_now = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point message_time;
					msg >> message_time;
					std::cout << "Ping: " << std::chrono::duration<double>(time_now - message_time).count() << "\n";
				}
				break;
				case simple_messaging::MessageType::ServerDeny:	{
					std::cout << "Connection denied: " << std::endl;
				}
				break;
				case simple_messaging::MessageType::ServerMessage: {
					std::cout << " <- " << msg.body << std::endl;
				}
				break;
				case simple_messaging::MessageType::ServerAskName: {
					std::cout << "Server asked client name " << std::endl;
					simple_messaging::message reply_msg {
						{simple_messaging::MessageType::ServerTellName, static_cast<uint32_t>(get_name().size())},
						get_name() };
						 send_to_server(reply_msg);
					std::cout << "Sent name: " << get_name() << std::endl;
				}
				break;
				default:
				break;
				}
			}
		}
	}
};

int main(int argc, char* argv[]) {
	std::string my_name;
	std::string dest_name;

	if (argc >= 3) {
        my_name = argv[1];
        dest_name = argv[2];
    } else {
        std::cout << "Need two strings in arguments: [my_name] [dest_name]" << std::endl;
    }

	my_client msg_client;
	msg_client.set_name(my_name);
	msg_client.connect("127.0.0.1", 60000);
	std::size_t message_no = 1;
	while (msg_client.is_connected()) {
		if (message_no == 1) {
			msg_client.ping_server();
		}
		std::this_thread::sleep_for(std::chrono::seconds(2));
		std::string message_to_send = "MSG " + std::to_string(message_no++) + " Hi from " + my_name;	
    	msg_client.send_to_client(dest_name,  message_to_send);

		std::cout << "                            -> " << message_to_send << std::endl;

		msg_client.process_incoming_messages();
	}
	std::cout << "Server down" << std::endl;
	return 0;
}