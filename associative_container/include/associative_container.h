#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ASSOCIATIVE_CONTAINER_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ASSOCIATIVE_CONTAINER_H

#include <iostream>
#include <vector>
#include <operation_not_supported.h>

template<typename compare, typename tkey>
concept compator = requires(const compare c, const tkey& lhs, const tkey& rhs) // существует ли возможность
                   {
                       {c(lhs, rhs)} -> std::same_as<bool>;
                   } && std::copyable<compare> && std::default_initializable<compare>;
// объект compare является копируемым и дефолтно инициализируемым
// (можно ли создать объект типа compare без параметров, т.е. констурктором по умолчанию)

template<typename f_iter, typename tkey, typename tval>
concept input_iterator_for_pair = std::input_iterator<f_iter> // требование для итератора быть входным
        && std::same_as<typename std::iterator_traits<f_iter>::value_type, std::pair<tkey, tval>>;  // тип значений на которые указывает итератор f_iter является парой std::pair<tkey, tval>

        // typename тк компилятор не гарантирует, что выражение std::iterator_traits<f_iter>::value_type является типом,
        // поэтому мы явно указываем, что это тип,
        // с помощью typename.


/**
 * You will strongly need this while doing your cursal work
**/
#if 0
template<typename T>
concept serializable = requires (T t, std::fstream s, nlohmann::json j)
                       {
                           {t.serialize(s)};
                           {T::deserialize(s)} -> std::same_as<T>;
                           {t.serializa_size()} -> std::same_as<size_t>;
                           {t.to_json()} -> std::same_as<nlohmann::json>;
                           {T::from_json(j)} -> std::same_as<T>;
                       } && std::copyable<T>;
#endif

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ASSOCIATIVE_CONTAINER_H