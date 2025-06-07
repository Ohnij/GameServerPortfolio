#pragma once
#include "IOCPDefine.h"
#include <mutex>
#include <functional>


//ClientManager
// 
// ����� Ŭ���̾�Ʈ ����
// 




class ClientManager
{
public:
	//�̱���
	static ClientManager& Instance();

	inline void SetFactory(std::shared_ptr<class ClientAllocator> allocator) { _allocator = allocator; }


	std::shared_ptr<class Client> GetClientBySocket(SOCKET sock);
	std::shared_ptr<class Client> GetClientById(int clientId);

	int GetClientIdBySocket(SOCKET socket);

	std::shared_ptr<class Client> CreateClient(SOCKET socket);
	void DisconnectClient(SOCKET socket);

private:
	bool RemoveClient(SOCKET sock);


private:
	ClientManager() = default;
	~ClientManager() = default;

	int GenerateClientId();
	std::mutex _mutex;
	std::unordered_map<SOCKET, std::shared_ptr<class Client>> _socket_clients;
	std::unordered_map<int, std::shared_ptr<class Client>> _id_clients;
	int _next_client_id = 1;

	std::shared_ptr<class ClientAllocator> _allocator;
};
