//
// Created by dar on 6/16/16.
//

#ifndef PWI_OOG_PACKET_H
#define PWI_OOG_PACKET_H

#include <ConnectionData.h>
#include <DataStream.h>
#include <functional>

enum class PacketType {
    CLIENT_DEFAULT = 1 << 0,
    SERVER_DEFAULT = 1 << 8,
    SERVER_CONTAINER = 1 << 9 | (int) SERVER_DEFAULT
};

class Packet {
public:
    using handler_type = std::function<void(Packet &)>;

    Packet(DataStream &stream, ConnectionData &connectionData)
        : stream(stream),
          connectionData(connectionData) {
        prepareData();
    }

    void setHandler(handler_type handler) {
        if (this->data_handler != nullptr) throw std::runtime_error("Trying to reassign data handler");
        this->data_handler = handler;
    }

    const DataStream &getStream() const {
        return stream;
    }

    DataStream &getStream() {
        return stream;
    }

    void handleData() {
        if (data_handler) data_handler(*this);
    }

    virtual void prepareData() {}

    ~Packet() {}; //disable move

    Packet(const Packet &) = delete;

    friend class PacketManager;

protected:
    DataStream stream;
    ConnectionData &connectionData;
    handler_type data_handler = nullptr;
};

#endif //PWI_OOG_PACKET_H
