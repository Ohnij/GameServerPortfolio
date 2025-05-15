#pragma once
#include <memory>
#include <queue>
#include <mutex>

//ObjectPool
// ������Ʈ ���� Ǯ���� ���� ���ø�
// ���������� ����Ҽ� ������ �� �ʼ�

template <class T>
class ObjectPool
{
public:

	std::shared_ptr<T> Get() 
	{
		std::lock_guard<std::mutex> lock(_queue_mutex);
		if (_pool_queue.empty())
			return std::make_shared<T>();
		
		auto obj = _pool_queue.front();
		_pool_queue.pop();
		return obj;
	}

	void Return(std::shared_ptr<T> pool_obj)
	{
		std::lock_guard<std::mutex> lock(_queue_mutex);
		_pool_queue.push(pool_obj);
	}
private:

private:
	std::mutex _queue_mutex;
	std::queue<std::shared_ptr<T>> _pool_queue;
};


//PoolManager
// ������ƮǮ �������� �����ϱ�

class PoolManager
{
public:
	template <typename T>
	static ObjectPool<T>& GetPool()
	{
		static ObjectPool<T> instance;
		return instance;
	}
};
