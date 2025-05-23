#pragma once
#include <memory>
#include <queue>
#include <mutex>

//ObjectPool
// 오브젝트 통합 풀링을 위한 템플릿
// 여러군데서 사용할수 있으니 락 필수

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
				return nullptr; //리미트가 있는데 현재 카운트랑 똑같다면 이미 다만들었음
			}
			++_pool_object_count; //생성시++
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

	//미리 생성
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
	/// 최대 풀링 오브젝트 제한
	/// </summary>
	/// <param name="limit_count"> : 0=not limit, 0>current limit, n=limit n</param>
	void LimitCount(int limit_count)
	{
		std::lock_guard<std::mutex> lock(_queue_mutex);
		if (limit_count < 0) //음수 = 현재카운트에서 생성제한
			_pool_object_limit = _pool_object_count;
		else
			_pool_object_limit = limit_count; //입력값
	}
private:

private:
	std::mutex _queue_mutex;
	int _pool_object_count = 0; //객체생성된 수;
	int _pool_object_limit = 0; //생성제한
	std::queue<std::shared_ptr<T>> _pool_queue;
};


//PoolManager
// 오브젝트풀 전역으로 관리하기

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
