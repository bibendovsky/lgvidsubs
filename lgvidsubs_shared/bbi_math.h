#ifndef BBI_MATH_H
#define BBI_MATH_H


#include <algorithm>


namespace bbi {


class Math {
public:
    template<class T>
    static T clamp (T value, T min_value, T max_value)
    {
        return std::max (std::min (value, max_value), min_value);
    }


private:
    Math ();
    Math (const Math& that);
    ~Math ();
    Math& operator = (const Math& that);
}; // class Math


} // namespace bbi


#endif // BBI_MATH_H
