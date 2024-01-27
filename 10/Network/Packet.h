#pragma once

// 특정 기능 제외, 불필요한 코드 제거로 컴파일 시간과 크기 최적화 매크로
// 나중에 알았지만 미리 컴파일된 헤더(pch)를 적용하면 자동적으로 등록된다.
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>

using namespace std;

#define MAX_CHAT_BUFFER 512

// 서버에서 쓰일 패킷 정보
struct PacketInfo
{
	UINT32 ClientIndex = 0;
	UINT16 PacketId = 0;
	UINT16 DataSize = 0;
	char* pDataPtr = nullptr;
};

// 패킷 ID
// 사실 아쉬움이 남아있다.
// 이유는 지금은 이렇게 적을 지 몰라도 나중에 가게 되면 너무 많아질 것이다.
// 파일로 찢어서 분류가 필요할 거라고 생각된다.
// 그렇다면 클래스로 새로 파야 될 것이다. 그렇게 switch로 진행을 하든 똑같이 함수포인터로 전송 후 해당 함수에서 클래스로 이동하게 하면 될 것이다.
// 웹과 달리 기능의 가짓수가 많은 만큼 좀 더 생각이 필요해보인다.
enum class PACKET_ID : UINT16
{
	// System
	SYS_END = 0,
	SYS_USER_CONNECT = 11,
	SYS_USER_DISCONNECT = 12,
	SYS_ERROR = 40,

	// Chat
	ROOM_CREATE_REQUEST = 301,
	ROOM_CREATE_RESPONSE = 302,

	ROOM_ENTER_REQUEST = 303,
	ROOM_ENTER_RESPONSE = 304,

	ROOM_LEAVE_REQUEST = 311,
	ROOM_LEAVE_RESPONSE = 312,

	ROOM_CHAT_REQUEST = 313,
	ROOM_CHAT_RESPONSE = 314
};

#pragma pack(push, 1)		// 여기서부터는 패킷 정보이기 때문에 뻥튀기되는 용량이 없도록 정리

/*-----------------------
		패킷 헤더
------------------------*/
struct PACKET_HEADER
{
	UINT16 PacketId;
	UINT16 PacketLength;
};

const UINT32 PACKET_HEADER_LENGTH = sizeof(PACKET_HEADER);		// 고정된 값으로 많이 사용될것이라 판단하여 상수화

/*-----------------------
		패킷 바디
------------------------*/

// 유저 접속
struct SYS_USER_CONNECT : public PACKET_HEADER
{
	char welcomeMessage[MAX_CHAT_BUFFER];
};

// 유저 접속종료 요청
struct SYS_USER_DISCONNECT_REQUEST_PACKET : public PACKET_HEADER
{
};

// 유저 접속종료 응답
struct SYS_USER_DISCONNECT_RESPONSE_PACKET : public PACKET_HEADER
{
};

// 에러
struct SYS_ERROR : public PACKET_HEADER
{
	unsigned short errorCode;
};

// 방 생성 요청
struct ROOM_CREATE_REQUEST_PACKET : public PACKET_HEADER
{
};

// 방 생성 응답
struct ROOM_CREATE_RESPONSE_PACKET : public PACKET_HEADER
{
	UINT32 RoomIndex;
};

// 방 입장 요청
struct ROOM_ENTER_REQUEST_PACKET : public PACKET_HEADER
{
	UINT32 RoomIndex;
};

// 방 입장 응답
struct ROOM_ENTER_RESPONSE_PACKET : public PACKET_HEADER
{
};

// 방 퇴장 요청
struct ROOM_LEAVE_REQUEST_PACKET : public PACKET_HEADER
{
	UINT32 RoomIndex;
};

// 방 퇴장 응답
struct ROOM_LEAVE_RESPONSE_PACKET : public PACKET_HEADER
{
};

// 채팅 요청
struct ROOM_CHAT_REQUEST_PACKET : public PACKET_HEADER
{
	UINT32 RoomIndex;
	char chatData[MAX_CHAT_BUFFER];
};

// 채팅 응답
struct ROOM_CHAT_RESPONSE_PACKET : public PACKET_HEADER
{
	char chatData[MAX_CHAT_BUFFER];
};

#pragma pack(pop)		// 용량 정리 종료