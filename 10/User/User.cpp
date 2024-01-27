#include "User.h"

enum class User::DOMAIN_STATE;

// 세션 첫 초기화 (유저 매니저가 만들어질때만)
void User::Init(const INT32 index)
{
	mIndex = index;
}

// 세션 초기화
void User::Clear()
{
	mRoomIndex = -1;
	mCurrentDomainState = DOMAIN_STATE::NONE;

	mPacketDataBufferWPos = 0;
	mPacketDataBufferRPos = 0;
}

// 로비로 이동
void User::EnterLobby()
{
	mRoomIndex = -1;
	mCurrentDomainState = DOMAIN_STATE::NONE;
}

// 방으로 이동
void User::EnterRoom(INT32 roomIndex)
{
	mRoomIndex = roomIndex;
	mCurrentDomainState = DOMAIN_STATE::ROOM;
}

// 현재 위치 
User::DOMAIN_STATE User::GetDomainState()
{
	return mCurrentDomainState;
}

// 현재 위치 세팅
void User::SetDomainState(DOMAIN_STATE value)
{
	mCurrentDomainState = value;
}

// 현재 방
INT32 User::GetCurrentRoom()
{
	return mRoomIndex;
}

// 현재 접속 클라이언트 idx
INT32 User::GetNetConnIdx()
{
	return mIndex;
}

// 패킷 가져오기
PacketInfo User::GetPacket()
{
	// 남은 바이트 계산
	UINT32 remainByte = mPacketDataBufferWPos - mPacketDataBufferRPos;

	// 패킷의 총 바이트가 헤더보다 적으면 완전히 다 안 옴
	if (remainByte < PACKET_HEADER_LENGTH)
		return PacketInfo();

	// 가장 최근에 읽었던 자리로부터 패킷 헤더 형태로 읽기
	auto pHeader = (PACKET_HEADER*)&mPacketDataBuffer[mPacketDataBufferRPos];

	// 패킷의 길이가 총 바이트보다 길다면 완전히 다 안 옴
	if (pHeader->PacketLength > remainByte)
		return PacketInfo();

	// 패킷 정보 넣기
	PacketInfo packetInfo;
	packetInfo.PacketId = pHeader->PacketId;
	packetInfo.DataSize = pHeader->PacketLength;
	packetInfo.pDataPtr = &mPacketDataBuffer[mPacketDataBufferRPos];

	// 읽은 패킷 길이만큼 위치 이동
	mPacketDataBufferRPos += pHeader->PacketLength;

	return packetInfo;
}

// 패킷 넣기
void User::SetPacketData(const UINT32 dataSize, char* pData)
{
	// 만약 쓰여진 패킷보다에 패킷 사이즈까지 더했는데 총 버퍼양보다 크다면? 즉, 과포화 될 사이즈라면
	if ((mPacketDataBufferWPos + dataSize) >= PACKET_DATA_BUFFER_SIZE)
	{
		// 남은 데이터 계산
		auto remainDataSize = mPacketDataBufferWPos - mPacketDataBufferRPos;

		// 아직 읽지 않은 데이터가 있다면
		if (remainDataSize > 0)
		{
			// 남은 데이터를 아예 맨 처음으로 옮김
			memcpy(&mPacketDataBuffer[0], &mPacketDataBuffer[mPacketDataBufferRPos], remainDataSize);

			// 데이터 사이즈만큼 쓰기 위치 이동
			mPacketDataBufferWPos = remainDataSize;
		}
		else
		{
			// 남은 데이터가 깔끔하게 없으니까 처음으로 이동
			mPacketDataBufferWPos = 0;
		}

		// 데이터가 있든 없든 처음으로 이동
		// 데이터가 있으면 데이터를 다시 처음으로 이동해서 썼기 때문에 0으로 이동하는것이고
		// 없으면 없는대로 쓰기 위치를 다시 앞으로 옮겼기 때문에 이동
		mPacketDataBufferRPos = 0;

		/*
		* 이렇게 만들어진 이유
		* 
		* 해당 패킷의 버퍼는 링버퍼(환형 큐)가 아니다.
		* 즉, 결국 언젠가는 버퍼에 데이터를 쌓다보면 끝에 도달하기 마련이고 아무런 조치를 하지 않으면 결국 오버플로우가 되고 말 것이다.
		* 때문에 언젠가 한번은 다시 처음쪽으로 옮겨줘야 할 필요가 있다는 것이다.
		* 그렇기 때문에 터지기 일보직전인 타이밍에 남은 데이터를 다시 처음으로 다 옮기고 세팅을 마친 이후에 다시 버퍼를 써나가는 것이다.
		* 
		* 간단하게
		* 터지기 일보직전 -> 아직 다 못 읽은 데이터들 다 처음으로 이동 -> 패킷 넣기
		* 를 반복한다고 보면 된다.
		*/
	}

	// 패킷 쓰여진 자리부터 메모리 복사
	memcpy(&mPacketDataBuffer[mPacketDataBufferWPos], pData, dataSize);

	// 쓰여진 데이터 사이즈만큼 위치 이동
	mPacketDataBufferWPos += dataSize;
}
