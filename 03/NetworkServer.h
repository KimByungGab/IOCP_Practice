#pragma once

#include "NetworkCore.h"

class NetworkServer : public NetworkCore
{
	virtual void OnConnect(const unsigned int clientIndex) override;
	virtual void OnClose(const unsigned int clientIndex) override;
	virtual void OnReceive(const unsigned int clientIndex, const unsigned int size, char* pData) override;
};

