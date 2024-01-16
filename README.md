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
