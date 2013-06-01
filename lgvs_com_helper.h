/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */


#ifndef LGVS_COM_HELPER_H
#define LGVS_COM_HELPER_H


#include <unknwn.h>


namespace lgvs {


class ComHelper {
public:
    template<class T>
    static void release_and_null (T*& object)
    {
        if (object == NULL)
            return;

        object->Release ();
        object = NULL;
    }


private:
    ComHelper ();
    ComHelper (const ComHelper& that);
    ~ComHelper ();
    ComHelper& operator = (const ComHelper& that);
}; // class ComHelper


} // namespace lgvs


#endif // LGVS_COM_HELPER_H
