#pragma once
#include <queue>
#include <mutex>

template<class T>
class SafeQueue 
{
public:
	SafeQueue() { }

	void Push(const T& val) {
		std::lock_guard<std::mutex> lock(_mut);
		_qdata.push(val);
	}

	void MaxSizePush(const T& val, const int nMaxSize = 32) {
		while (true) {
			_mut.lock();
			int nSize = _qdata.size();
			_mut.unlock();

			if (nSize > nMaxSize)
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			else
				break;
		}
		Push(val);
	}

	bool Pop(T& value) {
		std::lock_guard<std::mutex> lock(_mut);
		if (_qdata.empty())
			return false;

		value = _qdata.front();
		_qdata.pop();

		return true;
	}

	int Size() {
		std::lock_guard<std::mutex> lock(_mut);
		return _qdata.size();
	}

private:
	std::queue<T> _qdata;
	std::mutex _mut;
};