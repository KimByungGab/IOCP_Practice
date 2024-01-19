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
void NetworkServer::OnReceive(const unsigned int clientIndex, const unsigned int size)
{
	cout << "[OnReceive] �ε���: " << clientIndex << " / ������ũ��: " << size;
}

// ���� ����
void NetworkServer::Run()
{
	StartServer();
}

// ���� ����
void NetworkServer::End()
{
	// ��Ʈ��ũ ���� ������ ���� ����
	DestroyThread();
}