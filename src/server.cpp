#include "server.hpp"
#include "message.hpp"
#include <chrono>
#include <thread>

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
					on_client_connecting(new_connection, client_id_counter_);						
					connections_map_[std::to_string(client_id_counter_)] = (std::move(new_connection));
					connections_map_[std::to_string(client_id_counter_)]->connect_to_client(std::to_string(client_id_counter_));
					std::cout << "[" << client_id_counter_ << "] Connection Approved" << std::endl;
					message msg{{MessageType::ServerAskName}, ""};
					send_message_to_client(connections_map_[std::to_string(client_id_counter_)], msg);
				} else {
					std::cout << "SERVER: New Connection Error: " << ec.message() << std::endl;
				}
				wait_for_client_connection();
			});
	}

	bool server::send_message_to_client(std::shared_ptr<connection> client_connection, const message& msg, bool save_to_pending) {
		if (client_connection && client_connection->is_connected()) {
			client_connection->send(msg);
			if(storage_ ) {
				storage_->save_message( msg.body);
			}
			return true;
		}
		std::cout << "Client " << client_connection->get_client_id() << " unreachable" << std::endl;
		if(storage_ && save_to_pending) {
			storage_->save_message(msg.body, true);
		}
		return false;
	}

	bool server::send_message_to_client_by_name(std::string client_name, const message& msg, bool save_to_pending) {
		if(client_name_to_id_.count(client_name) > 0) {
			auto destination_client_connection = connections_map_[client_name_to_id_[client_name]];
			return send_message_to_client(destination_client_connection, msg, save_to_pending);
		}
		return false;
	}

	
	void server::send_message_to_all_clients(const message& msg, std::shared_ptr<connection> pIgnoreClient)	{
		for (auto& [_, client_connection] : connections_map_) {
			if (client_connection && client_connection->is_connected()) {
				if(client_connection != pIgnoreClient) {
					client_connection->send(msg);
					if(storage_) {
						storage_->save_message(msg.body);
					}
				}
			} else {
				std::cout << "Client " << client_connection->get_client_id() << " unreachable" << std::endl;
				if(storage_) {
					storage_->save_message(msg.body, true);
				}
			}
		}
	}

	void server::update(size_t max_messages, bool wait) {
		if (wait) { 
			input_messages_queue_.wait();
		}

		size_t messages_count = 0;
		while (messages_count < max_messages && !input_messages_queue_.empty()) {
			auto input_message = input_messages_queue_.pop();
			process_message(input_message.remote, input_message.msg);
			process_pending_messages();
			messages_count++;
		}
	}

	bool server::on_client_connecting(std::shared_ptr<connection> client_connection, uint32_t client_id)	{
		simple_messaging::message reply_message;
		reply_message.header.id = MessageType::ServerAccept;
		std::stringstream ss;
		ss << client_id;
		reply_message.body = ss.str();
		reply_message.header.size  = reply_message.body.size();
		client_connection->send(reply_message);
		return true;
	}

	void server::process_message(std::shared_ptr<connection> client_connection, message& msg) {
		switch (msg.header.id) {
		case MessageType::ServerPing: {
				std::cout << "[" << client_connection->get_client_id() << "]: Server Ping" << std::endl;
				client_connection->send(msg);
			}
			break;
		case MessageType::MessageAll: {
				std::cout << "[" << client_connection->get_client_id() << "]: Message All" << std::endl;
				msg.header.id = MessageType::ServerMessage;
				send_message_to_all_clients(msg, client_connection);
			}
			break;
		case MessageType::MessageClient: {
				std::string destination_client_name = msg.body.substr(0, msg.body.find(":"));
				msg.header.id = MessageType::ServerMessage;
				if(client_name_to_id_.count(destination_client_name) > 0) {
					std::cout << "[" << client_connection->get_client_id() << "]: Message to " << destination_client_name << std::endl;
					auto destination_client_connection = connections_map_[client_name_to_id_[destination_client_name]];
					send_message_to_client(destination_client_connection, msg);
				}
			}
			break;
		case MessageType::ServerTellName: {
				std::cout << "[" << client_connection->get_client_id() << "]: Message <" << msg.body <<  ">" << std::endl;
				client_name_to_id_[msg.body] = client_connection->get_client_id();
			}
			break;
		default:
			break;
		}
	}

	void server::process_pending_messages() {
		if (storage_) {
			auto pending_messages_files = storage_->list_messages();
			std::cout << "Pending messages: " << pending_messages_files.size() << std::endl;
			for (const auto& pending_message_file : pending_messages_files) {
				std::string pending_message = storage_->read_message(pending_message_file, true);
				std::string destination_client_name = pending_message.substr(0, pending_message.find(":"));
				message msg {{MessageType::ServerMessage, static_cast<uint32_t>(pending_message.size())}, pending_message};
				if (send_message_to_client_by_name(destination_client_name, msg, false)) {
					storage_->move_to_delivered(pending_message_file);
				}
			}
		}
	}

	void server::set_storage(std::shared_ptr<messages_storage> messages_storage) {
		storage_ = messages_storage;
	}

}