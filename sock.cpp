#include "sock.hpp"
#include "epoll_event.hpp"

#include <iostream>

#define LISTEN_BACKLOG 511

Net::SockAddr::SockAddr(int port)
{
    memset(&adr, 0, sizeof(adr));
    adr.sin_family = AF_INET;
    adr.sin_addr.s_addr = htonl(INADDR_ANY);
    adr.sin_port = htons(port);
}

Net::SockAddr::SockAddr(struct sockaddr_in a)
{
    adr = a;
}

Net::SockAddr::~SockAddr()
{

}

Net::Socket::Socket()
{
    fd = -1;
    sock_addr = nullptr;
    querybuf = nullptr; 
}

int Net::Socket::SetSocketReusePort()
{
    int on = 1;
    if(setsockopt(this->fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) < 0)
    {
        return C_ERR;
    }

    return C_OK;
}

int Net::Socket::SetNonBlock()
{
    int flags;

    if((flags = fcntl(this->fd, F_GETFL, 0)) == -1)
    {
        return C_ERR;
    }
    
    if(fcntl(this->fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        return C_ERR;
    }

    return C_OK;
}

int Net::Socket::CreateSocket()
{
    this->fd = socket(PF_INET, SOCK_STREAM, 0);
    if(this->fd == -1)
    {
        return C_ERR;
    }

    int on = 1;
    if(setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        return C_ERR;
    }

    type = SERVER;

    return C_OK;
}

int Net::Socket::BindSocket()
{
    if(bind(this->fd, (struct sockaddr*)&sock_addr->adr, sizeof(sock_addr->adr)) == -1)
    {
        return C_ERR;
    }

    return C_OK;
}

Net::Socket::~Socket()
{
    if(sock_addr != nullptr)
    {
        delete sock_addr;
    }
}

Net::TcpSocket::TcpSocket(SockAddr *adr, int _mask)
{
    this->fd = -1;
    sock_addr = adr; 
    this->mask = _mask;
}

Net::TcpSocket::TcpSocket(SockAddr *adr, int _mask, socket_t fd)
{
    this->fd = fd;
    sock_addr = adr;
    this->mask = _mask;
}

/* socket_nread를 사용하여 얼마큼 읽을 수 있는지 확인 */
int Net::TcpSocket::ReadSocket()
{
    if(socket_nread(fd, &querylen) == -1)
    {
        return C_ERR;
    }
    
    querybuf = new byte_t[querylen+1];
    int ret = read(fd, querybuf, querylen);
    if(ret < 0)
    {
        if(errno == EAGAIN)
        {
            return C_YET;
        }

        return C_ERR;
    }

    return C_OK;
}

int Net::TcpSocket::SendSocket(byte_t *buffer, int len)
{
    while(true)
    {
        int ret = write(fd, buffer, len);
        if(ret < 0)
        {
            if(errno == EAGAIN)
            {
                continue;
            }
            return C_ERR;
        }

        break;
    }
    return C_OK;
}


int Net::TcpSocket::ListenSocket()
{
    if(listen(this->fd, LISTEN_BACKLOG) == -1)
    {
        return C_ERR;
    }

    return C_OK;
}


Net::TcpSocket *Net::TcpSocket::AcceptSocket()
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    socket_t fd = accept4(this->fd, (sockaddr*)&addr, &addr_len, SOCK_NONBLOCK);

    if(fd <= 0)
    {
        return nullptr;
    }

    Net::SockAddr *sock = new Net::SockAddr(addr);
    Net::TcpSocket *socket = new Net::TcpSocket(sock, EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR, fd);\
    socket->type = CLIENT;

    return socket;
}

Net::TcpSocket::~TcpSocket()
{
    if(this->fd != -1)
    {
        close(this->fd);
    }
}
