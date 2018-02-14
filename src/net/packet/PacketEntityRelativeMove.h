#ifndef PACKETENTITYRELATIVEMOVE_H
#define PACKETENTITYRELATIVEMOVE_H

#include "Packet.h"
#include "PacketGlobals.h"

class PacketEntityRelativeMove : public Packet
{
public:
    PacketEntityRelativeMove() : Packet(PACKET_ENTITY_RELATIVE_MOVE) {}

    void Read(const Session &session) override
    {
        m_EID = session.Read<int32_t>();
        m_DX = session.Read<char>();
        m_DY = session.Read<char>();
        m_DZ = session.Read<char>();
    }

    void Action() override
    {
    }
    Packet *CreateInstance() const override
    {
        return new PacketEntityRelativeMove();
    }

protected:
    void SendContent(const Session &session) const override
    {
    }

    int32_t m_EID=0;
    char m_DX = 0, m_DY = 0, m_DZ = 0;
};

#endif // PACKETENTITYRELATIVEMOVE_H
