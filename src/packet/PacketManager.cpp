//
// Created by dar on 6/19/16.
//

#include "PacketManager.h"
#include "packet/ServerPacketFactory.h"

bool PacketManager::registerPacket(ServerPacketBaseFactory *packet) {
    auto position = getPosition(packet->type, packet->id);
    if (position >= factories.size()) factories.resize(position + 1);
    factories[position] = packet;
    return true;
}

void PacketManager::registerHandler(ServerPacketBaseFactory *factory,  Packet::handler_type handler) {
    auto position = getPosition(factory->type, factory->id);
    if (position >= factories.size() || !factories[position]) throw std::out_of_range("Trying to set handler of unknown packet");
    factories[position]->registerHandler(handler);
}

ServerPacketBaseFactory *PacketManager::getFactory(PacketType type, int id) {
    auto position = getPosition(type, id);
    if (position >= factories.size() || !factories[position]) throw std::out_of_range("Trying to get a factory of unknown packet");
    return factories[position];
}

const std::type_info &PacketManager::getFactoryInfo(ServerPacketBaseFactory *factory) {
    return factory->type_info();
}
