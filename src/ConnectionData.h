//
// Created by dar on 6/14/16.
//

#ifndef PWI_OOG_CONNECTIONDATA_H
#define PWI_OOG_CONNECTIONDATA_H

#include <vector>
#include "DataStream.h"
#include "Cipher.h"

struct ConnectionData {
    std::vector<byte> serverKey;
    std::vector<byte> serverVersion;
    std::vector<byte> CMKey;
    std::vector<byte> SMKey;
    std::vector<byte> authHash;
    Cipher cipher;
    std::string crc;
    std::string login;
    unsigned int accountId = 0;
};

#endif //PWI_OOG_CONNECTIONDATA_H
