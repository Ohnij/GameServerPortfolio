#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

class DBManager 
{
public:
	static DBManager& Instance();

	bool Init();

	void DBWorkerThread();
	
	void PushRequest(std::shared_ptr<class DBRequest>&& request);
	std::shared_ptr<class DBRequest> PopRequest();


	void DBTest();
private:
	DBManager();	//외부에서 생성하지 않도록 Instance에서만 생성, 획득
	~DBManager();

	std::thread _db_thread;
	std::shared_ptr<class DBConnection> _db_connection;

	std::atomic<bool> _running = false;

	std::mutex _mutex;
	std::condition_variable _cv; //DB작업이 있을때만 Worker돌게 (통신보다는 비교적 작업량이 적기때문)
	std::queue<std::shared_ptr<class DBRequest>> _db_queue;
};

