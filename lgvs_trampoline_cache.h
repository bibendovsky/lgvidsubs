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


#include <stdint.h>


namespace lgvs {


class TrampolineCache {
public:
    static const size_t MIN_ITEM_SIZE = 16;


    TrampolineCache();
    ~TrampolineCache();

    // Note: Size of item will be aligned by 16 bytes.
    bool initialize(
        size_t max_item_size,
        size_t max_item_count);

    void uninitialize();

    bool is_initialized() const;

    void* allocate();
    void deallocate(void* address);

    size_t get_max_item_size() const;


private:
    bool is_initialized_;
    size_t max_item_size_;
    size_t max_item_count_;
    size_t size_;
    void* cache_;

    TrampolineCache(const TrampolineCache& that);
    TrampolineCache& operator=(const TrampolineCache& that);
}; // class TrampolineCache


} // namespace lgvs
