#include "ClientManager.h"
#include "Client.h"
#include "ObjectPool.h"
#include <iostream>

ClientManager& ClientManager::Instance()
{
	static ClientManager instance;
	return instance;
}

//socket으로 Client찾기
std::shared_ptr<Client> ClientManager::GetClientBySocket(SOCKET sock)
{
	std::lock_guard<std::mutex> lock(_mutex);
	auto find_client = _socket_clients.find(sock);
	if (find_client == _socket_clients.end())
		return nullptr;

	return find_client->second;
}

std::shared_ptr<Client> ClientManager::GetClientById(int clientId)
{
	std::lock_guard<std::mutex> lock(_mutex);
	auto find_client = _id_clients.find(clientId);
	if (find_client == _id_clients.end())
		return nullptr;

	return find_client->second;
}

int ClientManager::GetClientIdBySocket(SOCKET socket)
{
	auto client = GetClientBySocket(socket);
	if (client == nullptr)
		return -1;
	return client->GetClientID();
}

std::shared_ptr<Client> ClientManager::CreateClient(SOCKET socket)
{
	auto client = PoolManager::GetPool<Client>().Get();
	if (client == nullptr)
		return nullptr;

	int client_uid = GenerateClientId();
	client->SetClient(client_uid, socket);

	std::lock_guard<std::mutex> lock(_mutex);
	_socket_clients.insert(std::make_pair(socket, client));
	_id_clients.insert(std::make_pair(client_uid, client));
	return client;
}

void ClientManager::DisconnectClient(SOCKET socket)
{
	int client_id = GetClientIdBySocket(socket);

	if (client_id < 1)
	{
		std::cerr << "클라이언트가 아닌 소켓!!" << std::endl;
		return;
	}

	bool result = RemoveClient(socket);

	if (result)
	{
		std::cerr << "클라이언트 " << client_id <<" 연결종료!!!" << std::endl;
	}

	return;
}

bool ClientManager::RemoveClient(SOCKET sock)
{
	std::lock_guard<std::mutex> lock(_mutex);

	auto find_socket_client = _socket_clients.find(sock);
	if (find_socket_client == _socket_clients.end())
	{
		std::cerr << "RemoveClient cant finded by socket" << std::endl;
		return false;
	}
	//Todo : 못찾았을떄 로그?
	
	int client_id = find_socket_client->second->GetClientID();
	auto find_id_client = _id_clients.find(client_id);
	if (find_id_client == _id_clients.end())
	{
		std::cerr << "RemoveClient cant finded by id" << std::endl;
		return false;
	}


	auto client_object = find_socket_client->second;
	client_object->ResetClient();
	PoolManager::GetPool<Client>().Return(client_object);

	_socket_clients.erase(sock);
	_id_clients.erase(client_id);
	return true;
}

int ClientManager::GenerateClientId()
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _next_client_id++;
}
