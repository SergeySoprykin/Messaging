#include "queue_with_lock.hpp"
#include "message.hpp"
#include "client.hpp"

#include <chrono>
#include <cstdint>
#include <string>
#include <thread>

int main(int argc, char* argv[])
{
	std::string my_name;
	std::string dest_name;

	if (argc >= 3) {
        my_name = argv[1];
        dest_name = argv[2];
    } else {
        std::cout << "Need two strings in arguments: [my_name] [dest_name]" << std::endl;
    }

	simple_messaging::client client1;
	client1.set_name(my_name);
	client1.connect("127.0.0.1", 60000);

	bool quit_requested = false;
	std::size_t message_no = 1;
	while (!quit_requested) {
		// std::this_thread::sleep_for(std::chrono::seconds(2));
		// client1.ping_server();
		// std::this_thread::sleep_for(std::chrono::seconds(2));
    	// client1.send_to_all("Hello all from " + client1.get_name());

		std::this_thread::sleep_for(std::chrono::seconds(2));
		std::string message_to_send = "MSG " + std::to_string(message_no++) + " Hi from " + my_name;
    	client1.send_to_client(dest_name,  message_to_send);
		std::cout << "                            -> " << message_to_send << std::endl;

		if (client1.is_connected()) {
			if (!client1.get_incoming_messages().empty()) {
				auto msg = client1.get_incoming_messages().pop().msg;

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
						{simple_messaging::MessageType::ServerTellName, static_cast<uint32_t>(client1.get_name().size())},
						client1.get_name() };
						 client1.send(reply_msg);
					std::cout << "Sent name: " << client1.get_name() << std::endl;
				}
				break;
				default:
				break;
				}
			}
		}
		else
		{
			std::cout << "Server Down\n";
			quit_requested = true;
		}
	}

	return 0;
}