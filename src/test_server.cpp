#include "Server.hpp"
#include "messages_storage.hpp"
#include <memory>

int main() {
	simple_messaging::server server(60000); 
	messages_storage storage;
	server.set_storage(std::make_shared<messages_storage>(storage));
	server.start();
	while (true) {
		server.update(-1, true);
	}
	return 0;
}