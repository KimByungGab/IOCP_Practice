#pragma once

#include "Packet.h"

#include <string>

#define PACKET_DATA_BUFFER_SIZE 8096	// 패킷 버퍼 사이즈

class User
{
public:
	// 위치 정보
	enum class DOMAIN_STATE
	{
		NONE,	// 로비
		ROOM	// 방
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
	INT32 mIndex = -1;			// 세션에 걸려있는 클라이언트 인덱스
	INT32 mRoomIndex = -1;		// 유저가 현재 있는 방 인덱스

	DOMAIN_STATE mCurrentDomainState = DOMAIN_STATE::NONE;		// 현재 위치

	UINT32 mPacketDataBufferWPos = 0;		// 패킷 데이터 읽은 위치(어디까지 읽었는지)
	UINT32 mPacketDataBufferRPos = 0;		// 패킷 데이터 쓴 위치(어디까지 데이터가 들어왔는지)
	char mPacketDataBuffer[PACKET_DATA_BUFFER_SIZE] = { 0, };		// 패킷 버퍼
};

