#pragma once
#include "IOCPDefine.h"
#include <memory>
#include <unordered_map>
#include <mutex>


//ClientManager
// 
// 연결된 클라이언트 관리
// 

class Client;
class ClientManager
{
public:
	//싱글톤
	static ClientManager& Instance();


	std::shared_ptr<Client> GetClientBySocket(SOCKET sock);
	std::shared_ptr<Client> GetClientById(int clientId);


	std::shared_ptr<Client> CreateClient(SOCKET socket);
	void RemoveClient(SOCKET sock);


private:
	int GenerateClientId();
	std::mutex _mutex;
	std::unordered_map<SOCKET, std::shared_ptr<Client>> _socket_clients;
	std::unordered_map<int, std::shared_ptr<Client>> _id_clients;
	int _next_client_id = 1;
};

