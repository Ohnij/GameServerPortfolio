#include "stdafx.h"
#include "DBManager.h"
#include "DBConnection.h"
#include "DBRequest.h"



DBManager::DBManager()
{
    _db_connection = std::make_shared<DBConnection>();
}
DBManager::~DBManager()
{
    _running = false;
    _cv.notify_all(); //모든스레드 종료 타도록
    if (_db_thread.joinable())
        _db_thread.join();
}


DBManager& DBManager::Instance()
{
    static DBManager instance;
    return instance;
}

bool DBManager::Init()
{
    //!! -- 외부에서 호출해서 제대로 Init 되었는지 확인 -- !!
	if (!_db_connection->Init())
		return false;

    _running = true; // false 면 스레드 종료
	_db_thread = std::thread([this](){DBWorkerThread();});
	return true;
}

void DBManager::DBWorkerThread()
{
    while (_running)
    {
        auto request = PopRequest();
        if (request)
            request->Execute(_db_connection);
    }
}

void DBManager::PushRequest(std::shared_ptr<DBRequest>&& request)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _db_queue.push(std::move(request));
    }
    _cv.notify_one();
}

std::shared_ptr<DBRequest> DBManager::PopRequest()
{
    //_cv 사용시 unique lock (다시 반환해줘야하기때문
    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait(lock, [&] {
        //여기엔 조건이 들어가야함 
        //기다리는조건 == true면 lock얻고 진행 
        //               false면 lock해제후 커널에 잠자기 _cv.notice..로 깨우면 다시 조건체크함.
        return !_db_queue.empty() || !_running;
    });

    if (!_db_queue.empty())
    {
        auto req = _db_queue.front();
        _db_queue.pop();
        return req;
    }
    return nullptr;
}



void DBManager::DBTest()
{
    if (!_running)
        return;

    //_db_connection->TestQuery();
    //_db_connection->TestQuery2();

    //std::string id = "test";
    //std::string pw = "testpw";
    //int out_id;
    //int out_res;
    //_db_connection->TestQuery3(id, pw, out_id, out_res);

    //printf("id:%d, res:%d\n", out_id, out_res);
}