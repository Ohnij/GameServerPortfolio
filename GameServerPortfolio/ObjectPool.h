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
		{
			if (_pool_object_limit != 0 && _pool_object_limit <= _pool_object_count  )
			{
				return nullptr; //����Ʈ�� �ִµ� ���� ī��Ʈ�� �Ȱ��ٸ� �̹� �ٸ������
			}
			++_pool_object_count; //������++
			return std::make_shared<T>();
		}
		auto obj = _pool_queue.front();
		_pool_queue.pop();
		return obj;
	}

	void Return(std::shared_ptr<T> pool_obj)
	{
		std::lock_guard<std::mutex> lock(_queue_mutex);
		_pool_queue.push(pool_obj);
	}

	//�̸� ����
	void ReserveObject(int reserve_count)
	{
		std::lock_guard<std::mutex> lock(_queue_mutex);
		for (; _pool_object_count < reserve_count; ++_pool_object_count)
		{
			auto object = std::make_shared<T>();
			_pool_queue.push(object);
		}
	}

	/// <summary>
	/// �ִ� Ǯ�� ������Ʈ ����
	/// </summary>
	/// <param name="limit_count"> : 0=not limit, 0>current limit, n=limit n</param>
	void LimitCount(int limit_count)
	{
		std::lock_guard<std::mutex> lock(_queue_mutex);
		if (limit_count < 0) //���� = ����ī��Ʈ���� ��������
			_pool_object_limit = _pool_object_count;
		else
			_pool_object_limit = limit_count; //�Է°�
	}
private:

private:
	std::mutex _queue_mutex;
	int _pool_object_count = 0; //��ü������ ��;
	int _pool_object_limit = 0; //��������
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
