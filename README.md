# epoll_question
동일한 listener socket을 공유하는 다수의 워커 프로세스가 각 각의 epoll 인스턴스를 만들고 accept 시도시 로드밸런싱이 잘 되는가 확인

## EPOLL 서버 실행 
이때 9988포트가 활성화되게 됩니다.   
만약 포트번호 변경을 원할 시 main.hpp에서 PORT 매크로값을 변경하시면 됩니다.
```Shell
git clone https://github.com/txuna/epoll_question.git
cd epoll_question
make 
./server 2 
```

## 클라이언트 실행 
```SHELL
python3 client.py  
```

## Result

```SHELL
$ ./server 2
```

```SHELL
$ python3 client.py
```

```SHELL
Created Process: 95774
Created Process: 95776
Created Process: 95775
Created Process: 95777
Created Process: 95778
Created Process: 95779
Created Process: 95788
Created Process: 95784
Created Process: 95785
Created Process: 95791
Created Process: 95792
Created Process: 95793
Created Process: 95790
Created Process: 95780
Created Process: 95786
Created Process: 95783
Created Process: 95781
Created Process: 95782
Created Process: 95787
Created Process: 95789
[95774] PROCESS NUM OF WAKEUP: 91
[95775] PROCESS NUM OF WAKEUP: 0
[95776] PROCESS NUM OF WAKEUP: 9
[95777] PROCESS NUM OF WAKEUP: 0
[95778] PROCESS NUM OF WAKEUP: 0
[95779] PROCESS NUM OF WAKEUP: 0
[95780] PROCESS NUM OF WAKEUP: 0
[95781] PROCESS NUM OF WAKEUP: 0
[95782] PROCESS NUM OF WAKEUP: 0
[95783] PROCESS NUM OF WAKEUP: 0
[95784] PROCESS NUM OF WAKEUP: 0
[95785] PROCESS NUM OF WAKEUP: 0
[95786] PROCESS NUM OF WAKEUP: 0
[95787] PROCESS NUM OF WAKEUP: 0
[95788] PROCESS NUM OF WAKEUP: 0
[95789] PROCESS NUM OF WAKEUP: 0
[95790] PROCESS NUM OF WAKEUP: 0
[95791] PROCESS NUM OF WAKEUP: 0
[95792] PROCESS NUM OF WAKEUP: 0
[95793] PROCESS NUM OF WAKEUP: 0
```
95774 PID를 가진 프로세스만 WAKEUP되어 클라이언트의 연결 요청을 처리하는 것을 볼 수 있다. 