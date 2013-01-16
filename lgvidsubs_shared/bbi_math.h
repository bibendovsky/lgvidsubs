#ifndef BBI_MATH_H
#define BBI_MATH_H


#ifdef _WIN32
    #ifdef max
        #undef max
    #endif
#endif

#include <algorithm>


namespace bbi {


class Math {
public:
    template<class T>
    static T clamp (T value, T min_value, T max_value)
    {
        return std::max ((std::min) (value, max_value), min_value);
    }


private:
    Math ();

    Math (const Math& that);

    Math& operator = (const Math& that);

    ~Math ();
}; // class Math


} // namespace bbi


#endif // BBI_MATH_H
