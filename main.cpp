#include "main.hpp"


/**
 * 입증
 * 1. 동일한 listener socket을 공유하는 다수의 워커 프로세스가 accept을 시도함
 * 2. 동일한 listener socket을 공유하는 다수의 워커 프로세스가 epoll-accept 시도함
 *
 * 3. SO_REUSEPORT 옵션을 사용하여 각 각의 워커 프로세스가 동일 포트에 소켓을 각 각 바인딩 시도하여 accept을 시도함
 * 4. SO_REUSEPORT 옵션을 사용하여 각 각의 워커 프로세스가 동일 포트에 소켓을 각 각 바인딩 시도하여 epoll-accept을 시도함
 * 
 * 이 때 프로세스가 적절하게 로드밸런싱 되는지 확인하기
*/

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        std::cout<<"[Usage] ./server [OPTION]"<<std::endl;
        std::cout<<"---OPTION LIST---"<<std::endl;
        std::cout<<"1. SINGLE ACCEPT"<<std::endl;
        std::cout<<"2. SINGLE EPOLL ACCEPT"<<std::endl;
        std::cout<<"3. MULTIPLE ACCEPT"<<std::endl;
        std::cout<<"4. MULTIPLE EPOLL ACCEPT"<<std::endl;
        return 1;
    }

    int option = atoi(argv[1]);
    switch (option)
    {
        /*
        case SINGLE_ACCEPT:
        { 
            SingleAccept single_accept; 
            single_accept.Run();
            break;
        }
        */
        
        case SINGLE_EPOLL_ACCEPT:
        {
            SingleEpollAccept single_epoll_accept; 
            single_epoll_accept.Run();
            break;
        }
        /*
        case MULTIPLE_ACCEPT:
        {
            MultipleAccept multiple_accept; 
            multiple_accept.Run();
            break;
        }

        case MULTIPLE_EPOLL_ACCEPT:
        {
            MultipleEpollAccept multiple_epoll_accept;
            multiple_epoll_accept.Run();
            break;
        }
        */

        default:
            std::cout<<"Invalid Option"<<std::endl;
            break;
    }

    return 0;
}

/* Accept Class */
Accept::Accept()
{

}

void Accept::WaitProcess()
{
    /* WAIT */
    while(true)
    {
        int status; 
        pid_t done = wait(&status); 
        if(done == -1)
        {
            if(errno == ECHILD)
            {
                break;
            }
        }
        else
        {
            if(!WIFEXITED(status) || WEXITSTATUS(status) != 0)
            {
                std::cerr<<"pid "<<done<<" failed: "<<status<<std::endl;
                break;
            }
        }
    }
}

void Accept::MultiRun()
{

}

Accept::~Accept()
{

}

/* SingleAccept Class */
/*
SingleAccept::SingleAccept()
{

}

SingleAccept::~SingleAccept()
{

}
*/

/**
 *  Round Robin 
[Process ID: 17566] Connection Client
[Process ID: 17567] Connection Client
[Process ID: 17568] Connection Client
[Process ID: 17569] Connection Client
[Process ID: 17570] Connection Client
[Process ID: 17566] Connection Client
[Process ID: 17567] Connection Client
[Process ID: 17568] Connection Client
[Process ID: 17569] Connection Client
[Process ID: 17570] Connection Client
*/
/*
void SingleAccept::Run()
{
    Net::SockAddr *addr = new Net::SockAddr(PORT);
    Net::TcpSocket *socket = new Net::TcpSocket(addr, EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR);

    if(socket->CreateSocket() == C_ERR)
    {
        std::cerr<<"Failed CreateSocket(): "<<errno<<std::endl;
        return;
    }

    if(socket->BindSocket() == C_ERR)
    {
        std::cerr<<"Failed BindSocket(): "<<errno<<std::endl;
        return;
    }

    if(socket->ListenSocket() == C_ERR)
    {
        std::cerr<<"Failed ListenSocket(): "<<errno<<std::endl;
        return;
    }

    int pipes[WORKER_PROCESS_NUM][2];

    for(int i=0;i<WORKER_PROCESS_NUM;i++)
    {
        pid_t pid = fork();
        if(pid < 0)
        {
            std::cerr<<"fork() Error"<<std::endl;
            return;
        }

        else if(pid == 0)
        {
            while(true)
            {
                Net::TcpSocket *client = socket->AcceptSocket();
                std::cout<<"[Process ID: "<<getpid()<<"] "<<"Connection Client"<<std::endl;
                delete client;
            }
        }

        else
        {
            continue;
        }
    }

    WaitProcess();
}
*/

/* SingleEpollAccept Class*/
SingleEpollAccept::SingleEpollAccept()
{
    
}

SingleEpollAccept::~SingleEpollAccept()
{

}

void SingleEpollAccept::Run()
{
    Net::SockAddr *addr = new Net::SockAddr(PORT);
    Net::TcpSocket *socket = new Net::TcpSocket(addr, EPOLLEXCLUSIVE | EPOLLIN);

    if(socket->CreateSocket() == C_ERR)
    {
        std::cerr<<"Failed CreateSocket(): "<<errno<<std::endl;
        return;
    }

    if(socket->BindSocket() == C_ERR)
    {
        std::cerr<<"Failed BindSocket(): "<<errno<<std::endl;
        return;
    }

    if(socket->ListenSocket() == C_ERR)
    {
        std::cerr<<"Failed ListenSocket(): "<<errno<<std::endl;
        return;
    }

    if(socket->SetNonBlock() == C_ERR)
    {
        std::cerr<<"Failed SetNonBlock(): "<<errno<<std::endl;
        return;
    }

    /* 부모프로세스와 통신을 위한 파이프라인 구성 */
    int pipefd[WORKER_PROCESS_NUM][2];
    /* 프로세스 번호 저장 배열 */
    pid_t process_list[WORKER_PROCESS_NUM] = {0};

    for(int i=0;i<WORKER_PROCESS_NUM;i++)
    {
        if(pipe(pipefd[i]) == -1)
        {
            std::cout<<"Failed pipe(): "<<errno<<std::endl;
            continue;
        }

        pid_t pid = fork();
	    process_list[i] = pid; 

        if(pid < 0)
        {
            std::cerr<<"fork() Error"<<std::endl;
            return;
        }

        /* Child */
        else if(pid == 0)
        {
            int pi = i;
            Epoll::EventLoop el;
            if(el.CreateEventLoop() == C_ERR)
            {
                std::cerr<<"Failed CreateEventLoop(): "<<errno<<std::endl;
                delete socket;
                return;
            }

            if(el.AddEvent(socket) == C_ERR)
            {
                std::cerr<<"Failed AddEvent(): "<<errno<<std::endl;
                delete socket;
                return;
            }

            std::cout<<"Created Process: "<<getpid()<<std::endl;
            while(true)
            {
                int retval = el.FetchEvent();
                for(int i=0;i<retval;i++)
                {
                    Epoll::ev_t e = el.fired[i];
                    Net::Socket *socket = el.LoadSocket(e.fd);

                    if(e.mask != EPOLLIN)
                    {
                        el.DelEvent(socket);
                        continue;
                    }

                    switch(socket->type)
                    {
                        case SERVER:
                        {
                            Net::TcpSocket *client = ((Net::TcpSocket*)socket)->AcceptSocket();
                            if(client == nullptr)
                            {
                                //std::cout<<"[Process ID: "<<getpid()<<"] "<<"Fake Wakeup"<<std::endl;
                                continue;
                            }
                            //std::cout<<"[Process ID: "<<getpid()<<"] "<<"Connection Client"<<std::endl;
                            write(pipefd[pi][1], &pi, sizeof(uint32_t));
                            delete client;
                            break;
                            
                        }

                        case CLIENT:
                        {
                            break;
                        }
                    }
                }

                if(retval > 0)
                {
                    delete []el.fired;
                }
            }
        }

        else
        {
            continue;
        }
    }

    /* 자식프로세스가 전달하는 메시지 받기 위한 EPOLL 인스턴스 생성 */
    int epfd = epoll_create(1024);

    for(int i=0;i<WORKER_PROCESS_NUM;i++)
    {
        struct epoll_event events; 
        events.events = EPOLLIN;
        events.data.fd = pipefd[i][0];

        if(epoll_ctl(epfd, EPOLL_CTL_ADD, pipefd[i][0], &events) < 0)
        {
            continue;
        }
    }

    struct epoll_event epoll_events[1024];
    int state[WORKER_PROCESS_NUM] = {0};
    /**
     * 클라이언트의 커넥션 100번만 진행하고 자식프로세스 강제 종료
    */
    int index = 0;
    while(true)
    {
        int ret = epoll_wait(epfd, epoll_events, 1024, -1);
        for(int i=0;i<ret;i++)
        {
            uint32_t buffer;
            read(epoll_events[i].data.fd, &buffer, sizeof(uint32_t));
            state[buffer] += 1;
            index += 1; 
        }
        if(index >= 100)
        {
            break;
        }
    }

    /**
     * 특정 프로세스가 얼마만큼 클라이언트를 ACCEPT했는지 표시
    */
    for(int i=0;i<WORKER_PROCESS_NUM;i++)
    {
        std::cout<<"["<<process_list[i]<<"] PROCESS NUM OF WAKEUP: "<<state[i]<<std::endl;
	    kill(process_list[i], SIGTERM);
    }

    WaitProcess();
}

/*
MultipleAccept::MultipleAccept()
{

}

MultipleAccept::~MultipleAccept()
{

}

void MultipleAccept::Run()
{
    for(int i=0;i<WORKER_PROCESS_NUM;i++)
    {
        pid_t pid = fork();
        if(pid < 0)
        {
            std::cerr<<"fork() Error"<<std::endl;
            return;
        }

        else if(pid == 0)
        {
            Net::SockAddr *addr = new Net::SockAddr(PORT);
            Net::TcpSocket *socket = new Net::TcpSocket(addr, EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR);
        
            if(socket->CreateSocket() == C_ERR)
            {
                std::cerr<<"Failed CreateSocket(): "<<errno<<std::endl;
                exit(0);
            }

            if(socket->SetSocketReusePort() == C_ERR)
            {
                std::cerr<<"Failed SetSocketReusePort(): "<<errno<<std::endl;
                exit(0);
            }

            if(socket->BindSocket() == C_ERR)
            {
                std::cerr<<"Failed BindSocket(): "<<errno<<std::endl;
                exit(0);
            }

            if(socket->ListenSocket() == C_ERR)
            {
                std::cerr<<"Failed ListenSocket(): "<<errno<<std::endl;
                exit(0);
            }

            while(true)
            {
                Net::TcpSocket *cs = ((Net::TcpSocket*)socket)->AcceptSocket();
                if(cs != nullptr)
                {
                    std::cout<<"[Process ID: "<<getpid()<<"] "<<"Connection Client"<<std::endl;
                    delete cs;
                }
            }
        }

        else
        {
            continue;
        }
    }

    WaitProcess();
}

MultipleEpollAccept::MultipleEpollAccept()
{

}

MultipleEpollAccept::~MultipleEpollAccept()
{

}

void MultipleEpollAccept::Run()
{
    for(int i=0;i<WORKER_PROCESS_NUM;i++)
    {
        pid_t pid = fork();
        if(pid < 0)
        {
            std::cerr<<"fork() Error"<<std::endl;
            return;
        }

        else if(pid == 0)
        {
            Net::SockAddr *addr = new Net::SockAddr(PORT);
            Net::TcpSocket *socket = new Net::TcpSocket(addr, EPOLLET | EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR);
        
            if(socket->CreateSocket() == C_ERR)
            {
                std::cerr<<"Failed CreateSocket(): "<<errno<<std::endl;
                exit(0);
            }

            if(socket->SetSocketReusePort() == C_ERR)
            {
                std::cerr<<"Failed SetSocketReusePort(): "<<errno<<std::endl;
                exit(0);
            }

            if(socket->BindSocket() == C_ERR)
            {
                std::cerr<<"Failed BindSocket(): "<<errno<<std::endl;
                exit(0);
            }

            if(socket->ListenSocket() == C_ERR)
            {
                std::cerr<<"Failed ListenSocket(): "<<errno<<std::endl;
                exit(0);
            }

            if(el.CreateEventLoop() == C_ERR)
            {
                std::cerr<<"Failed CreateEventLoop(): "<<errno<<std::endl;
                exit(0);
            }

            if(el.AddEvent(socket) == C_ERR)
            {
                std::cerr<<"Failed AddEvent(): "<<errno<<std::endl;
                exit(0);
            }

            while(true)
            {
                int retval = el.FetchEvent();
                for(int i=0;i<retval;i++)
                {
                    Epoll::ev_t e = el.fired[i];
                    Net::Socket *socket = el.LoadSocket(e.fd);

                    if(e.mask != EPOLLIN)
                    {
                        el.DelEvent(socket);
                        continue;
                    }

                    switch(socket->type)
                    {
                        case SERVER:
                        {
                            Net::TcpSocket *client = ((Net::TcpSocket*)socket)->AcceptSocket();
                            std::cout<<"[Process ID: "<<getpid()<<"] "<<"Connection Client"<<std::endl;
                            delete client;
                            break;
                        }

                        case CLIENT:
                        {
                            break;
                        }
                    }
                }

                if(retval > 0)
                {
                    delete []el.fired;
                }
            }
        }

        else
        {
            continue;
        }
    }

    WaitProcess();
}
*/