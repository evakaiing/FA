//
// Created by Des Caldnd on 5/27/2024.
//

#ifndef MP_OS_BIG_INT_H
#define MP_OS_BIG_INT_H

#include <vector>
#include <utility>
#include <iostream>
#include <concepts>
#include <pp_allocator.h>
#include <not_implemented.h>

namespace __detail
{
    constexpr unsigned int generate_half_mask()
    {
        unsigned int res = 0;

        for(size_t i = 0; i < sizeof(unsigned int) * 4; ++i)
        {
            res |= (1u << i);
        }

        return res;
    }

    constexpr size_t nearest_greater_power_of_2(size_t size) noexcept
    {
        int ones_counter = 0, index = -1;

        constexpr const size_t o = 1;

        for (int i = sizeof(size_t) * 8 - 1; i >= 0; --i)
        {
            if (size & (o << i))
            {
                if (ones_counter == 0)
                    index = i;
                ++ones_counter;
            }
        }

        return ones_counter <= 1 ? (1u << index) : (1u << (index + 1));
    }

    inline int countl_zero_uint_impl(unsigned int x) {
        if (x == 0) {
            return std::numeric_limits<unsigned int>::digits; // Все биты нулевые
        }

        int count = 0;
        // Определяем количество бит в unsigned int (обычно 32)
        const unsigned int num_bits = std::numeric_limits<unsigned int>::digits;

        // Начинаем проверку со старшего бита
        // Маска для самого старшего бита (например, 0x80000000 для 32-битного unsigned int)
        unsigned int mask = 1u << (num_bits - 1);

        while (mask != 0) { // Пока маска не сместится за пределы числа
            if ((x & mask) == 0) { // Если текущий старший бит равен 0
                count++;          // Увеличиваем счетчик ведущих нулей
            } else {
                break;            // Нашли первый установленный бит, дальше нулей нет
            }
            mask >>= 1;           // Сдвигаем маску вправо для проверки следующего бита
        }
        return count;
    }
}

class big_int
{
    bool _sign; // 1 +  0 -
    std::vector<unsigned int, pp_allocator<unsigned int>> _digits;

    friend class fraction;

private:
    void optimise() noexcept;
    static std::strong_ordering modulo_comparison(const big_int &left, const big_int &right, size_t shift = 0) noexcept;

    void get_new_value_from_string(const std::string& num, unsigned int radix);

    static void plus_operation_without_sign(big_int &left, const big_int &right, size_t shift = 0) noexcept;
    static void minus_operation_without_sign(big_int &left, const big_int &right, size_t shift = 0);

    static big_int multiply_table(const big_int &left, const big_int& right) noexcept;
    static big_int multiply_karatsuba(const big_int& left, const big_int& right);
    static big_int divide_table(const big_int& numerator, const big_int& denominator);
    static big_int binary_search_quotient(const big_int& numerator, const big_int& denominator);

public:
    static std::strong_ordering compare_with_sign(const big_int &left, const big_int& right, size_t shift = 0) noexcept;
    enum class multiplication_rule
    {
        trivial,
        Karatsuba,
        SchonhageStrassen
    };

    enum class division_rule
    {
        trivial,
        Newton,
        BurnikelZiegler
    };

private:

    /** Decides type of mult/div that depends on size of lhs and rhs
     */
    multiplication_rule decide_mult(size_t rhs) const noexcept;
    division_rule decide_div(size_t rhs) const noexcept;

public:

    using value_type = unsigned int;

    template<class alloc>
    explicit big_int(const std::vector<unsigned int, alloc> &digits, bool sign = true, pp_allocator<unsigned int> allocator = pp_allocator<unsigned int>());

    explicit big_int(const std::vector<unsigned int, pp_allocator<unsigned int>> &digits, bool sign = true);

    explicit big_int(std::vector<unsigned int, pp_allocator<unsigned int>> &&digits, bool sign = true) noexcept;

    explicit big_int(const std::string& num, unsigned int radix = 10, pp_allocator<unsigned int> = pp_allocator<unsigned int>());

    big_int(std::initializer_list<unsigned int> init, bool sign = true) noexcept;

    template<std::integral Num>
    explicit big_int(Num d, pp_allocator<unsigned int> = pp_allocator<unsigned int>());

    explicit big_int(pp_allocator<unsigned int> = pp_allocator<unsigned int>());

    big_int(const big_int &other);

    big_int(big_int &&other) noexcept;

    big_int& operator=(const big_int &other);
    big_int& operator=(big_int &&other) noexcept;

    explicit operator bool() const noexcept; //false if 0 , else true

    big_int& operator++() &;
    big_int operator++(int);

    big_int& operator--() &;
    big_int operator--(int);

    big_int operator-() const;

    big_int& operator+=(const big_int& other) &;

    /** Shift will be needed for multiplication implementation
     *  @example Shift = 0: 111 + 222 = 333
     *  @example Shift = 1: 111 + 222 = 2331
     */
    big_int& plus_assign(const big_int& other, size_t shift = 0) &;


    big_int& operator-=(const big_int& other) &;

    big_int& minus_assign(const big_int& other, size_t shift = 0) &;

    /** Delegates to multiply_assign and calls decide_mult
     */
    big_int& operator*=(const big_int& other) &;

    big_int& multiply_assign(const big_int& other, multiplication_rule rule = multiplication_rule::trivial) &;

    // Вспомогательные статические приватные методы для Шёнхаге-Штрассена
    static uint64_t ss_power(uint64_t base, uint64_t exp, uint64_t mod);
    static void ss_ntt_transform(std::vector<uint64_t>& data, uint64_t mod, uint64_t root_val_for_L, bool inverse);
    static uint64_t ss_solve_crt_two_moduli(uint64_t val1_mod_m1, uint64_t val2_mod_m2);
    static big_int ss_multiply_core_crt(const big_int& a, const big_int& b, pp_allocator<unsigned int> alloc);

    // Основной статический метод для Шёнхаге-Штрассена
    static big_int multiply_schonhage_strassen(const big_int& left, const big_int& right);

    static uint64_t ss_modInverse(uint64_t n, uint64_t mod);

    big_int& operator/=(const big_int& other) &;

    big_int& divide_assign(const big_int& other, division_rule rule = division_rule::trivial) &;

    big_int& operator%=(const big_int& other) &;

    big_int& modulo_assign(const big_int& other, division_rule rule = division_rule::trivial) &;

    big_int operator+(const big_int& other) const;
    big_int operator-(const big_int& other) const;
    big_int operator*(const big_int& other) const;
    big_int operator/(const big_int& other) const;
    big_int operator%(const big_int& other) const;

    std::strong_ordering operator<=>(const big_int& other) const noexcept;

    bool operator==(const big_int& other) const noexcept;

    big_int& operator<<=(size_t shift) &;

    big_int& operator>>=(size_t shift) &;


    big_int operator<<(size_t shift) const;
    big_int operator>>(size_t shift) const;

    big_int operator~() const;

    big_int& operator&=(const big_int& other) &;

    big_int& operator|=(const big_int& other) &;

    big_int& operator^=(const big_int& other) &;


    big_int operator&(const big_int& other) const;
    big_int operator|(const big_int& other) const;
    big_int operator^(const big_int& other) const;

    friend std::ostream &operator<<(std::ostream &stream, big_int const &value);

    friend std::istream &operator>>(std::istream &stream, big_int &value);

    [[nodiscard]] std::string to_string() const;

    [[nodiscard]] bool is_negative() const noexcept;
    [[nodiscard]] bool is_positive() const noexcept;
    [[nodiscard]] bool is_zero() const noexcept;

    big_int abs() const;
};

template<class alloc>
big_int::big_int(const std::vector<unsigned int, alloc> &digits, bool sign, pp_allocator<unsigned int> allocator) : _sign(sign)
{
    _digits = std::vector<unsigned int, pp_allocator<unsigned int>>(digits.begin(), digits.end(), allocator);

    optimise();

}

template<std::integral Num>
big_int::big_int(Num num, pp_allocator<unsigned int> alloc)
    : _digits(alloc), _sign(true)
{
    uint64_t value = 0;

    if constexpr (std::signed_integral<Num>) {
        if (num < 0) {
            _sign = false;
            value = static_cast<uint64_t>(-static_cast<int64_t>(num));
        } else {
            value = static_cast<uint64_t>(num);
        }
    } else {
        value = static_cast<uint64_t>(num);
    }

    while (value > 0) {
        _digits.push_back(static_cast<unsigned int>(value & 0xFFFFFFFF));
        value >>= 32;
    }


    optimise();
}







big_int operator""_bi(unsigned long long n);

#endif //MP_OS_BIG_INT_H
