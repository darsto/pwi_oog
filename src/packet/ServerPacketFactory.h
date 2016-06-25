//
// Created by dar on 6/23/16.
//

#ifndef PWI_OOG_PACKETFACTORY_H
#define PWI_OOG_PACKETFACTORY_H

#include "PacketManager.h"

struct ServerPacketBaseFactory {
    const PacketType type;
    const int id;

    ServerPacketBaseFactory(const PacketType type, const int id) : type(type), id(id) {}

    void registerHandler(Packet::handler_type handler) {
        if (this->handler) throw 300; //todo
        this->handler = handler;
    }

    virtual std::unique_ptr<Packet> createPacket(DataStream &stream, ConnectionData &data) = 0;
    virtual const std::type_info &type_info() const = 0;

protected:
    Packet::handler_type handler = nullptr;
};

template<class TPacket>
struct ServerPacketFactory : ServerPacketBaseFactory {
    using ServerPacketBaseFactory::ServerPacketBaseFactory;

    std::unique_ptr<Packet> createPacket(DataStream &stream, ConnectionData &data) override {
        auto packet = std::make_unique<TPacket>(stream, data);
        packet->prepareData();
        if (handler) handler(*packet.get());
        return packet;
    }

    const std::type_info &type_info() const override {
        return typeid(TPacket);
    }
};

#define SERVER_PACKET(pId, pName) \
class pName ; \
namespace { \
    static ServerPacketFactory<pName> pName ## FactoryInstance(PacketType::SERVER_DEFAULT, pId); \
    static bool pName ## Registered = PacketManager::getInstance().registerPacket(& pName ## FactoryInstance); \
} \
class pName : public Packet { \
public: \
    static const int ID = pId; \
protected: \
    using Packet::Packet;

#endif //PWI_OOG_PACKETFACTORY_H