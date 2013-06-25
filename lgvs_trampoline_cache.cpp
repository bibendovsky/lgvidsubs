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


#include "lgvs_trampoline_cache.h"

#include <new>

#include <windows.h>

#include "lgvs_system.h"


namespace {


class Node {
public:
    Node* prev;
    Node* next;

    Node()
    {
        prev = this;
        next = this;
    }

    void link_back(Node* node)
    {
        node->prev = prev;
        node->next = this;

        prev->next = node;
        prev = node;
    }

    void unlink()
    {
        next->prev = prev;
        prev->next = next;

        prev = this;
        next = this;
    }

    bool is_linked()
    {
        return prev != this || next != this;
    }

    bool is_unlinked()
    {
        return prev == this && next == this;
    }
}; // class Node


class RootNodes {
public:
    Node free;
    Node used;

    static void initialize(
        void* data,
        size_t item_size,
        size_t item_count)
    {
        new (data) RootNodes();

        auto& free_root = static_cast<RootNodes*>(data)->free;

        auto items = reinterpret_cast<uint8_t*>(data) + sizeof(RootNodes);

        for (size_t i = 0; i < item_count; ++i) {
            free_root.link_back(reinterpret_cast<Node*>(items));
            items += sizeof(Node) + item_size;
        }
    }
}; // class RootNodes


} // namespace


namespace lgvs {


TrampolineCache::TrampolineCache() :
    is_initialized_(false),
    max_item_size_(0),
    max_item_count_(0),
    size_(0),
    cache_(nullptr)
{
}

TrampolineCache::~TrampolineCache()
{
    uninitialize();
}

bool TrampolineCache::initialize(
    size_t max_item_size,
    size_t max_item_count)
{
    uninitialize();

    if (max_item_size == 0)
        return false;

    if (max_item_count == 0)
        return false;

    auto page_size = System::get_page_size();

    max_item_size = ((max_item_size + (MIN_ITEM_SIZE - 1)) / MIN_ITEM_SIZE) * MIN_ITEM_SIZE;

    auto size = (max_item_count * (max_item_size + sizeof(Node))) +
        sizeof(RootNodes);

    size = ((size + (page_size - 1)) / page_size) * page_size;

    auto cache = ::VirtualAlloc(
        nullptr,
        size,
        MEM_COMMIT,
        PAGE_EXECUTE_READWRITE);

    if (cache == nullptr)
        return false;

    RootNodes::initialize(cache, max_item_size, max_item_count);

    is_initialized_ = true;
    max_item_size_ = max_item_size;
    max_item_count_ = max_item_count;
    size_ = size;
    cache_ = cache;

    return true;
}

void TrampolineCache::uninitialize()
{
    if (cache_ != nullptr) {
        ::VirtualFree(
            cache_,
            size_,
            MEM_DECOMMIT);

        cache_ = nullptr;
    }

    is_initialized_ = false;
    max_item_size_ = 0;
    max_item_count_ = 0;
    size_ = 0;
}

bool TrampolineCache::is_initialized() const
{
    return is_initialized_;
}

void* TrampolineCache::allocate()
{
    if (!is_initialized())
        return nullptr;

    auto& root_nodes = *reinterpret_cast<RootNodes*>(cache_);

    if (root_nodes.free.is_unlinked())
        return nullptr;

    auto node = root_nodes.free.next;

    node->unlink();

    root_nodes.used.link_back(node);

    return &node[1];
}

void TrampolineCache::deallocate(void* address)
{
    if (!is_initialized())
        return;

    if (address == nullptr)
        return;

    auto node = &static_cast<Node*>(address)[-1];

    node->unlink();

    auto& free_root = reinterpret_cast<RootNodes*>(cache_)->free;

    free_root.link_back(node);
}

size_t TrampolineCache::get_max_item_size() const
{
    return max_item_size_;
}


} // namespace lgvs
