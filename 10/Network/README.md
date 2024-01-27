# 네트워크 관련 파일

* ClientInfo: 클라이언트 정보 클래스
* NetworkCore: 네트워크의 전반적인 코어 코드 클래스(윈속 초기화, bind, listen, AcceptEx, WSASend, WSARecv 등등)
* NetworkDefine.h: 네트워크를 할 때 필요한 기본적인 include 및 미리 정의가 필요한 구조체, enum
* NetworkServer: main 파일에서 NetworkCore를 작동시키기 위한 클래스
* Packet.h: 패킷 정보 파일. (서버에서 사용할 패킷 정보 구조체, 패킷 ID enum 클래스, 패킷 헤더, 패킷 바디)
* PacketManager: 패킷을 실질적으로 만질 수 있는 매니저 클래스.
