
#ifndef SERIALIZATION_TRAITS_HPP
#define SERIALIZATION_TRAITS_HPP

#pragma once
#include <fstream>
#include <string>
#include <utility>
#include <concepts>

template<typename T>
struct serialization_traits;

template<>
struct serialization_traits<int> {
    static void serialize(const int& value, std::fstream& stream) {
        stream.write(reinterpret_cast<const char*>(&value), sizeof(int));
    }
    static int deserialize(std::fstream& stream) {
        int value;
        stream.read(reinterpret_cast<char*>(&value), sizeof(int));
        return value;
    }
    static size_t serialize_size(const int&) {
        return sizeof(int);
    }
};

template<>
struct serialization_traits<long long> {
    static void serialize(const long long& value, std::fstream& stream) {
        stream.write(reinterpret_cast<const char*>(&value), sizeof(long long));
    }
    static long long deserialize(std::fstream& stream) {
        long long value;
        stream.read(reinterpret_cast<char*>(&value), sizeof(long long));
        return value;
    }
    static size_t serialize_size(const long long&) {
        return sizeof(long long);
    }
};

template<>
struct serialization_traits<unsigned> {
    static void serialize(const unsigned& value, std::fstream& stream) {
        stream.write(reinterpret_cast<const char*>(&value), sizeof(unsigned));
    }
    static unsigned deserialize(std::fstream& stream) {
        unsigned value;
        stream.read(reinterpret_cast<char*>(&value), sizeof(unsigned));
        return value;
    }
    static size_t serialize_size(const unsigned&) {
        return sizeof(unsigned);
    }
};


template<>
struct serialization_traits<std::string> {
    static void serialize(const std::string& value, std::fstream& stream) {
        size_t size = value.size();
        stream.write(reinterpret_cast<const char*>(&size), sizeof(size));
        stream.write(value.data(), size);
    }

    static std::string deserialize(std::fstream& stream) {
        size_t size;
        stream.read(reinterpret_cast<char*>(&size), sizeof(size));
        std::string result(size, '\0');
        stream.read(result.data(), size);
        return result;
    }

    static size_t serialize_size(const std::string& value) {
        return sizeof(size_t) + value.size();
    }
};

template<typename T>
concept serializable = requires(const T t, std::fstream& s) {
    { serialization_traits<T>::serialize(t, s) };
    { serialization_traits<T>::deserialize(s) } -> std::same_as<T>;
    { serialization_traits<T>::serialize_size(t) } -> std::same_as<size_t>;
};

template<typename tkey, typename tvalue>
    requires serializable<tkey> && serializable<tvalue>
struct serialization_traits<std::pair<tkey, tvalue>> {
    static void serialize(const std::pair<tkey, tvalue>& value, std::fstream& stream) {
        serialization_traits<tkey>::serialize(value.first, stream);
        serialization_traits<tvalue>::serialize(value.second, stream);
    }

    static std::pair<tkey, tvalue> deserialize(std::fstream& stream) {
        auto first = serialization_traits<tkey>::deserialize(stream);
        auto second = serialization_traits<tvalue>::deserialize(stream);
        return {first, second};
    }

    static size_t serialize_size(const std::pair<tkey, tvalue>& value) {
        return serialization_traits<tkey>::serialize_size(value.first) +
               serialization_traits<tvalue>::serialize_size(value.second);
    }
};


#endif //SERIALIZATION_TRAITS_HPP


