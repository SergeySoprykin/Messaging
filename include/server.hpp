#pragma once

#include "queue_with_lock.hpp"
#include "message.hpp"
#include "connection.hpp"

#include <memory>
#include <thread>
#include <iostream>
#include <cstdint>
#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>
#include <cstdint>
#include <sstream>

namespace simple_messaging
{

class server
{
public:
	server(uint16_t port);
	virtual ~server();

	bool start();

	void stop();

	void wait_for_client_connection();

	void send_message_to_client(std::shared_ptr<connection> client, const message& msg);
	
	void send_message_to_all_clients(const message& msg, std::shared_ptr<connection> pIgnoreClient = nullptr);

	void update(size_t nMaxMessages = -1, bool bWait = false);

private:
	virtual bool on_client_connecting(std::shared_ptr<connection> client, uint32_t client_id);
	virtual void on_message(std::shared_ptr<connection> client, message& msg);
	
	queue_with_lock<owned_message> input_messages_queue_;
	std::unordered_map<std::string, std::shared_ptr<connection>> connections_map_;
	boost::asio::io_context asio_context_;
	std::thread asio_context_thread_;
	boost::asio::ip::tcp::acceptor asio_acceptor_;
	uint32_t client_id_counter_ = 10000;
};

}