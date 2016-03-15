#include "server.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "clients.h"

#define MAX_BUF 10240
#define MAX_MSG 1024


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
    int flags = fcntl(listening_fd, F_GETFL, 0);
    fcntl(listening_fd, F_SETFL, flags | O_NONBLOCK);
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
}


int Server::AcceptNewClient()
{
    int new_player_fd = accept(listening_fd, 0, 0);
    int flags = fcntl(new_player_fd, F_GETFL, 0);
    fcntl(new_player_fd, F_SETFL, flags | O_NONBLOCK);

    if (new_player_fd != -1)
        clients.AddClient(new_player_fd);
    else
    {
        perror("accept");
        exit(0);
    }

    clients[clients.GetSize() - 1].AddToQueue(std::string("Welcome!\n"));
    fprintf(stdout, "LOG: accepted connection\n");
    fflush(stdout);

    return new_player_fd;
}


void Server::AddToBroadcastQueue(std::string s)
{
    for (int i = 0; i < clients.GetSize(); ++i)
        clients[i].AddToQueue(s);
}


void Server::AddNewClientEvent(int fd, struct kevent *&changelist, int &nchanges)
{
    nchanges += 2;
    int size = nchanges * sizeof(*changelist);
    changelist = (struct kevent *)realloc(changelist, size);
    bzero(changelist + (nchanges - 1), 2 * sizeof(changelist));
    EV_SET(changelist, fd, EVFILT_READ, EV_ADD, 0, 0, 0);
    EV_SET(changelist + 1, fd, EVFILT_WRITE, EV_ADD, 0, 0, 0);
}


void Server::ExpandEventList(struct kevent *& eventlist, int &nevents)
{
    nevents += 2;
    int size = nevents * sizeof(*eventlist);
    eventlist = (struct kevent *)realloc(eventlist, size);
}


void Server::ResetChangeList(struct kevent *& changelist, int &nchanges)
{
    if (changelist)
    {
        free(changelist);
        changelist = 0;
        nchanges = 0;
    }
}


void Server::DeleteClient(int fd, struct kevent *&changelist, int &nchanges)
{
    nchanges += 2;
    int size = nchanges * sizeof(*changelist);
    changelist = (struct kevent *)realloc(changelist, size);
    bzero(changelist + (nchanges - 1), 2 * sizeof(changelist));
    EV_SET(changelist, fd, EVFILT_READ, EV_DELETE, 0, 0, 0);
    EV_SET(changelist + 1, fd, EVFILT_WRITE, EV_DELETE, 0, 0, 0);

    clients.DeleteClient(fd);
    fprintf(stdout, "LOG: connection terminated\n");
    fflush(stdout);
}


void Server::MainLoop()
{
    int kq = kqueue();

    struct kevent *changelist = (struct kevent *)malloc(sizeof(*changelist));
    struct kevent *eventlist = (struct kevent *)malloc(sizeof(*eventlist));
    int nchanges = 1;
    int nevents = nchanges;

    bzero(changelist, sizeof(*changelist));
    EV_SET(changelist, listening_fd, EVFILT_READ, EV_ADD, 0, 0, 0);

    while(true)
    {
        bzero(eventlist, nevents * sizeof(*eventlist));
        int kn = kevent(kq, changelist, nchanges, eventlist, nevents, 0);
        ResetChangeList(changelist, nchanges);

        for (int i = 0; i < kn; ++i)
        {
            if(eventlist[i].ident == listening_fd)
            {
                int fd = AcceptNewClient();
                AddNewClientEvent(fd, changelist, nchanges);
                ExpandEventList(eventlist, nevents);
                continue;
            }
            if(eventlist[i].filter == EVFILT_READ)
            {
                Client &client = clients.GetClientByFd(eventlist[i].ident);
                int n = client.Read();
                if (n != 0)
                {
                    char *msg = client.GetMessage();
                    if (msg)
                    {
                        fprintf(stdout, "LOG: %s", msg);
                        fflush(stdout);
                        AddToBroadcastQueue(msg);
                    }
                }
                else
                    DeleteClient(eventlist[i].ident, changelist, nchanges);
            }
            if(eventlist[i].filter == EVFILT_WRITE)
            {
                clients.GetClientByFd(eventlist[i].ident).Write();
            }
        }
    }
    free(eventlist);
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