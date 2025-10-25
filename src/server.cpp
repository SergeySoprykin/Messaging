#include "server.hpp"

namespace simple_messaging
{

	server::server(uint16_t port)
		: asio_acceptor_(asio_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) { }

	server::~server() {
		stop();
	}

	bool server::start() {
		try {
			wait_for_client_connection();
			asio_context_thread_ = std::thread([this]() { asio_context_.run(); });
		}
		catch (std::exception& e) {
			std::cerr << "SERVER: Exception: " << e.what() << std::endl;
			return false;
		}
		std::cout << "SERVER: Started!" << std::endl;
		return true;
	}

	void server::stop()	{
		asio_context_.stop();
		if (asio_context_thread_.joinable()) {
			 asio_context_thread_.join();
		}
		std::cout << "SERVER: Stopped!" << std::endl;
	}

	void server::wait_for_client_connection() {
		asio_acceptor_.async_accept(
			[this](std::error_code ec, boost::asio::ip::tcp::socket socket) {
				if (!ec) {
					std::cout << "SERVER: New Connection: " << socket.remote_endpoint() << std::endl;
					std::shared_ptr<connection> new_connection = 
						std::make_shared<connection>(connection::owner::server,	asio_context_, std::move(socket), input_messages_queue_);
					client_id_counter_++;
					if (on_client_connecting(new_connection, client_id_counter_)) {								
						connections_map_[std::to_string(client_id_counter_)] = (std::move(new_connection));
						connections_map_[std::to_string(client_id_counter_)]->connect_to_client(std::to_string(client_id_counter_));
						std::cout << "[" << client_id_counter_ << "] Connection Approved" << std::endl;
					} else {
						std::cout << " Connection Denied" << std::endl;;
					}
				} else {
					std::cout << "SERVER: New Connection Error: " << ec.message() << std::endl;
				}
				wait_for_client_connection();
			});
	}

	void server::send_message_to_client(std::shared_ptr<connection> client_connection, const message& msg) {
		if (client_connection && client_connection->is_connected()) {
			client_connection->send(msg);
		} else {
			std::cout << "Removing client " << client_connection->get_client_id() << std::endl;
			client_connection.reset();
			connections_map_.erase(client_connection->get_client_id());
		}
	}
	
	void server::send_message_to_all_clients(const message& msg, std::shared_ptr<connection> pIgnoreClient)	{
		for (auto& [_, client] : connections_map_) {
			if (client && client->is_connected()) {
				if(client != pIgnoreClient) {
					client->send(msg);
				}
			} else {
		std::cout << "Removing client " << client->get_client_id() << std::endl;
				client.reset();
			}
		}
	}

	void server::update(size_t max_messages, bool wait) {
		if (wait) { 
			input_messages_queue_.wait();
		}

		size_t messages_count = 0;
		while (messages_count < max_messages && !input_messages_queue_.empty()) {
			auto input_message = input_messages_queue_.pop_front();
			on_message(input_message.remote, input_message.msg);
			messages_count++;
		}
	}

	bool server::on_client_connecting(std::shared_ptr<connection> client, uint32_t client_id)	{
		simple_messaging::message reply_message;
		reply_message.header.id = MessageType::ServerAccept;
		std::stringstream ss;
		ss << client_id;
		reply_message.body = ss.str();
		reply_message.header.size  = reply_message.body.size();
		client->send(reply_message);
		return true;
	}

	void server::on_message(std::shared_ptr<connection> client, message& msg) {
		switch (msg.header.id) {
		case MessageType::ServerPing: {
				std::cout << "[" << client->get_client_id() << "]: Server Ping" << std::endl;
				client->send(msg);
			}
			break;
		case MessageType::MessageAll: {
				std::cout << "[" << client->get_client_id() << "]: Message All" << std::endl;;
				msg.header.id = MessageType::ServerMessage;
				send_message_to_all_clients(msg, client);
			}
			break;
		default:
			break;
		}
	}

}