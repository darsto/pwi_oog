//
// Created by dar on 7/10/16.
//

#ifndef PWI_OOG_NSTRING_H
#define PWI_OOG_NSTRING_H

#include <string>
#include "DataStream.h"

class NString {
    std::string data;

public:
    NString();
    NString(const std::string &data);
    const std::string &get() const;
    void read(DataStream &stream);
    void write(DataStream &stream);
};

#endif //PWI_OOG_NSTRING_H
