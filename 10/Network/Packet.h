#pragma once

// Ư�� ��� ����, ���ʿ��� �ڵ� ���ŷ� ������ �ð��� ũ�� ����ȭ ��ũ��
// ���߿� �˾����� �̸� �����ϵ� ���(pch)�� �����ϸ� �ڵ������� ��ϵȴ�.
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>

using namespace std;

#define MAX_CHAT_BUFFER 512

// �������� ���� ��Ŷ ����
struct PacketInfo
{
	UINT32 ClientIndex = 0;
	UINT16 PacketId = 0;
	UINT16 DataSize = 0;
	char* pDataPtr = nullptr;
};

// ��Ŷ ID
// ��� �ƽ����� �����ִ�.
// ������ ������ �̷��� ���� �� ���� ���߿� ���� �Ǹ� �ʹ� ������ ���̴�.
// ���Ϸ� ��� �з��� �ʿ��� �Ŷ�� �����ȴ�.
// �׷��ٸ� Ŭ������ ���� �ľ� �� ���̴�. �׷��� switch�� ������ �ϵ� �Ȱ��� �Լ������ͷ� ���� �� �ش� �Լ����� Ŭ������ �̵��ϰ� �ϸ� �� ���̴�.
// ���� �޸� ����� �������� ���� ��ŭ �� �� ������ �ʿ��غ��δ�.
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

#pragma pack(push, 1)		// ���⼭���ʹ� ��Ŷ �����̱� ������ ��Ƣ��Ǵ� �뷮�� ������ ����

/*-----------------------
		��Ŷ ���
------------------------*/
struct PACKET_HEADER
{
	UINT16 PacketId;
	UINT16 PacketLength;
};

const UINT32 PACKET_HEADER_LENGTH = sizeof(PACKET_HEADER);		// ������ ������ ���� ���ɰ��̶� �Ǵ��Ͽ� ���ȭ

/*-----------------------
		��Ŷ �ٵ�
------------------------*/

// ���� ����
struct SYS_USER_CONNECT : public PACKET_HEADER
{
	char welcomeMessage[MAX_CHAT_BUFFER];
};

// ���� �������� ��û
struct SYS_USER_DISCONNECT_REQUEST_PACKET : public PACKET_HEADER
{
};

// ���� �������� ����
struct SYS_USER_DISCONNECT_RESPONSE_PACKET : public PACKET_HEADER
{
};

// ����
struct SYS_ERROR : public PACKET_HEADER
{
	unsigned short errorCode;
};

// �� ���� ��û
struct ROOM_CREATE_REQUEST_PACKET : public PACKET_HEADER
{
};

// �� ���� ����
struct ROOM_CREATE_RESPONSE_PACKET : public PACKET_HEADER
{
	UINT32 RoomIndex;
};

// �� ���� ��û
struct ROOM_ENTER_REQUEST_PACKET : public PACKET_HEADER
{
	UINT32 RoomIndex;
};

// �� ���� ����
struct ROOM_ENTER_RESPONSE_PACKET : public PACKET_HEADER
{
};

// �� ���� ��û
struct ROOM_LEAVE_REQUEST_PACKET : public PACKET_HEADER
{
	UINT32 RoomIndex;
};

// �� ���� ����
struct ROOM_LEAVE_RESPONSE_PACKET : public PACKET_HEADER
{
};

// ä�� ��û
struct ROOM_CHAT_REQUEST_PACKET : public PACKET_HEADER
{
	UINT32 RoomIndex;
	char chatData[MAX_CHAT_BUFFER];
};

// ä�� ����
struct ROOM_CHAT_RESPONSE_PACKET : public PACKET_HEADER
{
	char chatData[MAX_CHAT_BUFFER];
};

#pragma pack(pop)		// �뷮 ���� ����