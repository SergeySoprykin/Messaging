#include "client.hpp"

namespace simple_messaging
{

	client::~client() {
		disconnect();
	}

	bool client::connect(const std::string& host, const uint16_t port) {
		try {
			boost::asio::ip::tcp::resolver resolver(asio_context_);
			boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

			connection_ = std::make_unique<connection>(connection::owner::client, asio_context_, boost::asio::ip::tcp::socket(asio_context_), incoming_messages_queue_);
			connection_->connect_to_server(endpoints);
			asio_context_thread_ = std::thread([this]() { asio_context_.run(); });
		}
		catch (std::exception& e) {
			std::cerr << "Client Exception: " << e.what() << "\n";
			return false;
		}
		return true;
	}

	void client::disconnect() {
		if(is_connected()) {
			connection_->disconnect();
		}
		asio_context_.stop();
		if (asio_context_thread_.joinable()) {
			asio_context_thread_.join();
		}
		connection_.release();
	}

	bool client::is_connected() {
		return connection_->is_connected();
	}

	void client::ping_server() {
		simple_messaging::message msg;
		msg.header.id = MessageType::ServerPing;
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();		
		msg << timeNow;
		send_to_server(msg);
	}

	void client::send_to_client(const std::string destination, const std::string body) {
		simple_messaging::message msg;
		msg.header.id = MessageType::MessageClient;
		msg.body = destination + ":" + body;
		msg.header.size = msg.body.size();
		send_to_server(msg);
	}


	void client::send_to_server(const message& msg) {
		if (is_connected()) {
			connection_->send(msg);
		}
	}

	void client::set_name(std::string new_id) {
		id_ = new_id;
	}

	std::string client::get_name() {
		return id_;
	}

	queue_with_lock<owned_message>& client::get_incoming_messages() { 
		return incoming_messages_queue_;
	}

}
