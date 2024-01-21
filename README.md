# IOCP_Practice
IOCP 단계별 연습

## Thanks for
기본적인 단계별 연습을 제공해주신 '최흥배'님께 감사의 말씀을 드립니다.
- 참고한 IOCP 단계별 연습과정: https://github.com/jacking75/edu_cpp_IOCP/tree/master

## Repository 소개
본 Repository는 IOCP의 단계적인 실습에서 저의 코딩 습관을 추가하여 일부 구조는 변경하였으며 그에 따라 변수명도 약간의 변경이 있습니다.
그리고 하나하나 구조를 익히기 위해 주석을 최대한 많이 달아놓으면서 복습하는 개념의 Repository입니다.

## 실습 단계 (구현한 것들만 나열)
1. 가장 간단한 IOCP 서버. Echo 서버 코드 만들기
    * IOCP API 동작에 대해서 이해할 수 있다.
2. OverlappedEx 구조체에 있는 char m_szBuf[ MAX_SOCKBUF ]를 stClientInfo 구조체로 이동 및 코드 분리하기
    * 앞 단계에는 OverlappedEx 구조체에 m_szBuf가 있어서 불 필요한 메모리 낭비가 발생함
3. 애플리케이션과 네트워크 코드 분리하기
    * IOCP를 네트워크 라이브러리화 하기 위해 네트워크와 서버 로직 코드를 분리한다.
    * 연결, 끊어짐, 데이터 받음을 애플리케이션에 전달하기
4. 네트워크와 로직(패킷 or 요청) 처리 각각의 스레드로 분리하기
    * Send를 Recv와 다른 스레드에서 하기
    * send를 연속으로 보낼 수 있는 구조가 되어야 한다.
5. Send 구현 (실습 6단계)
    * queue에 담아 놓고 순차적으로 보내기.
6. Accept -> AcceptEx로 전환 (실습 7단계)
    * 6단계까지는 Accept 처리를 동기 I/O로 했다. 이것을 비동기I/O로 바꾼다. 이로써 네트워크 동작이 모두 비동기 I/O가 된다.
