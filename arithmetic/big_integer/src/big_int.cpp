//
// Created by Des Caldnd on 5/27/2024.
//

#include "../include/big_int.h"
#include <ranges>
#include <exception>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>



namespace {
    const uint64_t SS_MOD1 = 998244353;
    const uint64_t SS_ROOT1 = 3;
    const uint64_t SS_MOD2 = 1004535809;
    const uint64_t SS_ROOT2 = 3;

    uint64_t get_m1_inv_m2() {
        // Вызываем статический метод класса big_int
        static const uint64_t val = big_int::ss_modInverse(SS_MOD1, SS_MOD2);
        return val;
    }
    uint64_t get_m_product() {
        static const uint64_t val = SS_MOD1 * SS_MOD2;
        return val;
    }
}


std::strong_ordering big_int::modulo_comparison(const big_int &left, const big_int &right, size_t shift) noexcept {

    size_t effective_right_size = right._digits.size() + shift;
    size_t left_size = left._digits.size();

    // Сначала сравним длины чисел.
    if (left_size < effective_right_size) {
        return std::strong_ordering::less;
    }
    if (left_size > effective_right_size) {
        return std::strong_ordering::greater;
    }

    // Если длины совпадают, сравниваем цифры по порядку, начиная с самого старшего разряда.
    // Индекс i пробегает от left_size-1 до 0.
    for (size_t i = left_size; i-- > 0;) {
        unsigned int left_digit = left._digits[i];
        // Для right, если i находится в области смещения, т.е. i < shift, то соответствующая цифра равна 0.
        // Иначе индекс в right равен (i - shift).
        unsigned int right_digit = (i < shift || (i - shift) >= right._digits.size()) ? 0 : right._digits[i - shift];
        if (left_digit < right_digit) {
            return std::strong_ordering::less;
        }
        if (left_digit > right_digit) {
            return std::strong_ordering::greater;
        }
    }

    // Если все разряды совпадают, числа равны.
    return std::strong_ordering::equal;
}

std::strong_ordering big_int::compare_with_sign(const big_int &left, const big_int &right, size_t shift) noexcept {
    if (left._sign != right._sign) {
        // Если знаки разные, то положительное число больше отрицательного
        return left._sign ? std::strong_ordering::greater : std::strong_ordering::less;
    }
    if (left._sign == 0 && right._sign == 0) {
        auto result = modulo_comparison(left, right, shift);
        if (result == std::strong_ordering::less) return std::strong_ordering::greater;
        if (result == std::strong_ordering::greater) return std::strong_ordering::less;
        return std::strong_ordering::equal;
    }

    return modulo_comparison(left, right, shift);
}

std::strong_ordering big_int::operator<=>(const big_int &other) const noexcept {
    return compare_with_sign(*this, other, 0);
}

big_int::operator bool() const noexcept {
    return !(_digits.empty());
}

big_int &big_int::operator++() & {
    return *this += big_int({1});
}

big_int big_int::operator++(int) {
    big_int copy = *this;
    ++*this;
    return copy;
}

big_int &big_int::operator--() & {
    return *this -= big_int({1});
}

big_int big_int::operator--(int) {
    big_int copy = *this;
    --*this;
    return copy;
}

big_int &big_int::operator+=(const big_int &other) & {
    return plus_assign(other, 0);
}

big_int &big_int::operator-=(const big_int &other) & {
    return *this += (-other);
}

void big_int::plus_operation_without_sign(big_int &left, const big_int &right, size_t shift) noexcept {

    // Убедимся, что в результате достаточно места для разрядов right со сдвигом
    if (left._digits.size() < right._digits.size() + shift) {
        left._digits.resize(right._digits.size() + shift, 0);
    }

    unsigned int carry = 0;
    size_t i = 0;
    // Проходим по цифрам числа right
    constexpr unsigned int half_bits = sizeof(unsigned int) * 4;
    const unsigned int low_mask = __detail::generate_half_mask();
    unsigned int high_mask = low_mask << half_bits;

    for (; i < right._digits.size(); ++i) {
        size_t j = i + shift; // Смещаем индекс на shift
        // Если нет переноса разряда

        const unsigned int left_low = left._digits[j] & low_mask;
        const unsigned int left_high = left._digits[j] >> half_bits;
        const unsigned int right_low = right._digits[j] & low_mask;
        const unsigned int right_high = right._digits[j] >> half_bits;

        const unsigned int low_result = left_low + right_low + carry;
        const unsigned int high_result = (low_result >> half_bits) + left_high + right_high;

        left._digits[j] = (high_result << half_bits) | (low_result & low_mask);
        carry = high_result >> half_bits;
    }

    // Продолжаем обработку остаточного переноса, если он ещё остался
    size_t j = i + shift;
    while (carry != 0 && j < left._digits.size()) {
        const unsigned int left_low = left._digits[i] & low_mask;
        const unsigned int left_high = left._digits[i] >> half_bits;
        const unsigned int right_low = right._digits[i] & low_mask;
        const unsigned int right_high = right._digits[i] >> half_bits;

        const unsigned int low_result = left_low + right_low + carry;
        const unsigned int high_result = (low_result >> half_bits) + left_high + right_high;

        left._digits[j] = (high_result << half_bits) | (low_result & low_mask);
        carry = high_mask >> half_bits;

        ++j;
    }
    if (carry != 0) {
        left._digits.push_back(1);
    }
}

void big_int::minus_operation_without_sign(big_int &left, const big_int &right, size_t shift) {
    // left >= right
    if (modulo_comparison(left, right) == std::strong_ordering::less) {
        throw std::invalid_argument("Subtraction cannot be performed: left is smaller than right");
    }

    uint32_t carry = 0;
    size_t i = 0;
    // Проходим по цифрам числа right
    for (; i < right._digits.size(); ++i) {
        size_t j = i + shift; // Смещаем индекс на shift
        // Если результат вычитания без переноса положительный
        if (left._digits[j] >= (right._digits[i] + carry)) {
            left._digits[j] -= (right._digits[i] + carry);
            carry = 0;
        } else {
            // Если перенос
            left._digits[j] = (UINT_MAX - (right._digits[i] + carry) + left._digits[j] + 1);
            carry = 1; // После переноса нужно вычесть 1 из следующего разряда
        }
    }

    // Продолжаем обработку остаточного переноса
    size_t j = i + shift;
    while (carry != 0 && j < left._digits.size()) {
        if (left._digits[j] > 0) {
            left._digits[j]--;
            carry = 0;
        } else {
            left._digits[j] = UINT_MAX;
            carry = 1;
        }
        ++j;
    }

    left.optimise();
}

big_int big_int::multiply_table(const big_int &left, const big_int &right) noexcept {
    constexpr unsigned int HALF_BITS = (sizeof(unsigned int) * 8) / 2;  // 16
    constexpr unsigned int HALF_MASK = (1u << HALF_BITS) - 1;          // 0xFFFF

    size_t n = left._digits.size();
    size_t m = right._digits.size();
    big_int result;
    // Результат может занять n+m+1 разряд
    result._digits.assign(n + m + 1, 0u);

    for (size_t i = 0; i < n; ++i) {
        unsigned int a = left._digits[i];
        unsigned int a_low  = a & HALF_MASK;
        unsigned int a_high = a >> HALF_BITS;

        for (size_t j = 0; j < m; ++j) {
            unsigned int b = right._digits[j];
            unsigned int b_low  = b & HALF_MASK;
            unsigned int b_high = b >> HALF_BITS;

            // четыре частичных произведения
            unsigned int p_ll = a_low  * b_low;
            unsigned int p_lh = a_low  * b_high;
            unsigned int p_hl = a_high * b_low;
            unsigned int p_hh = a_high * b_high;

            // сумма средних произведений с переносом
            unsigned int mid_lo = (p_lh & HALF_MASK) + (p_hl & HALF_MASK);
            unsigned int carry_mid_lo = mid_lo >> HALF_BITS;
            mid_lo &= HALF_MASK;
            unsigned int mid_hi = (p_lh >> HALF_BITS) + (p_hl >> HALF_BITS) + carry_mid_lo;

            // собираем low часть: lo_lo | ((lo_hi + mid_lo) << HALF_BITS)
            unsigned int lo_lo = p_ll & HALF_MASK;
            unsigned int lo_hi = p_ll >> HALF_BITS;
            unsigned int res_lo_hi = lo_hi + mid_lo;
            unsigned int carry_lo = res_lo_hi >> HALF_BITS;
            res_lo_hi &= HALF_MASK;
            unsigned int low = (res_lo_hi << HALF_BITS) | lo_lo;

            // общий перенос в high: carry_lo + mid_hi + (p_hh low part)
            unsigned int carry_low = carry_lo + mid_hi;
            unsigned int high = p_hh + carry_low;


            // Добавляем low
            unsigned int sum1   = result._digits[i + j] + low;
            unsigned int carry1 = (sum1 < low) ? 1u : 0u;
            result._digits[i + j] = sum1;

            // Добавляем high + carry1
            unsigned int to_add = high + carry1;
            unsigned int sum2   = result._digits[i + j + 1] + to_add;
            unsigned int carry2 = (sum2 < to_add) ? 1u : 0u;
            result._digits[i + j + 1] = sum2;

            // Финальный перенос
            result._digits[i + j + 2] += carry2;
        }
    }

    // Убираем ведущие нули
    while (result._digits.size() > 1 && result._digits.back() == 0u) {
        result._digits.pop_back();
    }

    result._sign = (left._sign == right._sign);
    result.optimise();
    return result;
}

big_int big_int::multiply_karatsuba(const big_int& left, const big_int& right) {
    if (left.is_zero() || right.is_zero()) {
        return big_int(0);
    }
    size_t m = std::max(left._digits.size(), right._digits.size());

    // Базовый случай: если числа состоят из одной цифры (32 бита)
    if (m == 1) {
        big_int result;
        result = left * right;
        result._sign = (left._sign == right._sign);
        return result;
    }

    size_t half = (m + 1) / 2;  // Округляем вверх

    // Лямбда для безопасного разбиения
    auto split = [](const big_int& num, size_t pos) -> std::pair<big_int, big_int> {
        pos = std::min(pos, num._digits.size());
        return {
            big_int(std::vector<unsigned int>(num._digits.begin(), num._digits.begin() + pos), num._sign),
            big_int(std::vector<unsigned int>(num._digits.begin() + pos, num._digits.end()), num._sign)
    };
    };

    auto [left_low, left_high] = split(left, half);
    auto [right_low, right_high] = split(right, half);

    // Рекурсивные вызовы
    big_int z0 = multiply_karatsuba(left_low, right_low);
    big_int z2 = multiply_karatsuba(left_high, right_high);

    // Вычисляем z1 = (left_low + left_high) * (right_low + right_high) - z0 - z2
    big_int sum_left = left_low + left_high;
    big_int sum_right = right_low + right_high;
    big_int z1 = multiply_karatsuba(sum_left, sum_right) - z0 - z2;

    // Сборка результата
    big_int result = z0;
    result += (z1 << (half * sizeof(unsigned int) * 8));
    result += (z2 << (2 * half * sizeof(unsigned int) * 8));

    result._sign = (left._sign == right._sign);
    result.optimise();
    return result;
}




big_int big_int::binary_search_quotient(const big_int &numerator, const big_int &denominator) {
    // Бинарный двоичный поиск
    big_int result;
    result._digits.push_back(0);
    unsigned int &closest = result._digits[0];
    for (int i = sizeof(unsigned int) * 8 - 1; i >= 0; --i) {
        const unsigned int temp = closest;
        closest |= 1 << i;
        big_int multiplied = result * denominator;

        auto comp = modulo_comparison(multiplied, numerator);
        if (comp == std::strong_ordering::equal) {
            return result;
        } else if (comp == std::strong_ordering::less) {
            continue;
        } else {
            result._digits[0] = temp;
        }
    }
    return result;
}

big_int big_int::divide_table(const big_int &numerator, const big_int &denominator) {
    if (denominator.is_zero()) {
        throw std::invalid_argument("Zero division");
    }
    big_int abs_dividend = numerator;
    big_int abs_divisor = denominator;
    abs_dividend._sign = true;
    abs_divisor._sign = true;

    big_int quotient; // частное
    big_int remainder; // остаток
    remainder._digits.push_back(0);

    for (long long pos = static_cast<long long>(abs_dividend._digits.size()) - 1; pos >= 0; --pos) {
        // вставляем в остаток самую старшую цифру делимого
        remainder._digits.insert(remainder._digits.begin(), abs_dividend._digits[pos]);
        remainder.optimise();

        // Находим цифру частного:
        // такую что (digit * absDivisor) <= remainder.
        big_int digit = binary_search_quotient(remainder, abs_divisor);

        // Записываем найденную цифру в частное.
        quotient._digits.insert(quotient._digits.begin(), digit._digits[0]);

        // Вычитаем произведение делителя на найденную цифру из остатка.
        big_int prod = abs_divisor * digit;
        remainder = remainder - prod;
        remainder.optimise();
    }
    quotient.optimise();
    // Определяем знак частного: оно положительно, если делимое и делитель имеют одинаковый знак.
    quotient._sign = (numerator._sign == denominator._sign);
    return quotient;
}

big_int big_int::operator+(const big_int &other) const {
    big_int result = *this;
    result += other;
    return result;
}

big_int big_int::operator-(const big_int &other) const {
    return (*this + (-other));
}

big_int big_int::operator*(const big_int &other) const {
    return multiply_table(*this, other);
}

big_int big_int::operator/(const big_int &other) const {
    return divide_table(*this, other);
}

big_int big_int::operator%(const big_int &other) const {
    big_int copy = *this;
    copy %= other;
    return copy;
}

big_int big_int::operator&(const big_int &other) const {
    big_int copy = *this;
    copy &= other;
    return copy;
}

big_int big_int::operator|(const big_int &other) const {
    big_int copy = *this;
    copy |= other;
    return copy;
}

big_int big_int::operator^(const big_int &other) const {
    big_int copy = *this;
    copy ^= other;
    return copy;
}

big_int big_int::operator<<(size_t shift) const {
    big_int copy = *this;
    copy <<= shift;
    return copy;
}

big_int big_int::operator>>(size_t shift) const {
    big_int copy = *this;
    copy >>= shift;
    return copy;
}

big_int &big_int::operator%=(const big_int &other) & {
    return ((*this) -= ((*this / other) * other));
}

big_int big_int::operator~() const {
    auto res = *this;
    res._sign = !res._sign;
    for (auto &num: res._digits) {
        num = ~num;
    }
    res.optimise();
    return res;
}

big_int &big_int::operator&=(const big_int &other) & {
    if (!_sign && other._sign)
        _sign = true;

    for (size_t i = 0; i < _digits.size(); ++i)
        _digits[i] &= i < other._digits.size() ? other._digits[i] : 0;

    optimise();
    return *this;
}

big_int &big_int::operator|=(const big_int &other) & {
    if (_sign && !other._sign)
        _sign = false;

    if (_digits.size() < other._digits.size())
        _digits.resize(other._digits.size(), 0);

    for (size_t i = 0; i < _digits.size(); ++i)
        _digits[i] |= other._digits[i];

    optimise();
    return *this;
}

big_int &big_int::operator^=(const big_int &other) & {
    if (_sign != other._sign)
        _sign = false;
    else
        _sign = true;

    if (_digits.size() < other._digits.size())
        _digits.resize(other._digits.size(), 0);

    for (size_t i = 0; i < _digits.size(); ++i)
        _digits[i] ^= other._digits[i];

    optimise();
    return *this;
}

big_int &big_int::operator<<=(size_t shift) & {
    if (shift / (8 * sizeof(unsigned int)) > 0) {
        const size_t n = shift / (8 * sizeof(unsigned int));

        std::vector<unsigned int> vec(n, 0);

        _digits.insert(_digits.begin(), vec.begin(), vec.end());

        shift %= 8 * sizeof(unsigned int);
    }

    if (shift == 0)
        return *this;

    unsigned int c = 0;

    for (auto &num: _digits) {
        const auto tmp = num;
        // сдвиг влево + добавление перенесенных старших битов
        num = (num << shift) | c;
        // запоминаем верхние "непоместившиеся" биты
        c = tmp >> (8 * sizeof(unsigned int) - shift);
    }

    if (c != 0)
        _digits.push_back(c);

    optimise();
    return *this;
}

big_int &big_int::operator>>=(size_t shift) & {
    if (shift / (8 * sizeof(unsigned int)) > 0) {
        const size_t n = shift / (8 * sizeof(unsigned int));

        if (n > _digits.size()) {
            _digits.clear();
            _sign = true;
            return *this;
        }
        _digits.erase(_digits.begin(), _digits.begin() + n);

        shift %= 8 * sizeof(unsigned int);
    }

    if (shift == 0) {
        return *this;
    }
    unsigned int c = 0;

    for (auto &num: _digits) {
        const auto tmp = num;

        num = (num >> shift) | c;
        c = tmp << (8 * sizeof(unsigned int) - shift);
    }
    optimise();
    return *this;
}

big_int &big_int::plus_assign(const big_int &other, size_t shift) & {
    if (this->_sign == other._sign) {
        plus_operation_without_sign(*this, other, shift);
        return *this;
    }
    // знаки разные
    // 1 случай this >= other
    // Знак остается от this
    auto result = modulo_comparison(*this, other);
    if (result == std::strong_ordering::greater || result == std::strong_ordering::equal) {
        minus_operation_without_sign(*this, other, shift);
        return *this;
    }

    // 2 случай this < other
    big_int copy = other;
    minus_operation_without_sign(copy, *this, shift);
    *this = copy;
    return *this;
}

big_int &big_int::minus_assign(const big_int &other, size_t shift) & {
    return plus_assign(-other, shift);
}

big_int &big_int::operator*=(const big_int &other) & {
    *this = std::move((*this) * other);
    optimise();
    return *this;
}

big_int &big_int::operator/=(const big_int &other) & {
    big_int res = *this / other;
    *this = std::move(res);
    optimise();
    return *this;
}

std::string big_int::to_string() const {
    if (is_zero()) {
        return "0";
    }

    big_int copy = *this;
    std::string result;

    // делаем положительной копию
    copy._sign = true;
    const big_int ten{10};
    while (!copy.is_zero()) {
        big_int remain;
        remain = copy % ten;
        remain.optimise();
        copy /= ten;
        copy.optimise();
        result += remain.is_zero() ? "0" : std::to_string(remain._digits[0]);
    }
    if (_sign == false) {
        result += '-';
    }
    std::ranges::reverse(result);
    return result;
}

std::ostream &operator<<(std::ostream &stream, const big_int &value) {
    stream << value.to_string();
    return stream;
}

std::istream &operator>>(std::istream &stream, big_int &value) {
    std::string string_number;
    stream >> string_number;
    value.get_new_value_from_string(string_number, 10);
    return stream;
}

bool big_int::operator==(const big_int &other) const noexcept {
    return ((*this <=> other) == std::strong_ordering::equal);
}

big_int::big_int(const std::vector<unsigned int, pp_allocator<unsigned int> > &digits, bool sign) : _sign(sign),
    _digits(digits) {
    optimise();
}

big_int::big_int(const std::initializer_list<unsigned int> init, const bool sign) noexcept
    : _sign(sign) {
    for (unsigned int it: init) {
        _digits.push_back(it);
    }
    optimise();
}

big_int::big_int(std::vector<unsigned int, pp_allocator<unsigned int> > &&digits, bool sign) noexcept : _sign(sign), _digits(std::move(digits)) {
    optimise();
}

void big_int::get_new_value_from_string(const std::string &num, unsigned int radix) {
    if (radix > 36 || radix < 2)
        throw std::invalid_argument("Radix must be in [2, 36], but is " + std::to_string(radix));
    // Сохраняем все в копию, чтобы избежать потери изначального значения при ошибках
    big_int result;
    auto it = num.begin();
    // пустая строчка
    if (it == num.end()) {
        return;
    }
    bool sign = true;
    if (*it == '-') {
        sign = false;
        ++it;
    } else if (*it == '+') {
        ++it;
    }

    for (; it != num.end(); ++it) {
        const char c = *it;
        unsigned int digit;
        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (std::tolower(c) >= 'a' && std::tolower(c) <= 'z') {
            digit = std::tolower(c) - 'a' + 10;
        } else {
            throw std::invalid_argument(
                "Symbol " + std::to_string(c) + " is not a valid character in radix " + std::to_string(radix));
        }

        if (digit >= radix) {
            throw std::invalid_argument(
                "Symbol " + std::to_string(c) + " is not a valid character in radix " + std::to_string(radix));
        }

        big_int radix_big_int({radix}, true);
        result *= radix_big_int;
        result += big_int({digit});
    }
    result._sign = sign;
    optimise();
    *this = std::move(result);
}

big_int::big_int(const std::string &num, unsigned int radix, pp_allocator<unsigned int>) : _sign(true), _digits() {
    get_new_value_from_string(num, radix);
}

big_int::big_int(const big_int &other) : _sign(other._sign), _digits(other._digits) {
    optimise();
}

big_int::big_int(big_int &&other) noexcept : _sign(other._sign), _digits(std::move(other._digits)) {
    other._sign = true;
    other._digits.clear();
    optimise();
}

big_int &big_int::operator=(big_int &&other) noexcept {
    if (this == &other) {
        return *this;
    }
    _digits.clear();

    _sign = other._sign;
    _digits = std::move(other._digits);

    other._sign = true;
    other._digits.clear();

    optimise();
    return *this;
}

big_int &big_int::operator=(const big_int &other) {
    if (this == &other) {
        return *this;
    }

    _sign = other._sign;
    _digits.clear();
    _digits = other._digits;
    optimise();
    return *this;
}


big_int::big_int(pp_allocator<unsigned int>) : _sign(true) {
    _digits = std::vector<unsigned int, pp_allocator<unsigned int> >();
    optimise();
}

big_int &big_int::multiply_assign(const big_int &other, big_int::multiplication_rule rule) & {
    if (rule == multiplication_rule::trivial) {
        *this = std::move(multiply_table(*this, other));
    } else if (rule == multiplication_rule::Karatsuba) {
        *this = std::move(multiply_karatsuba(*this, other));
    } else if(rule == multiplication_rule::SchonhageStrassen){
        *this = std::move(multiply_schonhage_strassen(*this, other));
    }
    return *this;
}

big_int &big_int::divide_assign(const big_int &other, big_int::division_rule rule) & {
    if (rule == division_rule::trivial) {
        *this = std::move(divide_table(*this, other));
    }
    return *this;
}

big_int &big_int::modulo_assign(const big_int &other, big_int::division_rule rule) & {
    // return ((*this) -= ((*this / other) * other));
    if (rule == division_rule::trivial) {
        *this -= (divide_table(*this, other) * other);
    }

    return *this;
}

big_int big_int::operator-() const {
    big_int copy = *this;
    copy._sign = !copy._sign;

    return copy;
}


big_int operator""_bi(unsigned long long n) {
    unsigned int first_digit = n & 0xFFFFFFFFU;
    unsigned int second_digit = n >> 32;
    return big_int({first_digit, second_digit});
}

void big_int::optimise() noexcept {
    while (!_digits.empty() && _digits.back() == 0) {
        _digits.pop_back();
    }
    if (_digits.empty()) {
        _sign = true;
    }
}

bool big_int::is_negative() const noexcept {
    return this->_sign == 0;
}

bool big_int::is_positive() const noexcept {
    return this->_sign == 1;
}

bool big_int::is_zero() const noexcept {
    return _digits.empty();
}

big_int big_int::abs() const {
    big_int result = *this;
    result._sign = true;
    return result;
}



uint64_t big_int::ss_power(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t res = 1;
    base %= mod;
    while (exp > 0) {

        if (exp % 2 == 1) {
            res = (res * base) % mod;
        }

        base = (base * base) % mod;
        exp /= 2;
    }
    return res;
}

uint64_t big_int::ss_modInverse(uint64_t n, uint64_t mod) {
    return ss_power(n, mod - 2, mod);
}

void big_int::ss_ntt_transform(std::vector<uint64_t>& data, uint64_t mod, uint64_t root_val_for_L, bool inverse) {
    size_t n = data.size();
    if (n == 0) return;

    // Битовая инверсия
    for (size_t i = 1, j = 0; i < n; i++) {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j)
            std::swap(data[i], data[j]);
    }

    for (size_t len = 2; len <= n; len <<= 1) {
        uint64_t wlen = root_val_for_L;
        // wlen = g * (N / len) mod
        for (size_t temp_n = n; temp_n > len; temp_n >>= 1) {
            wlen = (wlen * wlen) % mod;
        }
        // Идем блоками len
        for (size_t i = 0; i < n; i += len) {
            uint64_t w = 1;
            for (size_t j = 0; j < len / 2; j++) {
                uint64_t u = data[i + j];
                uint64_t v = (data[i + j + len / 2] * w) % mod;
                data[i + j] = u + v;
                if (data[i + j] >= mod) data[i + j] -= mod;
                data[i + j + len / 2] = u - v + mod;
                if (data[i + j + len / 2] >= mod) data[i + j + len / 2] -= mod;
                w = (w * wlen) % mod;
            }
        }
    }

    if (inverse) {
        uint64_t n_inv = ss_modInverse(n, mod);
        for (size_t i = 0; i < n; i++) {
            data[i] = (data[i] * n_inv) % mod;
        }
    }
}

uint64_t big_int::ss_solve_crt_two_moduli(uint64_t val1_mod_m1, uint64_t val2_mod_m2) {
    // Объединить два остатка по разным модулям в одно число по произведению модулей
    uint64_t diff = val2_mod_m2 - (val1_mod_m1 % SS_MOD2);
    if (val2_mod_m2 < (val1_mod_m1 % SS_MOD2)) {
        diff += SS_MOD2;
    }
    // h = diff * M_1 ^ -1 (mod_2)
    uint64_t h = (diff * get_m1_inv_m2()) % SS_MOD2;
    uint64_t result_val = val1_mod_m1 + (h * SS_MOD1);
    uint64_t term_h_m1 = (h * SS_MOD1);
    uint64_t final_sum = val1_mod_m1 + term_h_m1;

    return final_sum % get_m_product();
}


big_int big_int::ss_multiply_core_crt(const big_int& a, const big_int& b, pp_allocator<unsigned int> alloc) {
    const int m_bits_per_chunk = 20;

    size_t a_total_bits = 0;
    if (!a.is_zero()) {
        a_total_bits = (a._digits.size() - 1) * std::numeric_limits<unsigned int>::digits;
        if (!a._digits.empty()) {
            a_total_bits += (std::numeric_limits<unsigned int>::digits - __detail::countl_zero_uint_impl(a._digits.back()));
        }
    }

    size_t b_total_bits = 0;
    if (!b.is_zero()) {
        b_total_bits = (b._digits.size() - 1) * std::numeric_limits<unsigned int>::digits;
        if (!b._digits.empty()) {
            b_total_bits += (std::numeric_limits<unsigned int>::digits - __detail::countl_zero_uint_impl(b._digits.back()));
        }
    }
    if (a_total_bits == 0 || b_total_bits == 0) {
        return big_int(alloc);
    }

    size_t num_chunks_a = (a_total_bits + m_bits_per_chunk - 1) / m_bits_per_chunk;
    size_t num_chunks_b = (b_total_bits + m_bits_per_chunk - 1) / m_bits_per_chunk;
    if (a_total_bits > 0 && num_chunks_a == 0) num_chunks_a = 1;
    if (b_total_bits > 0 && num_chunks_b == 0) num_chunks_b = 1;
    size_t required_coeffs = num_chunks_a + num_chunks_b - 1;
    if (required_coeffs == 0) required_coeffs = 1;
    size_t L = __detail::nearest_greater_power_of_2(required_coeffs);
    if (L > (1u << 21)) {
        throw std::overflow_error("Schonhage-Strassen+CRT: L too large");
    }
    auto get_coeffs_lambda =
    [&](const big_int& num, size_t num_chunks_to_extract, const std::string& num_name) {
        std::vector<uint64_t> poly(L, 0);

        unsigned int current_digit_idx = 0;
        int bits_in_current_digit_processed = 0;  // Сколько бит уже взяли из текущего слова
        unsigned int current_unsigned_int_val = 0;
        if (!num._digits.empty()) {
            current_unsigned_int_val = num._digits[0];
        }

        for (size_t chunk_idx = 0; chunk_idx < num_chunks_to_extract; ++chunk_idx) {
            if (chunk_idx >= L) break;

            uint64_t current_chunk_val = 0;
            int bits_to_collect_for_chunk = m_bits_per_chunk; // Сколько бит нужно для блока (изначально 20)
            int current_chunk_bit_pos = 0;

            // Собираем 20 бит для текущего блока, по частям из слов
            while (bits_to_collect_for_chunk > 0) {
                if (current_digit_idx >= num._digits.size()) break;

                int bits_available_in_uint = std::numeric_limits<unsigned int>::digits - bits_in_current_digit_processed;
                int bits_to_take_now = std::min(bits_to_collect_for_chunk, bits_available_in_uint);

                unsigned int extracted_part = (current_unsigned_int_val >> bits_in_current_digit_processed) & ((1u << bits_to_take_now) - 1);
                current_chunk_val |= (static_cast<uint64_t>(extracted_part) << current_chunk_bit_pos);

                bits_in_current_digit_processed += bits_to_take_now;
                bits_to_collect_for_chunk -= bits_to_take_now;
                current_chunk_bit_pos += bits_to_take_now;

                if (bits_in_current_digit_processed >= std::numeric_limits<unsigned int>::digits) {
                    current_digit_idx++;
                    bits_in_current_digit_processed = 0;
                    if (current_digit_idx < num._digits.size()) {
                        current_unsigned_int_val = num._digits[current_digit_idx];
                    }
                }
            }
            poly[chunk_idx] = current_chunk_val;
        }
        return poly;
    };


    std::vector<uint64_t> poly_a_coeffs = get_coeffs_lambda(a, num_chunks_a, "a");
    std::vector<uint64_t> poly_b_coeffs = get_coeffs_lambda(b, num_chunks_b, "b");

    // NTT для MOD1
    std::vector<uint64_t> p_a_ntt1 = poly_a_coeffs;
    std::vector<uint64_t> p_b_ntt1 = poly_b_coeffs;
    uint64_t omega_L_mod1 = ss_power(SS_ROOT1, (SS_MOD1 - 1) / L, SS_MOD1);
    ss_ntt_transform(p_a_ntt1, SS_MOD1, omega_L_mod1, false);
    ss_ntt_transform(p_b_ntt1, SS_MOD1, omega_L_mod1, false);
    std::vector<uint64_t> p_c_ntt1(L);
    for (size_t i = 0; i < L; ++i) p_c_ntt1[i] = (p_a_ntt1[i] * p_b_ntt1[i]) % SS_MOD1;
    uint64_t inv_omega_L_mod1 = ss_modInverse(omega_L_mod1, SS_MOD1);
    ss_ntt_transform(p_c_ntt1, SS_MOD1, inv_omega_L_mod1, true);

    // NTT для MOD2
    std::vector<uint64_t> p_a_ntt2 = poly_a_coeffs;
    std::vector<uint64_t> p_b_ntt2 = poly_b_coeffs;
    uint64_t omega_L_mod2 = ss_power(SS_ROOT2, (SS_MOD2 - 1) / L, SS_MOD2);
    ss_ntt_transform(p_a_ntt2, SS_MOD2, omega_L_mod2, false);
    ss_ntt_transform(p_b_ntt2, SS_MOD2, omega_L_mod2, false);
    std::vector<uint64_t> p_c_ntt2(L);
    for (size_t i = 0; i < L; ++i) p_c_ntt2[i] = (p_a_ntt2[i] * p_b_ntt2[i]) % SS_MOD2;
    uint64_t inv_omega_L_mod2 = ss_modInverse(omega_L_mod2, SS_MOD2);
    ss_ntt_transform(p_c_ntt2, SS_MOD2, inv_omega_L_mod2, true);

    big_int final_result(alloc);
    std::vector<uint64_t> final_result_coeffs_vec(L);
    for (size_t i = 0; i < L; ++i) {
        final_result_coeffs_vec[i] = ss_solve_crt_two_moduli(p_c_ntt1[i], p_c_ntt2[i]);
    }

    for(size_t i = 0; i < L; ++i) {
        if (final_result_coeffs_vec[i] == 0 && i >= (num_chunks_a + num_chunks_b -1) ) {
            continue;
        }

        if (final_result_coeffs_vec[i] > 0) {
            big_int term(final_result_coeffs_vec[i], alloc);
            term <<= (i * m_bits_per_chunk);
            final_result += term;
        }
    }
    final_result.optimise();
    return final_result;
}


big_int big_int::multiply_schonhage_strassen(const big_int& left, const big_int& right) {

    pp_allocator<unsigned int> common_alloc = left._digits.get_allocator();

    if (left.is_zero() || right.is_zero()) {
        return big_int(common_alloc);
    }

    bool result_sign_is_positive = (left._sign == right._sign);

    big_int abs_left_arg = left.abs();

    big_int abs_right_arg = right.abs();

    big_int result_val = ss_multiply_core_crt(abs_left_arg, abs_right_arg, common_alloc);

    if (!result_val.is_zero()) {
        result_val._sign = result_sign_is_positive;
    } else {
        result_val._sign = true;
    }
    result_val.optimise();
    return result_val;
}
