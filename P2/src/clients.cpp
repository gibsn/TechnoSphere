#include "clients.h"

#include <sys/socket.h>


Client::Client(int _fd)
    : fd(_fd),
    q(std::queue<std::string>())
{}


Client::~Client()
{
    shutdown(fd, 2);
}


void Client::Write()
{
    int n = 0;
    std::string msg;

    while (q.size() > 0 && n + q.front().size() < MAX_MSG)
    {
        msg += q.front();
        q.pop();
        n += msg.size();
    }
    write(fd, msg.c_str(), msg.size());
}


char *Client::Read()
{
    int n = read(fd, buf, MAX_BUF);
    buf[n] = '\0';

    if (n != 0)
        return buf;
    else
        return 0;
}


void Client::AddToQueue(std::string s)
{
    q.push(s);
}


Clients::Clients()
    : arr(std::vector<Client *>())
{}


Client &Clients::GetClientByFd(int fd)
{
    for (int i = 0; i < arr.size(); ++i)
        if (arr[i]->GetFd() == fd)
            return *arr[i];

    return *arr[0];
}


void Clients::AddClient(int fd)
{
    arr.push_back(new Client(fd));
}


void Clients::DeleteClient(int fd)
{
    int i = 0;
    for (i = 0; i < arr.size(); ++i)
        if (arr[i]->GetFd() == fd)
            break;

    arr.erase(arr.begin() + i);
}


Clients::~Clients()
{
    for (int i = 0; i < arr.size(); ++i)
        delete arr[i];
}