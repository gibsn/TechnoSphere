#ifndef SERVER_H_SENTRY
#define SERVER_H_SENTRY

#include "clients.h"

class Server
{
    int listening_fd;
    int port;
    Clients clients;

    void CreateListeningSocket();
    void AcceptNewClient();
    void MainLoop();

public:
    Server(int);
    ~Server();

    void Run();
};

#endif