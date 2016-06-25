//
// Created by dar on 6/16/16.
//

#ifndef PWI_OOG_HEXCONVERTER_H
#define PWI_OOG_HEXCONVERTER_H

#include <vector>
#include "DataStream.h"

// doesn't work with uppercase characters
unsigned char hexToDec(char hex);

std::vector<byte> hexToDec(const std::string &hex);

#endif //PWI_OOG_HEXCONVERTER_H
