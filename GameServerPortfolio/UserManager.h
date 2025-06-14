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
    std::shared_ptr<User> GetSession(int iSessionID);   //세션번호로찾기
    std::shared_ptr<User> GetUser(int iAccountID);      //AccountID로찾기
    std::shared_ptr<User> GetCharacter(int iCharacterID); //CharacterID로찾기
    void RemoveUser(int iSessionID);


private:
    std::mutex m_mutex;
    std::unordered_map<int, std::shared_ptr<User>> m_Users;
};