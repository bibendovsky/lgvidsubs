#ifndef BBI_MATH_H
#define BBI_MATH_H


#include <algorithm>


namespace bbi {


    class Math {
    public:
        template<class T>
        static T clamp (
            T value,
            T minValue,
            T maxValue)
        {
            return (std::max) ((std::min) (value, maxValue), minValue);
        }


    private:
        Math ();

        Math (
            const Math& that);

        Math& operator = (
            const Math& that);

        ~Math ();
    }; // class Math


} // namespace bbi


#endif // BBI_MATH_H
