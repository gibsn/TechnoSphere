//TODO
//  Process long messages

#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h>

#include <string>

#define MAX_BUF 1025
#define MAX_MSG 1024


class Application
{
    int sockfd;
    int port;
    std::string ip;
    char buf[MAX_BUF];

    void EstablishConnection();
    void ReadMessage();
    void ProcessSelect();
    int ReadFromSocket();
    void MainLoop();

public:
    Application(int, const char *);
    ~Application();

    void Run();
};


Application::Application(int _port, const char *_ip)
    : port(_port),
    ip(_ip)
{}


Application::~Application()
{
    shutdown(sockfd, 2);
}


void Application::EstablishConnection()
{
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    /*
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    */
    if(!inet_aton(ip.c_str(), &addr.sin_addr))
    {
        fprintf(stderr, "Invalid IP, please try again\n\n");
        exit(0);
    }
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("connect");
        fprintf(stderr, "Please, try again\n\n");
        exit(0);
    }
}


void Application::ReadMessage()
{
    char c;
    int i = 0;

    while(((c = getchar()) != '\n') && i < MAX_MSG)
    {
        buf[i] = c;
        ++i;
    }
    if (c == '\n')
        buf[i] = '\n';
    buf[i + 1] = '\0';
}


int Application::ReadFromSocket()
{
    int n = read(sockfd, buf, MAX_BUF);
    buf[n] = '\0';

    return n;
}


void Application::ProcessSelect()
{
    int res, max_d;
    fd_set readfds;

    max_d = sockfd;
    FD_ZERO(&readfds);
    FD_SET(0, &readfds);
    FD_SET(sockfd, &readfds);
    res = select(max_d + 1, &readfds, 0, 0, 0);
    if (res < 0)
    {
        perror("select");
        exit(0);
    }
    if (FD_ISSET(0, &readfds))
    {
        ReadMessage();
        write(sockfd, buf, strlen(buf));
    }
    if (FD_ISSET(sockfd, &readfds))
    {
        if (ReadFromSocket() <= 0)
        {
            fprintf(stderr, "You have been disconnected\n");
            exit(0);
        }
        printf("%s", buf);
    }
}


void Application::MainLoop()
{
    while(true)
        ProcessSelect();
}


void Application::Run()
{
    EstablishConnection();
    MainLoop();
}


int main()
{
    Application app(3100, "127.0.0.1");

    app.Run();

    return 0;
}