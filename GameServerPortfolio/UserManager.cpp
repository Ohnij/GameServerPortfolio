#include "stdafx.h"
#include "UserManager.h"




UserManager::UserManager()
{
}

UserManager::~UserManager()
{
}

UserManager& UserManager::Instance()
{
	static UserManager instance;
	return instance;
}

void UserManager::AddUser(int iSessionID, std::shared_ptr<User> user)
{
}

std::shared_ptr<User> UserManager::GetSession(int iSessionID)
{
	return std::shared_ptr<User>();
}

std::shared_ptr<User> UserManager::GetUser(int iAccountID)
{
	return std::shared_ptr<User>();
}

std::shared_ptr<User> UserManager::GetCharacter(int iCharacterID)
{
	return std::shared_ptr<User>();
}

void UserManager::RemoveUser(int iSessionID)
{
}
