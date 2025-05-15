#include "ClientManager.h"
#include "Client.h"
#include "ObjectPool.h"


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

std::shared_ptr<Client> ClientManager::CreateClient(SOCKET socket)
{
	auto client = PoolManager::GetPool<Client>().Get();

	int client_uid = GenerateClientId();
	client->SetClient(client_uid);
	client->_socket = socket;


	std::lock_guard<std::mutex> lock(_mutex);
	_socket_clients.insert(std::make_pair(socket, client));
	_id_clients.insert(std::make_pair(client_uid, client));
	return client;
}

void ClientManager::RemoveClient(SOCKET sock)
{
	std::lock_guard<std::mutex> lock(_mutex);

	auto find_socket_client = _socket_clients.find(sock);
	if (find_socket_client == _socket_clients.end())
		return;
	//Todo : 못찾았을떄 로그?
	
	int client_id = find_socket_client->second->_client_number;
	auto find_id_client = _id_clients.find(client_id);
	if (find_id_client == _id_clients.end())
		return;


	auto client_object = find_socket_client->second;
	PoolManager::GetPool<Client>().Return(client_object);

	_socket_clients.erase(sock);
	_id_clients.erase(client_id);
}

int ClientManager::GenerateClientId()
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _next_client_id++;
}
