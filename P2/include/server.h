#ifndef SERVER_H_SENTRY
#define SERVER_H_SENTRY

#include "clients.h"

class Server
{
    int listening_fd;
    int port;
    Clients clients;

    void CreateListeningSocket();
    int AcceptNewClient();
    void AddNewClientEvent(int, struct kevent *&, int &);
    void ExpandEventList(struct kevent *&, int &);
    void ResetChangeList(struct kevent *&, int &);
    void AddToBroadcastQueue(std::string);
    void DeleteClient(int, struct kevent *&, int &);
    void MainLoop();

public:
    Server(int);
    ~Server();

    void Run();
};

#endif