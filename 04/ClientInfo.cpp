#include "ClientInfo.h"

// mClientInfos(Ŭ�� vector)�� �� Ŭ�� ���� �ʱ�ȭ
ClientInfo::ClientInfo(const unsigned int index)
{
	// Overlapped I/O ����ü �ʱ�ȭ
	memset(&m_stRecvOverlappedEx, 0, sizeof(stOverlappedEx));
	memset(&m_stSendOverlappedEx, 0, sizeof(stOverlappedEx));

	// ���� �ʱ�ȭ
	mSocket = INVALID_SOCKET;

	// �ε��� ����
	mIndex = index;

	// recv�� send ���� �ʱ�ȭ
	memset(mRecvBuf, 0, sizeof(mRecvBuf));
	memset(mSendBuf, 0, sizeof(mSendBuf));
}

// �ε��� ��ȯ
unsigned int ClientInfo::GetIndex()
{
	return mIndex;
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

void ClientInfo::SetSocket(SOCKET socket)
{
	mSocket = socket;
}

// ���� �ּҰ�(�迭) ��ȯ
char* ClientInfo::GetRecvBuffer()
{
	return mRecvBuf;
}

// Ŭ���̾�Ʈ ����
void ClientInfo::Close(bool bIsForce)
{
	struct linger stLinger = { 0, 0 };		// SO_DONTLINGER�� ������

	// bIsForce�� true�� SO_LINGER, timeout = 0���� �����ؼ� ���� ����. ������ �ս� �� �� ����
	if (bIsForce == true)
	{
		stLinger.l_onoff = 1;
	}

	// ������ send recv ���� �ߴ�
	shutdown(mSocket, SD_BOTH);

	// ���� �ɼ��� �����ص� linger�� ����
	setsockopt(mSocket, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

	// ���� ����
	closesocket(mSocket);

	// ������ �ʿ�������ٰ� ������� �����ϰ� ���ġ�ϴٰ� ���߿� ū���� �� �� ����.
	// ������ pool�� �뵵�� Ȱ���ϱ� ���Ͽ� ���ϸ� �׿���
	mSocket = INVALID_SOCKET;
}

// ����� ���� �뵵�� Ȯ���� �� ����.
// �Ƹ� �ʱ�ȭ�� ������ �ƴұ� �ͱ� �ѵ� ���� �������� �Ǵ��Ͽ� ���ܵд�.
void ClientInfo::Clear()
{
}