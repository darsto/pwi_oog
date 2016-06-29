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
        stream.read(key);
        stream.read(serverVersion, 4);
        stream.read(hashToSha256);
        stream.read(crc);
        stream.skipBytes(1);
    }

    std::vector<byte> key;
    std::vector<byte> serverVersion;
    bool hashToSha256;
    std::string crc;
    //[1 byte]
};

#endif //PWI_OOG_SERVERINFOPACKET_H
