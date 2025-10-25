#include "queue_with_lock.hpp"
#include "message.hpp"
#include "client.hpp"

#include <chrono>
#include <thread>

int main()
{
	simple_messaging::client cllient;
	cllient.connect("127.0.0.1", 60000);

	bool quit_requested = false;
	while (!quit_requested) {
		// std::this_thread::sleep_for(std::chrono::seconds(2));
		// c.PingServer();
		std::this_thread::sleep_for(std::chrono::seconds(2));
		if (cllient.get_id() != "") {
			cllient.send_to_all("Hello all from " + cllient.get_id());
		}

		if (cllient.is_connected()) {
			if (!cllient.get_incoming_messages().empty()) {
				auto msg = cllient.get_incoming_messages().pop_front().msg;

				switch (msg.header.id) {
				case simple_messaging::MessageType::ServerAccept: {
					cllient.set_id(msg.body);
					std::cout << "Server Accepted Connection, Id given: " << cllient.get_id() << std::endl;
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
				case simple_messaging::MessageType::MessageAll:	{ }
				break;
				case simple_messaging::MessageType::ServerMessage: {
					std::cout << msg.body << std::endl;
				}
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