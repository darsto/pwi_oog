//
// Created by dar on 6/30/16.
//

#ifndef PWI_OOG_ROLEINFO_H
#define PWI_OOG_ROLEINFO_H

#include <DataStream.h>

struct EquipInfo {
    int id;
    int cellId;
    int count;
    int maxCount;
    std::vector<byte> itemData;

    unsigned int procType;
    unsigned int expireDate;
    //[8 bytes]
    unsigned int mask;

    void read(DataStream &stream) {
        stream.read(id);
        stream.read(cellId);
        stream.read(count);
        stream.read(maxCount);
        stream.read(itemData);

        stream.read(procType);
        stream.read(expireDate);
        stream.skipBytes(8);
        stream.read(mask);
    }

    void write(DataStream &stream) {
        stream.write(id);
        stream.write(cellId);
        stream.write(count);
        stream.write(maxCount);
        stream.write(itemData);

        stream.write(procType);
        stream.write(expireDate);
        stream.skipBytes(8);
        stream.write(mask);
    }
};

struct RoleInfo {
    unsigned int UID;
    byte genderId;
    byte race;
    byte occupationId;
    int level;
    std::string name;
    std::vector<byte> face;
    std::vector<EquipInfo> equipInfo;
    bool activate;
    unsigned int deleteTime;
    unsigned int createTime;
    unsigned int lastOnlineTime;
    float posX;
    float posZ;
    float posY;

    void read(DataStream &stream) {
        stream.read(UID);
        stream.read(genderId);
        stream.read(race);
        stream.read(occupationId);
        stream.read(level);
        stream.skipBytes(4);
        stream.read(name);
        stream.read(face);
        stream.read(equipInfo);

        stream.read(activate);
        stream.read(deleteTime);
        stream.read(createTime);
        stream.read(lastOnlineTime);
        stream.read(posX);
        stream.read(posZ);
        stream.read(posY);
    }

    void write(DataStream &stream) {
        stream.write(UID);
        stream.write(genderId);
        stream.write(race);
        stream.write(occupationId);
        stream.write(level);
        stream.skipBytes(4);
        stream.write(name);
        stream.write(face);
        stream.write(equipInfo);

        stream.write(activate);
        stream.write(deleteTime);
        stream.write(createTime);
        stream.write(lastOnlineTime);
        stream.write(posX);
        stream.write(posZ);
        stream.write(posY);
    }
};

#endif //PWI_OOG_ROLEINFO_H
