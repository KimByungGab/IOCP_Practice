# 네트워크 관련 파일

* ClientInfo: 클라이언트 정보 클래스
* NetworkCore: 네트워크의 전반적인 코어 코드 클래스(윈속 초기화, bind, listen, AcceptEx, WSASend, WSARecv 등등)
* NetworkDefine.h: 네트워크를 할 때 필요한 기본적인 include 및 미리 정의가 필요한 구조체, enum
* NetworkServer: main 파일에서 NetworkCore를 작동시키기 위한 클래스
* Packet: 패킷 클래스. 현재는 Send만 담당하고 있다.
