#include "Server.hpp"
#include "messages_storage.hpp"
#include <memory>

int main() {
	simple_messaging::server server(60000); 
	simple_messaging::messages_storage storage;
	server.set_storage(std::make_shared<simple_messaging::messages_storage>(storage));
	server.start();
	while (true) {
		server.process_input_messages(-1, true);
	}
	return 0;
}