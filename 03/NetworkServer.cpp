#include "NetworkServer.h"

void NetworkServer::OnConnect(const unsigned int clientIndex)
{
	cout << "[OnConnect] �ε���: " << clientIndex << endl;
}

void NetworkServer::OnClose(const unsigned int clientIndex)
{
	cout << "[OnClose] �ε���: " << clientIndex << endl;
}

void NetworkServer::OnReceive(const unsigned int clientIndex, const unsigned int size, char* pData)
{
	cout << "[OnReceive] �ε���: " << clientIndex << " / ������ũ��: " << size;
}
