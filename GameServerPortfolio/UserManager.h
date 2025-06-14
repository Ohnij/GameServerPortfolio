#pragma once
#include <mutex>

class User;

#define USER_MANAGER UserManager::Instance()
class UserManager
{
private:
    UserManager();
    ~UserManager();
public:
    static UserManager& Instance();

    void AddUser(int iSessionID, std::shared_ptr<User> user);
    std::shared_ptr<User> GetSession(int iSessionID);   //���ǹ�ȣ��ã��
    std::shared_ptr<User> GetUser(int iAccountID);      //AccountID��ã��
    std::shared_ptr<User> GetCharacter(int iCharacterID); //CharacterID��ã��
    void RemoveUser(int iSessionID);


private:
    std::mutex m_mutex;
    std::unordered_map<int, std::shared_ptr<User>> m_Users;
};