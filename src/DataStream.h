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

    template<uint N, typename T>
    class has_bytes {

    public:
        enum {
            value = sizeof(T) >= N
        };
    };

    template<typename T>
    class can_read {
        typedef char one;
        typedef long two;

        template<typename C>
        static one test(decltype(&C::read));
        template<typename C>
        static two test(...);

    public:
        enum {
            value = sizeof(test<T>(0)) == sizeof(char)
        };
    };

    template<typename T>
    class can_write {
        typedef char one;
        typedef long two;

        template<typename C>
        static one test(decltype(&C::write));
        template<typename C>
        static two test(...);

    public:
        enum {
            value = sizeof(test<T>(0)) == sizeof(char)
        };
    };

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
    typename std::enable_if_t<std::is_fundamental<std::decay_t<T>>::value>
    read(T &t) {
        ByteData <T> bd(*this);
        t = std::move(bd.value);
    }

    template<typename T>
    typename std::enable_if_t<!std::is_fundamental<std::decay_t<T>>::value
                              &&
                              !std::is_same<std::decay_t<T>, std::string>::value>
    read(T &t) {
        static_assert(can_read<T>::value, "Trying to read an unserializable object");
        t.read(*this);
    }

    void read(std::string &str) {
        auto str_array = read<std::vector<byte>>();
        str = std::string(str_array.begin(), str_array.end());
    }

    template<typename T>
    void read(std::vector<T> &vec, size_t vec_length = 0) {
        if (vec_length == 0) readShortestInt(vec_length);
        int t_size = std::is_fundamental<std::decay_t<T>>::value ? sizeof(T) : 1; //this is just an optimization. If T is a non-fundamental type, sizeof(T) might contain extra padding bytes of T, which are not read from the stream
        if (this->pos + vec_length * t_size > this->length) throw std::runtime_error("Buffer overflow");
        vec.resize(vec_length);
        for (int i = 0; i < vec_length; ++i) {
            read(vec[i]);
        }
    }

    template<typename T>
    T read() {
        T val;
        read(val);
        return val;
    }

    // in data we receive, an integer may be 1 byte, 2 bytes, 4 bytes or 5 bytes
    template<typename T>
    typename std::enable_if_t<std::is_fundamental<std::decay_t<T>>::value
                              &&
                              has_bytes<4, std::decay_t<T>>::value>
    readShortestInt(T &value) {
        byte firstByte = buffer->at(pos);
        switch (firstByte & 0xE0) {
            case 0xE0: {
                skipBytes(1);
                int32_t cast;
                read(cast); // 4 bytes
                value = cast;
                return;
            }
            case 0xC0: {
                int32_t cast;
                read(cast); // 4 bytes
                value = cast & 0x3FFFFFFF;
                return;
            }
            case 0x80:
            case 0xA0: {
                int16_t cast;
                read(cast); // 2 bytes
                value = cast & 0x7FFF;
                return;
            }
            default: {
                byte cast;
                read(cast); // 1 byte
                value = cast;
                return;
            }
        }
    }

    template<typename T>
    typename std::enable_if_t<std::is_fundamental<std::decay_t<T>>::value
                              &&
                              has_bytes<4, std::decay_t<T>>::value>
    writeShortestInt(T &&value) {
        if (value < 0x80) {
            byte cast = value;
            write(cast);
        } else if (value < 0x4000) {
            ushort cast = value;
            write((int16_t) (cast | 0x8000));
        } else if (value < 0x20000000) {
            int32_t cast = value;
            write(cast | 0xC0000000);
        } else {
            int32_t cast = value;
            write((byte) 0xE0);
            write(cast);
        }
    }

    template<typename T>
    typename std::enable_if_t<std::is_fundamental<std::decay_t<T>>::value>
    write(T &&t) {
        ByteData <T> bd(*this, static_cast<T>(t));
    }

    template<typename T>
    typename std::enable_if_t<!std::is_fundamental<std::decay_t<T>>::value
                              &&
                              !std::is_same<std::decay_t<T>, std::string>::value>
    write(T &&t) {
        static_assert(can_write<T>::value, "Trying to write an unserializable object");
        t.write(*this);
    }

    void write(std::string &str) {
        std::vector<byte> data(str.begin(), str.end());
        write(data);
    }

    template<typename T>
    void write(std::vector<T> &vec) {
        int t_size = std::is_fundamental<std::decay_t<T>>::value ? sizeof(T) : 1; //this is just an optimization. If T is a non-fundamental, sizeof(T) might contain extra padding bytes of T, which are not written into the stream
        size_t vec_length = vec.size();
        if (this->pos + vec_length * t_size > MAX_LENGTH) throw std::runtime_error("Buffer overflow");
        writeShortestInt(vec_length);
        for (const T &t : vec) {
            write(static_cast<base_type<T>>(t));
        }
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

    void skipBytes(size_t n) {
        pos += n;
    }

    void setPositionForward(size_t i) {
        if (i < pos) throw std::runtime_error("Trying to set invalid buffer position.");
        pos = i;
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

        static_assert(std::is_fundamental<base_type<T>>::value, "Trying to create ByteData of non-fundamental type");

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
