## 변경된 점
1. 기존에 AcceptThread에서 accept를 이용하여 동기로 받았지만 AcceptEx를 이용하여 비동기로 언제든 받을 수 있게 처리
2. 1번의 사항으로 WorkerThread에서 GetQueuedCompletionStatus를 이용하여 나온 Accept 코드 처리
3. 2번의 사항으로 작업 동작 종류(enum class IOOperation)에 ACCEPT 추가

## 내 생각
이제 개인적으로 생각할 때 모든 네트워크의 기능이 비동기로 이루어졌다고 생각한다.<br>
이 파일이 되기까지 정말 많은 기능들이 이루어진 것 같다.<br>
처음은 기본적인 서버 골자인 WSAStartup - bind - listen - recv - send - closesocket - WSACleanup를 만들었다.<br>
그 다음은 네트워크 라이브러리화 하기 위하여 NetworkDefine.h와 NetworkServer 클래스를 만들어 main에서 직접 NetworkCore 클래스에 관여를 하지 않게 되었다. 즉 분리를 한 것이다.<br>
세번째로는 성능의 향상을 위하여 accept, recv, send를 각각 AcceptEx, WSARecv, WSASend로 변경하여 GetQueuedCompletionStatus를 이용할 수 있게 만들었다. 즉 동기에서 비동기로 전환했다.

아직 ConnextEx와 DisConnectEx가 있기는 하지만 ConnectEx는 connect와 연관이 되어있는것이니 client를 다루는쪽이니 넘어가는 게 낫다고 판단했다.<br>
DisconnectEx의 장점은 두 가지라고 생각하는데
1. 연결 해제의 비동기화
2. 소켓의 재사용

이라고 생각한다. 물론 그것도 좋은 생각이라고 생각한다. 소켓을 계속 만들어가면서 사용하는 게 아닌 한번 만들어 둔 소켓을 계속 사용하는것이니만큼 정말 좋은 요소라고 생각한다. 추후에 달아놓도록 해야겠다.

이러나 저러나 기본구조는 끝난 것 같다.<br>
다음은 이 구조를 이용하여 채팅서버를 만들어 볼 생각이다.<br>
채팅서버를 만들게 된다면 본격적으로 패킷의 세분화가 필요할 것이라고 판단이 된다.
