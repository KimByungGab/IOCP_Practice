#include "ClientInfo.h"

// mClientInfos(Ŭ�� vector)�� �� Ŭ�� ���� �ʱ�ȭ
ClientInfo::ClientInfo()
{
	// Overlapped I/O ����ü �ʱ�ȭ
	memset(&m_stRecvOverlappedEx, 0, sizeof(stOverlappedEx));
	memset(&m_stSendOverlappedEx, 0, sizeof(stOverlappedEx));
	memset(&m_stAcceptOverlappedEx, 0, sizeof(stOverlappedEx));

	// ���� �ʱ�ȭ
	mSocket = INVALID_SOCKET;

	// recv�� send ���� �ʱ�ȭ
	memset(mRecvBuf, 0, sizeof(mRecvBuf));
	memset(mSendBuf, 0, sizeof(mSendBuf));
	memset(mAcceptBuf, 0, sizeof(mAcceptBuf));
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

// �ֱ� �������� �ð� ��ȯ
UINT64 ClientInfo::GetLatestClosedTimeSec()
{
	return mLatestClosedTimeSec;
}

// �ֱ� �������� �ð� ����
void ClientInfo::SetLatestClosedTimeSec(UINT64 timeSec)
{
	mLatestClosedTimeSec = timeSec;
}

// ���� ���� Ȯ��
bool ClientInfo::IsConnected()
{
	return mIsConnect;
}

// ���� ���� ����
void ClientInfo::SetConnect(bool isConnect)
{
	mIsConnect = isConnect;
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