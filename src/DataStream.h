//
// Created by dar on 6/10/16.
//

#ifndef PWI_OOG_DATASTREAM_H
#define PWI_OOG_DATASTREAM_H

#include <sys/types.h>
#include <cstdio>
#include <exception>
#include <array>
#include <memory>

using byte = unsigned char;

/*
 * A wrapper class for serialized data.
 */
class DataStream {
    template<typename T>
    using base_type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

public:
    DataStream(std::shared_ptr<std::array<byte, 1024>> &buffer, size_t length)
        : buffer(buffer),
          length(length),
          pos(0),
          sealed(true),
          swapped(true) {}

    DataStream(std::shared_ptr<std::array<byte, 1024>> &buffer)
        : buffer(buffer),
          length(0),
          pos(0),
          sealed(false),
          swapped(true) {}

    template<typename T>
    T read() {
        ByteData <T> bd(*this);
        return bd.value;
    }

    template<typename T>
    void read(T &&t) {
        ByteData <T> bd(*this);
        t = std::move(bd.value);
    }

    template<typename T>
    std::vector<T> readArray(int elements_num) {
        if (pos + elements_num * sizeof(T) > length) throw std::runtime_error("Buffer overflow");
        std::vector<T> ret(elements_num);
        for (int i = 0; i < elements_num; ++i) {
            ByteData <T> bd(*this);
            ret[i] = std::move(bd.value);
        }
        return ret;
    }

    template<typename T>
    std::vector<T> readArray() {
        byte length = read<byte>();
        return readArray<T>(length);
    }

    std::string readString() {
        auto str_array = readArray<byte>();
        return std::string(str_array.begin(), str_array.end());
    }

    template<typename T>
    void write(T &&t) {
        ByteData <T> bd(*this, static_cast<T>(t));
    }

    template<typename T>
    void writeArray(const std::vector<T> &arr) {
        u_long length = arr.size();

        if (length < 256) {
            write((byte) length);
            for (const T &t : arr) {
                write(static_cast<base_type<T>>(t));
            }
        } else {
            throw std::runtime_error("Trying to write array of length >= 128.");
        }
    }

    void writeString(const std::string &str) {
        std::vector<byte> data(str.begin(), str.end());
        writeArray(data);
    }

    size_t getLength() const {
        return length;
    }

    size_t getPos() const {
        return pos;
    }

    const std::shared_ptr<std::array<byte, 1024>> getBuffer() const {
        return buffer;
    }

    bool isSealed() const {
        return sealed;
    }

    void seal() {
        this->sealed = true;
    }

    bool isSwapped() const {
        return swapped;
    }

    void setSwapped(bool swapped) {
        this->swapped = swapped;
    }

    static const int MAX_LENGTH = 1024;

private:
    std::shared_ptr<std::array<byte, 1024>> buffer;
    size_t length;
    size_t pos;
    bool sealed; //when true, the stream is read-only

    /*when true, all primitives are construced from read bytes in reverse order
     * [1 1 0 0] -> (swapped == 1) (int) 257
     * true by default */
    bool swapped;

    /*
     * A wrapper class for primitives (de)serialization
     */
    template<typename T>
    union ByteData {
        base_type<T> value;
        byte bytes[sizeof(T)];

        ByteData(DataStream &stream) {
            if (stream.pos + sizeof(T) > stream.length) throw std::runtime_error("Buffer overflow");
            for (int i = 0; i < sizeof(T); ++i) {
                bytes[stream.swapped ? (sizeof(T) - 1 - i) : i] = stream.buffer.get()->at(stream.pos + i);
            }
            stream.pos += sizeof(T);
        }

        template<typename U = T>
        ByteData(DataStream &stream, U &&value) {
            if (stream.sealed) throw std::runtime_error("Trying to write bytes into a sealed stream.");
            this->value = value;
            if (stream.pos + sizeof(U) > MAX_LENGTH) throw std::runtime_error("Buffer overflow");
            for (int i = 0; i < sizeof(U); ++i) {
                stream.buffer.get()->at(stream.pos + (stream.swapped ? (sizeof(U) - 1 - i) : i)) = bytes[i];
            }
            stream.pos += sizeof(U);
            stream.length = stream.pos;
        }
    };
};

#endif //PWI_OOG_DATASTREAM_H
