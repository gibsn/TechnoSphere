#ifndef CLIENTS_H_SENTRY
#define CLIENTS_H_SENTRY

#include <vector>
#include <string>
#include <queue>

#define MAX_BUF 1025
#define MAX_MSG 1024

class Client
{
    int fd;
    char buf[MAX_BUF];
    std::queue<std::string> q;

public:
    Client(int);
    ~Client();

    int GetFd() const { return fd; }

    void Write();
    char *Read();
    void AddToQueue(std::string);
};


class Clients
{
    std::vector<Client *> arr;

public:
    Clients();
    ~Clients();

    int GetSize() const { return arr.size(); }
    Client &GetClientByFd(int);

    void AddClient(int);
    void DeleteClient(int);
    Client &operator[] (int idx) { return *arr[idx]; }
};


#endif