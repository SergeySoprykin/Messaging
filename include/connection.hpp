#pragma once

#include "connection.hpp"
#include "queue_with_lock.hpp"
#include "message.hpp"


namespace simple_messaging
{

class connection : public std::enable_shared_from_this<connection> {
public:
	enum class owner {
		server,
		client
	};

	connection(owner parent, boost::asio::io_context& asio_context, boost::asio::ip::tcp::socket socket, queue_with_lock<owned_message>& input_queue);
	virtual ~connection();

	std::string get_client_id() const;
	void connect_to_client(const std::string& client_uid);
	void connect_to_server(const boost::asio::ip::tcp::resolver::results_type& endpoints);
	void disconnect();
	bool is_connected() const;
	void start_listening();
	void send(const message& msg);

private:
	void write_header();
	void write_body();
	void read_message_header();
	void read_message_body();
	void add_to_incoming_messages_queue();

	boost::asio::ip::tcp::socket socket_;
	boost::asio::io_context& asio_context_;

	queue_with_lock<message> output_messages_queue_;
	queue_with_lock<owned_message>& input_messages_queue_;

	message message_in_construction_;
	owner owner_ = owner::server;
	std::string client_id_;
};

}