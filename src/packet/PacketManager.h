//
// Created by dar on 6/16/16.
//

#ifndef PWI_OOG_PACKETMANAGER_H
#define PWI_OOG_PACKETMANAGER_H

#include "Packet.h"

class ServerPacketBaseFactory;

class PacketManager {
public:
    static PacketManager &getInstance() {
        static PacketManager instance;
        return instance;
    }

    bool registerPacket(ServerPacketBaseFactory *packet);

    void registerHandler(ServerPacketBaseFactory *factory, Packet::handler_type handler);

    template <class TPacket>
    void registerHandler(Packet::handler_type handler) {
        for (ServerPacketBaseFactory *factory : factories) {
            if (factory != nullptr && typeid(TPacket) == getFactoryInfo(factory)) {
                return registerHandler(factory, handler);
            }
        }
        throw std::out_of_range("Trying to set handler of unknown packet");
    }

    ServerPacketBaseFactory *getFactory(PacketType type, int id);
    const std::type_info &getFactoryInfo(ServerPacketBaseFactory *factory);

    PacketManager(PacketManager const &) = delete;
    void operator=(PacketManager const &)  = delete;

private:
    std::vector<ServerPacketBaseFactory*> factories;

    unsigned long getPosition(PacketType type, int id) {
        return (unsigned long) type | id;
    }

    PacketManager() {};

};

#endif //PWI_OOG_PACKETMANAGER_H
