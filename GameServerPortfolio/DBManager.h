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
	DBManager();	//�ܺο��� �������� �ʵ��� Instance������ ����, ȹ��
	~DBManager();

	std::thread _db_thread;
	std::shared_ptr<class DBConnection> _db_connection;

	std::atomic<bool> _running = false;

	std::mutex _mutex;
	std::condition_variable _cv; //DB�۾��� �������� Worker���� (��ź��ٴ� ���� �۾����� ���⶧��)
	std::queue<std::shared_ptr<class DBRequest>> _db_queue;
};

