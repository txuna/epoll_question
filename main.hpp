#ifndef __MAIN_H_
#define __MAIN_H_

#include <thread>
#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "epoll_event.hpp"
#include "sock.hpp"
#include "common.hpp"

#define PORT 9988

#define SINGLE_ACCEPT 1
#define SINGLE_EPOLL_ACCEPT 2
#define MULTIPLE_ACCEPT 3
#define MULTIPLE_EPOLL_ACCEPT 4

#define WORKER_PROCESS_NUM 20

class Accept
{
    public:
        Accept();
        ~Accept();
        void MultiRun();
        void WaitProcess();

};

/*
class SingleAccept : public Accept
{
    public:
        SingleAccept();
        ~SingleAccept();
        void Run();
};
*/

class SingleEpollAccept : public Accept
{
    public:
        SingleEpollAccept();
        ~SingleEpollAccept();
        void Run();
};

/*
class MultipleAccept : public Accept
{
    public:
        MultipleAccept();
        ~MultipleAccept();
        void Run();
};

class MultipleEpollAccept : public Accept
{
    private:
        Epoll::EventLoop el;
    public:
        MultipleEpollAccept();
        ~MultipleEpollAccept();
        void Run();
}; 
*/
#endif