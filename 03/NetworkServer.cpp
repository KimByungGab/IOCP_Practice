#include "NetworkServer.h"

void NetworkServer::OnConnect(const unsigned int clientIndex)
{
	cout << "[OnConnect] ÀÎµ¦½º: " << clientIndex << endl;
}

void NetworkServer::OnClose(const unsigned int clientIndex)
{
	cout << "[OnClose] ÀÎµ¦½º: " << clientIndex << endl;
}

void NetworkServer::OnReceive(const unsigned int clientIndex, const unsigned int size, char* pData)
{
	cout << "[OnReceive] ÀÎµ¦½º: " << clientIndex << " / µ¥ÀÌÅÍÅ©±â: " << size;
}
