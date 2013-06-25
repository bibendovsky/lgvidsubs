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


#ifndef LGVS_DETOUR_H
#define LGVS_DETOUR_H


#include "lgvs_trampoline_cache.h"


namespace lgvs {


class Detour {
public:
    Detour();
    ~Detour();

    bool initialize(
        void* source_function,
        void* detour_function,
        size_t code_size,
        TrampolineCache* trampoline_cache);

    void uninitialize();

    bool is_initialized() const;

    const void* get_trampoline() const;


private:
    bool is_initialized_;
    void* source_function_;
    void* detour_function_;
    void* trampoline_;
    size_t code_size_;
    TrampolineCache* trampoline_cache_;

    Detour(const Detour& that);
    Detour& operator=(const Detour& that);
}; // class Detour


} // namespace lgvs


#endif // LGVS_DETOUR_H
