#ifndef BBI_COM_HELPER_H
#define BBI_COM_HELPER_H


#include <unknwn.h>


namespace bbi {


class ComHelper {
public:
    template<class T>
    static void release_and_null (T*& object)
    {
        if (object == 0)
            return;

        object->Release ();
        object = 0;
    }


private:
    ComHelper ();
    ComHelper (const ComHelper& that);
    ~ComHelper ();
    ComHelper& operator = (const ComHelper& that);
}; // class ComHelper


} // namespace bbi


#endif // BBI_COM_HELPER_H
