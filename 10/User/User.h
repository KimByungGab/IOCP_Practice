#pragma once

#include "Packet.h"

#include <string>

#define PACKET_DATA_BUFFER_SIZE 8096	// ��Ŷ ���� ������

class User
{
public:
	// ��ġ ����
	enum class DOMAIN_STATE
	{
		NONE,	// �κ�
		ROOM	// ��
	};

	User() = default;
	~User() = default;

	void Init(const INT32 index);
	void Clear();
	void EnterLobby();
	void EnterRoom(INT32 roomIndex);
	DOMAIN_STATE GetDomainState();
	void SetDomainState(DOMAIN_STATE value);
	INT32 GetCurrentRoom();
	INT32 GetNetConnIdx();
	PacketInfo GetPacket();
	void SetPacketData(const UINT32 dataSize, char* pData);

private:
	INT32 mIndex = -1;			// ���ǿ� �ɷ��ִ� Ŭ���̾�Ʈ �ε���
	INT32 mRoomIndex = -1;		// ������ ���� �ִ� �� �ε���

	DOMAIN_STATE mCurrentDomainState = DOMAIN_STATE::NONE;		// ���� ��ġ

	UINT32 mPacketDataBufferWPos = 0;		// ��Ŷ ������ ���� ��ġ(������ �о�����)
	UINT32 mPacketDataBufferRPos = 0;		// ��Ŷ ������ �� ��ġ(������ �����Ͱ� ���Դ���)
	char mPacketDataBuffer[PACKET_DATA_BUFFER_SIZE] = { 0, };		// ��Ŷ ����
};

