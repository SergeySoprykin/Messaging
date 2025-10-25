#pragma once

#include <mutex>
#include <deque>
#include <algorithm>

#define ASIO_STANDALONE
#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

namespace simple_messaging
{

template<typename T>
class queue_with_lock
{
public:
	queue_with_lock() = default;
	queue_with_lock(const queue_with_lock<T>&) = delete;
	virtual ~queue_with_lock() { clear(); }

public:
	const T& front() {
		std::scoped_lock lock(mutex_);
		return queue_.front();
	}

	const T& back()	{
		std::scoped_lock lock(mutex_);
		return queue_.back();
	}

	T pop_front() {
		std::scoped_lock lock(mutex_);
		auto t = std::move(queue_.front());
		queue_.pop_front();
		return t;
	}

	T pop_back() {
		std::scoped_lock lock(mutex_);
		auto t = std::move(queue_.back());
		queue_.pop_back();
		return t;
	}

	void push_back(const T& item) {
		std::scoped_lock lock(mutex_);
		queue_.emplace_back(std::move(item));

		std::unique_lock<std::mutex> ul(mutex_blocking_);
		condition_variable_.notify_one();
	}

	void push_front(const T& item) {
		std::scoped_lock lock(mutex_);
		queue_.emplace_front(std::move(item));

		std::unique_lock<std::mutex> ul(mutex_blocking_);
		condition_variable_.notify_one();
	}

	bool empty() {
		std::scoped_lock lock(mutex_);
		return queue_.empty();
	}

	size_t count() {
		std::scoped_lock lock(mutex_);
		return queue_.size();
	}

	void clear() {
		std::scoped_lock lock(mutex_);
		queue_.clear();
	}

	void wait() {
		while (empty()) {
			std::unique_lock<std::mutex> ul(mutex_blocking_);
			condition_variable_.wait(ul);
		}
	}

protected:
	std::mutex mutex_;
	std::deque<T> queue_;
	std::condition_variable condition_variable_;
	std::mutex mutex_blocking_;
};

}
