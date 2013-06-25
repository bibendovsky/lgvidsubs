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


#include "lgvs_detour.h"

#include <stdint.h>
#include <cstddef>

#include <algorithm>

#include <windows.h>


namespace lgvs {


Detour::Detour() :
    is_initialized_(false),
    source_function_(nullptr),
    detour_function_(nullptr),
    trampoline_(nullptr),
    code_size_(0),
    trampoline_cache_(nullptr)
{
}

Detour::~Detour()
{
    uninitialize();
}

bool Detour::initialize(
    void* source_function,
    void* detour_function,
    size_t code_size,
    TrampolineCache* trampoline_cache)
{
    uninitialize();

    if (source_function == nullptr)
        return false;

    if (detour_function == nullptr)
        return false;

    if (trampoline_cache == nullptr)
        return false;

    if (!trampoline_cache->is_initialized())
        return false;

    if (code_size < 5 || code_size + 5 > trampoline_cache->get_max_item_size())
        return false;

    auto trampoline = static_cast<uint8_t*>(trampoline_cache->allocate());

    if (trampoline == nullptr)
        return false;

    auto src_func = static_cast<uint8_t*>(source_function);
    auto det_func = static_cast<uint8_t*>(detour_function);

    std::uninitialized_copy(
        &src_func[0],
        &src_func[code_size],
        trampoline);

    DWORD old_protect = 0;
    BOOL api_result = FALSE;

    api_result = ::VirtualProtect(
        src_func,
        5,
        PAGE_READWRITE,
        &old_protect);

    if (api_result == FALSE)
        return false;

    uint32_t jump_distance;

    jump_distance = det_func - &src_func[5];

    src_func[0] = 0xE9; // jmp [rel32]
    *reinterpret_cast<UINT32*>(&src_func[1]) = jump_distance;

    api_result = ::VirtualProtect(
        src_func,
        5,
        old_protect,
        &old_protect);

    jump_distance = &src_func[code_size] - &trampoline[code_size + 5];

    trampoline[code_size] = 0xE9; // jmp [rel32]
    *reinterpret_cast<uint32_t*>(&trampoline[code_size + 1]) = jump_distance;

    auto current_process = ::GetCurrentProcess();

    api_result = ::FlushInstructionCache(
        current_process,
        src_func,
        5);

    api_result = ::FlushInstructionCache(
        current_process,
        trampoline,
        trampoline_cache->get_max_item_size());

    is_initialized_ = true;
    source_function_ = src_func;
    detour_function_ = det_func;
    trampoline_ = trampoline;
    code_size_ = code_size;
    trampoline_cache_ = trampoline_cache;

    return true;
}

void Detour::uninitialize()
{
    if (!is_initialized())
        return;

    DWORD old_protect = 0;
    BOOL api_result = FALSE;

    api_result = ::VirtualProtect(
        source_function_,
        5,
        PAGE_READWRITE,
        &old_protect);

    std::uninitialized_copy(
        &static_cast<const uint8_t*>(trampoline_)[0],
        &static_cast<const uint8_t*>(trampoline_)[5],
        static_cast<uint8_t*>(source_function_));

    api_result = ::VirtualProtect(
        source_function_,
        5,
        old_protect,
        &old_protect);

    auto current_process = ::GetCurrentProcess();

    api_result = ::FlushInstructionCache(
        current_process,
        source_function_,
        5);

    trampoline_cache_->deallocate(trampoline_);

    is_initialized_ = false;
    source_function_ = nullptr;
    detour_function_ = nullptr;
    trampoline_ = nullptr;
}

bool Detour::is_initialized() const
{
    return is_initialized_;
}

const void* Detour::get_trampoline() const
{
    return trampoline_;
}


} // namespace lgvs
