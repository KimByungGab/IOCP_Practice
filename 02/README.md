## 변경된 점
기존의 stOverlappedEx의 Overlapped 확장 구조체에서 send와 recv 용으로 활용되었던 m_szBuf를 ClientInfo 클래스의 send버퍼와 recv버퍼로 나누었다.

## 내 생각
가장 큰 장점은 통신을 할 때마다 만들어지는 Overlapped 구조체에서 m_szBuf만큼의 리소스가 할당이 되어야했던 것을 Client에 버퍼들로 옮겨놓았기 때문에 Client가 서비스를 종료하거나 에러가 발생하여 소켓 연결이 종료되지 않는다면 계속 버퍼를 활용할 수 있게 된다.
즉 리소스의 유지가 가능해진다.

그리고 개인적이지만 두 번째의 장점은 Overlapped는 나는 말 그대로 socket 통신을 하면서 받고 줄 때의 정보 용도로만 활용을 하는 것이 좋다고 생각한다. 즉, 관심사 분리다.
