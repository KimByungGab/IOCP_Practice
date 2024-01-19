#include "ClientInfo.h"

// mClientInfos(Ŭ�� vector)�� �� Ŭ�� ���� �ʱ�ȭ
ClientInfo::ClientInfo()
{
	// Overlapped I/O ����ü �ʱ�ȭ
	memset(&m_stRecvOverlappedEx, 0, sizeof(stOverlappedEx));
	memset(&m_stSendOverlappedEx, 0, sizeof(stOverlappedEx));

	// ���� �ʱ�ȭ
	mSocket = INVALID_SOCKET;

	// recv�� send ���� �ʱ�ȭ
	memset(mRecvBuf, 0, sizeof(mRecvBuf));
	memset(mSendBuf, 0, sizeof(mSendBuf));
}

// �ε��� ��ȯ
unsigned int ClientInfo::GetIndex()
{
	return mIndex;
}

// �ε��� ����
void ClientInfo::SetIndex(const unsigned int index)
{
	mIndex = index;
}

// ���� ���� Ȯ��
bool ClientInfo::IsConnected()
{
	return mSocket != INVALID_SOCKET;
}

// Ŭ�� ���� ��ȯ
SOCKET ClientInfo::GetSocket()
{
	return mSocket;
}

// Ŭ�� ���� ����
void ClientInfo::SetSocket(SOCKET socket)
{
	mSocket = socket;
}

// ���� �ּҰ�(�迭) ��ȯ
char* ClientInfo::GetRecvBuffer()
{
	return mRecvBuf;
}

// ����� ���� �뵵�� Ȯ���� �� ����.
// �Ƹ� �ʱ�ȭ�� ������ �ƴұ� �ͱ� �ѵ� ���� �������� �Ǵ��Ͽ� ���ܵд�.
void ClientInfo::Clear()
{
}