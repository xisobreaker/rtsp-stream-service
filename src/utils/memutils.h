#ifndef MEMUTILS_H_H_H
#define MEMUTILS_H_H_H

#include <iostream>
#include <memory>
#include <stddef.h>

/**
 * 申请内存, 使用 malloc 分配一块内存
 * @param size 内存大小
 * @return 分配的内存指针
 */
void *mem_malloc(size_t size);

/**
 * 释放内存，释放通过 mem_malloc 申请的内存
 * @param ptr mem_malloc 创建的内存
 */
void mem_free(void *ptr);

/**********************************************************
 * 自定义删除器。
 **********************************************************/
struct MemDeleter {
    void operator()(void *ptr) const
    {
        mem_free(ptr);
    }
};

template <typename T>
std::shared_ptr<T> make_shared_ptr(size_t size)
{
    return std::shared_ptr<T>((T *)mem_malloc(size), MemDeleter());
}

#endif
