#include "connection.hpp"

namespace simple_messaging
{

	connection::connection(owner parent, boost::asio::io_context& asio_context, boost::asio::ip::tcp::socket socket, queue_with_lock<owned_message>& input_messages_queue)
		: socket_(std::move(socket)), asio_context_(asio_context), input_messages_queue_(input_messages_queue) {
		owner_ = parent;
	}

	connection::~connection() {}

	std::string connection::get_client_id() const {
		return client_id_;
	}

	void connection::connect_to_client(const std::string& new_client_id) {
		if (owner_ == owner::server) {
			if (socket_.is_open()) {
				client_id_ = new_client_id;
				read_message_header();
			}
		}
	}

	void connection::connect_to_server(const boost::asio::ip::tcp::resolver::results_type& endpoints) {
		if (owner_ == owner::client) {
			boost::asio::async_connect(socket_, endpoints,
				[this](std::error_code ec, [[maybe_unused]]boost::asio::ip::tcp::endpoint endpoint) {
					if (!ec) {
						read_message_header();
					}
				});
		}
	}

	void connection::disconnect() {
		if (is_connected()) {
			boost::asio::post(asio_context_, [this]() { socket_.close(); });
		}
	}

	bool connection::is_connected() const {
		return socket_.is_open();
	}

	void connection::start_listening() {}

	void connection::send(const message& msg) {
		boost::asio::post(asio_context_,
			[this, msg]() {
				bool already_writing = !output_messages_queue_.empty();
				output_messages_queue_.push(msg);
				if (!already_writing) {
					write_header();
				}
			});
	}

	void connection::write_header() {
		boost::asio::async_write(socket_, boost::asio::buffer(&output_messages_queue_.front().header, sizeof(message_header)),
			[this](std::error_code ec, [[maybe_unused]]std::size_t length) {
				if (!ec) {
					if (output_messages_queue_.front().header.size > 0)	{
						write_body();
					} else {
						output_messages_queue_.pop();
						if (!output_messages_queue_.empty()) {
							write_header();
						}
					}
				} else {
					socket_.close();
				}
			});
	}

	void connection::write_body() {
		boost::asio::async_write(socket_, boost::asio::buffer(output_messages_queue_.front().body.data(), output_messages_queue_.front().header.size),
			[this](std::error_code ec, [[maybe_unused]]std::size_t length)
			{
				if (!ec) {
					output_messages_queue_.pop();
					if (!output_messages_queue_.empty()) {
						write_header();
					}
				} else {
					socket_.close();
				}
			});
	}

	void connection::read_message_header() {
		boost::asio::async_read(socket_, boost::asio::buffer(&message_in_construction_.header, sizeof(message_header)),
			[this](std::error_code ec, [[maybe_unused]]std::size_t length){					
				if (!ec) {
					if (message_in_construction_.header.size > 0) {
						message_in_construction_.body.resize(message_in_construction_.header.size);
						read_message_body();
					} else {
						add_to_incoming_messages_queue();
					}
				} else {
					socket_.close();
				} });
	}

	void connection::read_message_body()	{
		boost::asio::async_read(socket_, boost::asio::buffer(message_in_construction_.body.data(), message_in_construction_.header.size),
			[this](std::error_code ec, [[maybe_unused]]std::size_t length) {						
				if (!ec) {
					add_to_incoming_messages_queue();
				}
				else {
					socket_.close();
				}
			});
	}

	void connection::add_to_incoming_messages_queue() {				
		if(owner_ == owner::server) {
			input_messages_queue_.push({ this->shared_from_this(), message_in_construction_ });
		} else {
			input_messages_queue_.push({ nullptr, message_in_construction_ });
		}
		read_message_header();
	}

}