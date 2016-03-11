#include "server.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "clients.h"

#define MAX_BUF 1024


Server::Server(int _port)
    : port(_port)
{}


void Server::CreateListeningSocket()
{
    struct sockaddr_in addr;
    int opt = 1;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    listening_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(listening_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(listening_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        exit(0);
    }
    if (listen(listening_fd, 5) == -1)
    {
        perror("listen");
        exit(0);
    }
    printf("Server is online\n");
}


void Server::AcceptNewClient()
{

}


void Server::MainLoop()
{
    int KQueue = kqueue();

    struct kevent KEvent;
    bzero(&KEvent, sizeof(KEvent));
    EV_SET(&KEvent, listening_fd, EVFILT_READ, EV_ADD, 0, 0, 0);
    kevent(KQueue, &KEvent, 1, NULL, 0, NULL);

    while(true)
    {
        bzero(&KEvent, sizeof(KEvent));
        kevent(KQueue, NULL, 0, &KEvent, 1, NULL);
        if(KEvent.filter == EVFILT_READ)
        {

        }

        if(KEvent.ident == listening_fd)
        {
            int SlaveSocket = accept(listening_fd, 0, 0);
            bzero(&KEvent, sizeof(KEvent));
            EV_SET(&KEvent, SlaveSocket, EVFILT_READ, EV_ADD, 0, 0, 0);
            kevent(KQueue, &KEvent, 1, NULL, 0, NULL);
        }
        else
        {
        }
    }
}


void Server::Run()
{
    CreateListeningSocket();
    MainLoop();
}


Server::~Server()
{
    shutdown(listening_fd, 2);
}


int main(int argc, char **argv)
{
    Server serv(3100);
    serv.Run();

    return 0;
}