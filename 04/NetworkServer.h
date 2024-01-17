#pragma once

#include "NetworkDefine.h"
#include "NetworkCore.h"
#include "Packet.h"

class NetworkServer : public NetworkCore
{
public:
	NetworkServer() = default;
	virtual ~NetworkServer() = default;

	virtual void OnConnect(const unsigned int clientIndex) override;
	virtual void OnClose(const unsigned int clientIndex) override;
	virtual void OnReceive(const unsigned int clientIndex, const unsigned int size, char* pData) override;

	void Run();
	void End();

private:
	void ProcessPacket();
	Packet DequePacketData();
	
private:
	bool mIsRunProcessThread = false;		// send ������ �ݺ��� ����
	thread mProcessThread;					// send ������
	mutex mLock;							// ��Ŷ ���� ���ؽ�
	deque<Packet> mPacketQueue;				// ��Ŷ ť
};

