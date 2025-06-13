#pragma once
#include <mutex>

class User;
class UserManager
{
public:
    static UserManager& Instance();

    void AddUser(int client_id, std::shared_ptr<User> user);
    std::shared_ptr<User> GetUser(int client_id);
    void RemoveUser(int client_id);

private:
    std::unordered_map<int, std::shared_ptr<User>> _user_map;
    std::mutex _mutex;
};