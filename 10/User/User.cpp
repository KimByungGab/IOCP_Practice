#include "User.h"

enum class User::DOMAIN_STATE;

// ���� ù �ʱ�ȭ (���� �Ŵ����� �����������)
void User::Init(const INT32 index)
{
	mIndex = index;
}

// ���� �ʱ�ȭ
void User::Clear()
{
	mRoomIndex = -1;
	mCurrentDomainState = DOMAIN_STATE::NONE;

	mPacketDataBufferWPos = 0;
	mPacketDataBufferRPos = 0;
}

// �κ�� �̵�
void User::EnterLobby()
{
	mRoomIndex = -1;
	mCurrentDomainState = DOMAIN_STATE::NONE;
}

// ������ �̵�
void User::EnterRoom(INT32 roomIndex)
{
	mRoomIndex = roomIndex;
	mCurrentDomainState = DOMAIN_STATE::ROOM;
}

// ���� ��ġ 
User::DOMAIN_STATE User::GetDomainState()
{
	return mCurrentDomainState;
}

// ���� ��ġ ����
void User::SetDomainState(DOMAIN_STATE value)
{
	mCurrentDomainState = value;
}

// ���� ��
INT32 User::GetCurrentRoom()
{
	return mRoomIndex;
}

// ���� ���� Ŭ���̾�Ʈ idx
INT32 User::GetNetConnIdx()
{
	return mIndex;
}

// ��Ŷ ��������
PacketInfo User::GetPacket()
{
	// ���� ����Ʈ ���
	UINT32 remainByte = mPacketDataBufferWPos - mPacketDataBufferRPos;

	// ��Ŷ�� �� ����Ʈ�� ������� ������ ������ �� �� ��
	if (remainByte < PACKET_HEADER_LENGTH)
		return PacketInfo();

	// ���� �ֱٿ� �о��� �ڸ��κ��� ��Ŷ ��� ���·� �б�
	auto pHeader = (PACKET_HEADER*)&mPacketDataBuffer[mPacketDataBufferRPos];

	// ��Ŷ�� ���̰� �� ����Ʈ���� ��ٸ� ������ �� �� ��
	if (pHeader->PacketLength > remainByte)
		return PacketInfo();

	// ��Ŷ ���� �ֱ�
	PacketInfo packetInfo;
	packetInfo.PacketId = pHeader->PacketId;
	packetInfo.DataSize = pHeader->PacketLength;
	packetInfo.pDataPtr = &mPacketDataBuffer[mPacketDataBufferRPos];

	// ���� ��Ŷ ���̸�ŭ ��ġ �̵�
	mPacketDataBufferRPos += pHeader->PacketLength;

	return packetInfo;
}

// ��Ŷ �ֱ�
void User::SetPacketData(const UINT32 dataSize, char* pData)
{
	// ���� ������ ��Ŷ���ٿ� ��Ŷ ��������� ���ߴµ� �� ���۾纸�� ũ�ٸ�? ��, ����ȭ �� ��������
	if ((mPacketDataBufferWPos + dataSize) >= PACKET_DATA_BUFFER_SIZE)
	{
		// ���� ������ ���
		auto remainDataSize = mPacketDataBufferWPos - mPacketDataBufferRPos;

		// ���� ���� ���� �����Ͱ� �ִٸ�
		if (remainDataSize > 0)
		{
			// ���� �����͸� �ƿ� �� ó������ �ű�
			memcpy(&mPacketDataBuffer[0], &mPacketDataBuffer[mPacketDataBufferRPos], remainDataSize);

			// ������ �����ŭ ���� ��ġ �̵�
			mPacketDataBufferWPos = remainDataSize;
		}
		else
		{
			// ���� �����Ͱ� ����ϰ� �����ϱ� ó������ �̵�
			mPacketDataBufferWPos = 0;
		}

		// �����Ͱ� �ֵ� ���� ó������ �̵�
		// �����Ͱ� ������ �����͸� �ٽ� ó������ �̵��ؼ� ��� ������ 0���� �̵��ϴ°��̰�
		// ������ ���´�� ���� ��ġ�� �ٽ� ������ �Ű�� ������ �̵�
		mPacketDataBufferRPos = 0;

		/*
		* �̷��� ������� ����
		* 
		* �ش� ��Ŷ�� ���۴� ������(ȯ�� ť)�� �ƴϴ�.
		* ��, �ᱹ �������� ���ۿ� �����͸� �״ٺ��� ���� �����ϱ� �����̰� �ƹ��� ��ġ�� ���� ������ �ᱹ �����÷ο찡 �ǰ� �� ���̴�.
		* ������ ������ �ѹ��� �ٽ� ó�������� �Ű���� �� �ʿ䰡 �ִٴ� ���̴�.
		* �׷��� ������ ������ �Ϻ������� Ÿ�ֿ̹� ���� �����͸� �ٽ� ó������ �� �ű�� ������ ��ģ ���Ŀ� �ٽ� ���۸� �ᳪ���� ���̴�.
		* 
		* �����ϰ�
		* ������ �Ϻ����� -> ���� �� �� ���� �����͵� �� ó������ �̵� -> ��Ŷ �ֱ�
		* �� �ݺ��Ѵٰ� ���� �ȴ�.
		*/
	}

	// ��Ŷ ������ �ڸ����� �޸� ����
	memcpy(&mPacketDataBuffer[mPacketDataBufferWPos], pData, dataSize);

	// ������ ������ �����ŭ ��ġ �̵�
	mPacketDataBufferWPos += dataSize;
}
