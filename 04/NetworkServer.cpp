#include "NetworkServer.h"

// ���� �̺�Ʈ
void NetworkServer::OnConnect(const unsigned int clientIndex)
{
	cout << "[OnConnect] �ε���: " << clientIndex << endl;
}

// ���� ���� �̺�Ʈ
void NetworkServer::OnClose(const unsigned int clientIndex)
{
	cout << "[OnClose] �ε���: " << clientIndex << endl;
}

// recv �̺�Ʈ
void NetworkServer::OnReceive(const unsigned int clientIndex, const unsigned int size, char* pData)
{
	cout << "[OnReceive] �ε���: " << clientIndex << " / ������ũ��: " << size;

	// ��Ŷ�� ���� �־ send ���� ť�� ����
	Packet packet;
	packet.Set(clientIndex, size, pData);

	lock_guard<mutex> guard(mLock);
	mPacketQueue.push_back(packet);
}

// ���� ����
void NetworkServer::Run()
{
	// send ������ Ȱ��ȭ
	mIsRunProcessThread = true;
	mProcessThread = thread([this]()
		{
			ProcessPacket();
		});

	StartServer();
}

// ���� ����
void NetworkServer::End()
{
	// send ������ ��Ȱ��ȭ
	mIsRunProcessThread = false;

	// send �����尡 �۵����̶�� ����
	if (mProcessThread.joinable())
		mProcessThread.join();

	// ��Ʈ��ũ ���� ������ ���� ����
	DestroyThread();
}

// ��Ŷ ���μ���
void NetworkServer::ProcessPacket()
{
	while (mIsRunProcessThread)
	{
		// ���� �ֱ��� ��Ŷ ���޹ޱ�
		Packet packet = DequePacketData();

		// ��Ŷ�� �����Ѵٸ�
		if (packet.mDataSize != 0)
		{
			SendMsg(GetClientInfo(packet.mSessionIndex), packet.m_pPacketData, packet.mDataSize);
		}
		else
		{
			this_thread::sleep_for(1ms);
		}
	}
}

// ��Ŷ ������ �ޱ�
Packet NetworkServer::DequePacketData()
{
	// ��Ŷ
	Packet packet;

	// ������ ����
	lock_guard<mutex> guard(mLock);

	// ��Ŷ ť�� ����ٸ�
	if (mPacketQueue.empty())
	{
		return packet;	// �� ��Ŷ �״�� ������
	}

	// ��Ŷ ���� ����
	packet.Set(mPacketQueue.front());

	// ����� ��Ŷ�� �ʿ�����Ƿ� ������
	mPacketQueue.front().Release();
	mPacketQueue.pop_front();

	return packet;
}
