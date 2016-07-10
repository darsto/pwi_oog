//
// Created by dar on 7/10/16.
//

#include "NString.h"

NString::NString() {}

NString::NString(const std::string &data) : data(data) {}

const std::string &NString::get() const {
    return data;
}

void NString::read(DataStream &stream) {
    uni_int length = stream.read<uni_int>().value / 2;

    std::vector<byte> str_array(length);
    for (int i = 0; i < length; ++i) {
        str_array[i] = stream.read<byte>();
        stream.skipBytes(1);
    }

    data = std::string(str_array.begin(), str_array.end());
}

void NString::write(DataStream &stream) {
    uni_int length = data.length() * 2;
    stream.write(length);

    for (int i = 0; i < data.length(); ++i) {
        stream.write((byte) data[i]);
        stream.write((byte) 0);
    }
}