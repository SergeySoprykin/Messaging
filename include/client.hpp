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

	bool connect(const std::string& host, const uint16_t port);
	void disconnect();

	bool is_connected();
	void ping_server();
	void send_to_all(const std::string body);
	void send(const message& msg);

	void set_id(std::string new_id);
	std::string get_id();

	queue_with_lock<owned_message>& get_incoming_messages();

private:
	boost::asio::io_context asio_context_;
	std::thread asio_contect_thread_;
	std::unique_ptr<connection> connection_;
	queue_with_lock<owned_message> incoming_messages_queue_;
	std::string id_;
};

}
