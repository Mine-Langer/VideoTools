#pragma once
#include <queue>
#include <mutex>

template<class T>
class SafeQueue
{
public:
	SafeQueue() {}

	void Push(const T& val) 
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_data.push(val);
	}

	void MaxSizePush(const T& val,volatile bool* bFlag, const int nMaxSize = 32)
	{
		while (*bFlag)
		{
			_mutex.lock();
			int nSize = _data.size();
			_mutex.unlock();

			if (nSize > nMaxSize)
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			else
				break;
		}
		Push(val);
	}

	bool Pop(T& value)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_data.empty())
			return false;

		value = _data.front();
		_data.pop();

		return true;
	}

	bool Empty()
	{
		return _data.empty();
	}

private:
	std::queue<T> _data;
	std::mutex _mutex;
};