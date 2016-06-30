//
// Created by dar on 6/10/16.
//

#ifndef PWI_OOG_DATASTREAM_H
#define PWI_OOG_DATASTREAM_H

#include <sys/types.h>
#include <cstdio>
#include <exception>
#include <vector>
#include <array>
#include <memory>

using byte = unsigned char;

class DataStream;

struct uni_int { //universal int, takes 1, 2, 4 or 5 bytes
    int32_t value;

    operator int32_t();
    void read(DataStream &stream);
    void write(DataStream &stream);
};

/*
 * A wrapper class for serialized data.
 */
class DataStream {
    template<typename T>
    using base_type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

    template<uint N, typename T>
    struct has_bytes {
        static constexpr bool value = sizeof(T) >= N;
    };

    template<typename T>
    struct can_read {
    private:
        static constexpr auto check()
        -> typename std::is_same<
            void, decltype(std::declval<T>().read(std::declval<DataStream &>()))
        >::type;

        template<typename>
        static constexpr std::false_type check(...);

        typedef decltype(check()) type;

    public:
        static constexpr bool value = type::value;
    };

    template<typename T>
    struct can_write {
    private:
        static constexpr auto check()
        -> typename std::is_same<
            void, decltype(std::declval<T>().write(std::declval<DataStream &>()))
        >::type;

        template<typename>
        static constexpr std::false_type check(...);

        typedef decltype(check()) type;

    public:
        static constexpr bool value = type::value;
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
    void read(std::vector<T> &vec, size_t vec_length_t = 0) {
        uni_int vec_length = {(int32_t) vec_length_t};
        if (vec_length == 0) read(vec_length);
        //this is just an optimization. If T is a non-fundamental type, sizeof(T) might contain extra padding bytes of T, which are not read from the stream
        int t_size = std::is_fundamental<std::decay_t<T>>::value ? sizeof(T) : 1;
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
        //this is just an optimization. If T is a non-fundamental, sizeof(T) might contain extra padding bytes of T, which are not written into the stream
        int t_size = std::is_fundamental<std::decay_t<T>>::value ? sizeof(T) : 1;
        uni_int vec_length = {(int32_t) vec.size()};
        if (this->pos + vec_length * t_size > MAX_LENGTH) throw std::runtime_error("Buffer overflow");
        write(vec_length);
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
        if (!sealed) length = pos;
    }

    void setPositionForward(size_t i) {
        if (i < pos) throw std::runtime_error("Trying to set invalid buffer position.");
        pos = i;
        if (!sealed) length = pos;
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
