#pragma once
#include "connection.hpp"
#include <cstdint>

namespace simple_messaging
{

class client
{
public:
	client(){}
	~client();

	bool connect_to_server(const std::string& host, const uint16_t port);
	void disconnect_from_server();

	bool is_connected();
	void ping_server();
	void send_to_another_client(const std::string destination_client_id, const std::string body);
	void send_to_server(const message& msg);

	void set_name(const std::string& new_id);
	std::string get_name();

	queue_with_lock<owned_message>& get_input_messages();

	virtual void process_incoming_messages() {}

private:
	boost::asio::io_context asio_context_;
	std::thread asio_context_thread_;
	std::unique_ptr<connection> connection_;
	queue_with_lock<owned_message> input_messages_queue_;
	std::string id_;
};

}
