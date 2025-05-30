#include "../include/fraction.h"
#include "../../big_integer/include/big_int.h"
#include <math.h>

static const fraction PI(245850922_bi, 78256779_bi);
static const fraction HALF_PI = PI / fraction(2_bi, 1_bi);


big_int gcd(big_int a, big_int b) {
    a = a.abs();
    b = b.abs();
    while (!b.is_zero() && !a.is_zero()) {
        if ((a <=> b) == std::strong_ordering::greater) {
            a %= b;
        } else {
            b %= a;
        }
    }
    return a + b;
}



void fraction::optimise() {
    if (_denominator.is_zero())
        throw std::invalid_argument("Denominator cannot be zero");

    // Eсли дробь отрицательная, знак должен быть у знаменателя
    bool is_negative_fraction = _numerator.is_negative() != _denominator.is_negative();

    _numerator = _numerator.abs();
    _denominator = _denominator.abs();

    if (is_negative_fraction)
        _denominator = -_denominator;

    big_int g = gcd(_numerator, _denominator);
    _numerator /= g;
    _denominator /= g;
}


fraction fraction::abs() const {
    return fraction(_numerator, _denominator < 0_bi ? -_denominator : _denominator);
}


template<std::convertible_to<big_int> f, std::convertible_to<big_int> s>
fraction::fraction(f &&numerator, s &&denominator)
        : _numerator(std::forward<f>(numerator)),
          _denominator(std::forward<s>(denominator))
{
    optimise();
}

fraction::fraction(pp_allocator<big_int::value_type> alloc)
        : _numerator(big_int(alloc)),
          _denominator(big_int(alloc))
{}


fraction &fraction::operator+=(fraction const &other) & {
     // Ищем наибольшее общее кратное
     big_int g = gcd(_denominator, other._denominator);
     big_int lcm = (_denominator * other._denominator) / g;

     // Минимальные множители, на которые надо домножить числители
     big_int mul_this = lcm / _denominator;
     big_int mul_other = lcm / other._denominator;

     _numerator = _numerator * mul_this + other._numerator * mul_other;
     _denominator = lcm;

     optimise();
     return *this;
}


fraction fraction::operator+(fraction const &other) const {
    fraction result = *this;
    result += other;
    return result;
}



fraction &fraction::operator-=(fraction const &other) & {
    // Ищем наибольшее общее кратное
    big_int g = gcd(_denominator, other._denominator);
    big_int lcm = (_denominator * other._denominator) / g;

    // Минимальные множители, на которые надо домножить числители
    big_int mul_this = lcm / _denominator;
    big_int mul_other = lcm / other._denominator;

    _numerator = _numerator * mul_this - other._numerator * mul_other;
    _denominator = lcm;

    optimise();
    return *this;
}

fraction fraction::operator-(fraction const &other) const {
    fraction result = *this;
    result -= other;
    return result;
}


fraction &fraction::operator*=(fraction const &other) & {
    _numerator *= other._numerator;
    _denominator *= other._denominator;
    optimise();
    return *this;
}

fraction fraction::operator*(fraction const &other) const {
    fraction result = *this;
    result *= other;
    return result;
}

fraction &fraction::operator/=(fraction const &other) & {
    if (other._numerator == 0_bi) {
        throw std::invalid_argument("Division by zero");
    }
    _numerator *= other._denominator;
    _denominator *= other._numerator;
    optimise();
    return *this;
}

fraction fraction::operator/(fraction const &other) const {
    fraction result = *this;
    result /= other;
    return result;
}

fraction fraction::operator-() const {
    return fraction(_numerator, -_denominator);
}


bool fraction::operator==(fraction const &other) const noexcept {
    return _numerator == other._numerator && _denominator == other._denominator;
}

std::partial_ordering fraction::operator<=>(fraction const &other) const noexcept {
     big_int lhs = _numerator * other._denominator.abs();
     if (_denominator.is_negative()) {
         lhs._sign = false;
     }
     big_int rhs = other._numerator * _denominator.abs();
     if (other._denominator.is_negative()) {
         rhs._sign = false;
     }

     return lhs <=> rhs;
 }



std::ostream &operator<<(std::ostream &stream, fraction const &obj) {
    if (obj._numerator == 0_bi) {
        stream << "0";
    } else {
        if (obj._denominator < 0_bi)
            stream << "-";
        stream << obj._numerator.abs() << "/" << obj._denominator.abs();
    }
    return stream;
}

std::istream &operator>>(std::istream &stream, fraction &obj) {
    std::string token;
    stream >> token;

    std::size_t slash_pos = token.find('/');

    try {
        if (slash_pos == std::string::npos) {
            // Только числитель
            big_int num(token);
            obj = fraction(num, 1_bi);
        } else {
            // Дробь вида a/b
            std::string num_str = token.substr(0, slash_pos);
            std::string den_str = token.substr(slash_pos + 1);

            // Знаменатель пустой (например, 5/)
            if (den_str.empty()) {
                stream.setstate(std::ios::failbit);
                return stream;
            }

            big_int num(num_str);
            big_int den(den_str);

            obj = fraction(num, den);
        }
    } catch (...) {
        stream.setstate(std::ios::failbit);
    }

    return stream;
}
std::string fraction::to_string() const {
    std::string result;

    if (_numerator.is_zero()) {
        return "0/1";
    }

    if (_denominator.is_negative()) {
        result += "-";
    }

    result += _numerator.abs().to_string();
    result += "/";
    result += _denominator.abs().to_string();

    return result;
}


fraction fraction::sin(fraction const &epsilon) const {

    // sin(x) = x - x^3 / 3! + x^5 / 5! - ...
    fraction zero(0_bi, 1_bi);
    fraction one(1_bi, 1_bi);
    fraction two(2_bi, 1_bi);



    if (epsilon <= zero)
    {
         throw std::invalid_argument("Epsilon must be greater than 0");
    }

    fraction x(*this);

    while (x > PI)
        x -= fraction(2_bi, 1_bi) * PI;
    while (x < -PI)
        x += fraction(2_bi, 1_bi) * PI;

    fraction current = x;
    fraction sum = current;
    fraction n = one;

    do
    {
         fraction tmp = two * n;
         current *= -(x * x) / (tmp * (tmp + one));;
         sum += current;
         n += one;
    }
    while(current.abs() >= epsilon);
    return sum;
 }

fraction fraction::cos(fraction const &epsilon) const {
    fraction zero(0_bi, 1_bi);
    fraction one(1_bi, 1_bi);
    fraction two(2_bi, 1_bi);

    if (epsilon <= zero) {
        throw std::invalid_argument("Epsilon must be greater than 0");
    }

    fraction x(*this);

    while (x > PI)
        x -= fraction(2_bi, 1_bi) * PI;
    while (x < -PI)
        x += fraction(2_bi, 1_bi) * PI;
    fraction current = one;
    fraction sum = current;
    fraction n = one;
    do {
        fraction tmp = two * n;
        current *= -(x * x) / (tmp * (tmp - one));
        sum += current;
        n += one;
    } while (current.abs() >= epsilon);

    return sum;
}


fraction fraction::tg(const fraction &epsilon) const {
    fraction s = this->sin(epsilon);
    fraction c = this->cos(epsilon);

    if (c.abs() < epsilon) {
        throw std::invalid_argument("tg(x): cos(x) is too close to zero");
    }

    return s / c;
}


fraction fraction::ctg(const fraction &epsilon) const {
    fraction s = this->sin(epsilon);
    fraction c = this->cos(epsilon);

    if (s.abs() < epsilon) {
        throw std::invalid_argument("ctg(x): sin(x) is too close to zero");
    }

    return c / s;
}


fraction fraction::sec(const fraction &epsilon) const {
    fraction c = this->cos(epsilon);

    if (c.abs() < epsilon) {
        throw std::invalid_argument("sec(x): cos(x) is too close to zero");
    }

    return fraction(1_bi, 1_bi) / c;
}



fraction fraction::cosec(const fraction &epsilon) const {
    fraction s = this->sin(epsilon);

    if (s.abs() < epsilon) {
        throw std::invalid_argument("cosec(x): sin(x) is too close to zero");
    }

    return fraction(1_bi, 1_bi) / s;
}



fraction fraction::pow(size_t degree) const {
    fraction zero(0_bi, 1_bi);
    fraction one(1_bi, 1_bi);

    if (*this == zero && degree == 0) {
        throw std::invalid_argument("Attempt to raise zero in power zero");
    }

    if (degree == 0) {
        return one;
    }

    fraction value = *this;
    fraction tmp_res = one;

    while (degree > 0) {
        if (degree & 1) {
            tmp_res *= value;
        }

        value *= value;
        degree /= 2;
    }

    return tmp_res;
}

fraction fraction::root(size_t degree,fraction const &epsilon) const {
    // Метод Ньютона: x_{k+1}  = (1 / n) * ((n - 1) * x_k + a / x_k ^ n - 1
    // a - число, из которого извлекаем корень, x_k - текущее приближение
     if (degree == 0)
     {
         throw std::invalid_argument("Root degree cannot be zero");
     }
     if (degree == 1)
     {
         return *this;
     }

     fraction zero(0_bi, 1_bi);
     fraction one(1_bi, 1_bi);
     fraction two(2_bi, 1_bi);
     fraction fraction_degree(big_int(std::to_string(degree)), 1_bi);
     fraction tmp_fraction = *this;


     int sign = 1;
     int l = 0;

     if (tmp_fraction < zero)
     {
         if (degree & 1)
         {
             sign = -1;
             tmp_fraction *= -one;
         }
         else
         {
             throw std::invalid_argument("Cannot take odd degree root of negative number");
         }
     }

    // Начальное приближение - среднее арифметическое между 1 и a
     fraction current = (one + tmp_fraction) / two;
     fraction next = current;

     fraction one_by_n(1_bi, big_int(std::to_string(degree)));

     fraction prev_degree = fraction_degree - one;

     do
     {
         current = next;
         // Формула Ньютона
         next = one_by_n * (prev_degree * current + tmp_fraction / current.pow(degree - 1));
     }
     while ((next - current).abs() >= epsilon);

     if (sign == -1)
     {
         return -next;
     }
     return next;
}

fraction fraction::log2(const fraction &epsilon) const {
    // log b (x) == ln(x) / ln(b) => log2(x) = ln(x) / ln(2)
    fraction ln2 = fraction(2_bi, 1_bi).ln(epsilon);
    return this->ln(epsilon) / ln2;
}

fraction fraction::ln(const fraction &epsilon) const {
     // для ln(1 + x) = x - x^2 / 2 + ...
     // если вызываем ln(5) и считаем ln(1 + 4) - очень медленно и неэффективно
     // ln(x) = kln(2) * ln(a), a э (0,5; 1,5]
     // y = (a-1)/(a+1)
     // ln(a) = 2 * (y + y^3/3 + y^5/5 + ...)

     static const fraction zero(0_bi, 1_bi);
     static const fraction one(1_bi, 1_bi);
     static const fraction two(2_bi, 1_bi);

     if (*this <= zero)
         throw std::invalid_argument("Cannot take logarithm of non-positive number");

     if (*this < one) {
         return (-one) * (one / *this).ln(epsilon); // логарифм обратного: ln(a) = -ln(1/a)
     }

     if (*this > two) {
         fraction m = one;
         fraction dividend = *this;

         while ((dividend /= two) > one)
             m += one;

         return dividend.ln(epsilon) + m * two.ln(epsilon);
     }

     if (epsilon <= zero)
         throw std::invalid_argument("Epsilon must be greater than 0");

     fraction x = (*this - one) / (*this + one);
     fraction current = x;
     fraction sum = x;
     fraction x_squared = x * x;
     fraction n = one;

     while (current.abs() >= epsilon) {
         n += two;
         current = current * x_squared;
         fraction term = current / n;
         sum += term;
     }

     return sum * two;
 }



fraction fraction::lg(fraction const &epsilon) const
{
    // log_b(x) = ln(x) / ln(b) => log_10(x) = ln(x) / ln(10)
     return ln(epsilon) / fraction(10_bi, 1_bi).ln(epsilon);
}

fraction fraction::arctg(fraction const &epsilon) const
{
    // arctg(x) == arcsin( x / root(1 + x ^ 2)
    return (*this / ((fraction(1_bi, 1_bi)) + pow(2)).root(2, epsilon)).arcsin(epsilon);
}

fraction fraction::arcctg(fraction const &epsilon) const {
    // arctg(x) == arccos( x / root(1 + x ^ 2)
    return (*this / ((fraction(1_bi, 1_bi)) + pow(2)).root(2, epsilon)).arccos(epsilon);
}

fraction fraction::arcsin(fraction const &epsilon) const
{
    // Рекуррентная формула: a_{n+1} == a_n *  ((2 * n - 1) ^ 2 / ((2 * n) * (2 * n + 1))) * x ^ 2
    fraction zero(0_bi, 1_bi);
    fraction one(1_bi, 1_bi);
    fraction two(2_bi, 1_bi);

    if (epsilon <= zero)
    {
        throw std::invalid_argument("Epsilon must greater than 0");
    }

    if (abs() > one)
    {
        throw std::invalid_argument("Module of number must be not greater than 1");
    }

    if (abs() > fraction(4_bi, 5_bi))
    {
        // if x > 0,8 : arcsin(x) = pi/2 − arcsin(root(1 − x ^ 2))
        return PI / two - (one - pow(2)).root(2,epsilon).arcsin(epsilon);
    }

    fraction const &x = *this;
    fraction current = x;
    fraction sum = current;
    fraction n = one;

    do
    {
        fraction tmp = two * n;
        current *= (tmp - one).pow(2) * x * x / (tmp * (tmp + one));
        sum += current;
        n += one;
    }
    while(current.abs() >= epsilon);

    return sum;
}


fraction fraction::arccos(const fraction &epsilon) const {
    return HALF_PI - this->arcsin(epsilon);
}


fraction fraction::arcsec(const fraction &epsilon) const {
    // arcsec(x) = arccos(1 / x)
    return (fraction(1_bi, 1_bi) / *this).arccos(epsilon);
}

fraction fraction::arccosec(const fraction &epsilon) const {
    // arccosec(x) = arcsin(1 / x)
    return (fraction(1_bi, 1_bi) / *this).arcsin(epsilon);
}
