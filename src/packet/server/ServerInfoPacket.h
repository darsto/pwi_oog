//
// Created by dar on 6/16/16.
//

#ifndef PWI_OOG_SERVERINFOPACKET_H
#define PWI_OOG_SERVERINFOPACKET_H

#include <packet/Packet.h>
#include <packet/ServerPacketFactory.h>

SERVER_PACKET(1, ServerInfoPacket)
public:
    virtual void prepareData() override {
        key = stream.readArray<byte>();
        serverVersion = stream.readArray<byte>(4);
        hashToSha256 = stream.read<byte>();
        crc = stream.readString();
        unknown1 = stream.read<byte>();
    }

    std::vector<byte> key;
    std::vector<byte> serverVersion;
    bool hashToSha256;
    std::string crc;
    byte unknown1;
};

#endif //PWI_OOG_SERVERINFOPACKET_H
