#include "Packet.h"

// ��Ŷ ����1
void Packet::Set(Packet& value)
{
	mSessionIndex = value.mSessionIndex;
	mDataSize = value.mDataSize;
	m_pPacketData = value.m_pPacketData;
}

// ��Ŷ ����2
void Packet::Set(Packet* value)
{
	mSessionIndex = value->mSessionIndex;
	mDataSize = value->mDataSize;
	m_pPacketData = value->m_pPacketData;
}

// ��Ŷ ����3
void Packet::Set(unsigned int sessionIndex, unsigned int dataSize, char* packetData)
{
	mSessionIndex = sessionIndex;
	mDataSize = dataSize;
	m_pPacketData = packetData;
}
