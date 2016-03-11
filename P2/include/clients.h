#ifndef CLIENTS_H_SENTRY
#define CLIENTS_H_SENTRY

#include <vector>

class Client
{
    int fd;
};

class Clients
{
    std::vector<Client> arr;

public:
    Clients();
};


#endif