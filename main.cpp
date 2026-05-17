#include "memorypool.h"
#include <iostream>
#include <vector>
#include <thread>
#include <cassert>
#include <cstring>

// 测试1：基本内存分配与释放功能
void testBasicAllocFree() {
    std::cout << "=== 测试1：基本分配释放 ===" << std::endl;
    // 创建内存池：每个块数据大小16字节，共10个块
    MemoryPool pool(16, 10);

    // 分配10个块（池的最大容量）
    std::vector<void*> blocks;
    for (int i = 0; i < 10; ++i) {
        void* p = pool.alloc();
        assert(p != nullptr); // 前10次分配必须成功
        blocks.push_back(p);
        // 写入测试数据
        memset(p, i, 16);
    }

    // 第11次分配应该失败（池已空）
    void* p11 = pool.alloc();
    assert(p11 == nullptr);
    std::cout << "基本分配释放测试通过\n" << std::endl;

    // 释放所有块，验证可再次分配
    for (void* p : blocks) {
        pool.free(p);
    }

    // 再次分配10个块，验证释放后可用
    for (int i = 0; i < 10; ++i) {
        void* p = pool.alloc();
        assert(p != nullptr);
        memset(p, i + 10, 16);
    }
}

// 测试2：并发分配/释放（验证线程安全）
void testConcurrentAllocFree() {
    std::cout << "=== 测试2：并发分配释放 ===" << std::endl;
    const int threadNum = 4;    // 测试线程数
    const int allocPerThread = 100; // 每个线程分配次数
    MemoryPool pool(32, threadNum * allocPerThread); // 足够容纳所有分配
    std::vector<std::thread> threads;

    // 线程函数：分配并释放内存
    auto worker = [&](int threadId) {
        std::vector<void*> blocks;
        // 分配
        for (int i = 0; i < allocPerThread; ++i) {
            void* p = pool.alloc();
            if (p != nullptr) {
                blocks.push_back(p);
                // 写入线程标识
                *(int*)p = threadId;
            }
        }
        // 释放
        for (void* p : blocks) {
            pool.free(p);
        }
        };

    // 启动线程
    for (int i = 0; i < threadNum; ++i) {
        threads.emplace_back(worker, i);
    }

    // 等待所有线程结束
    for (auto& t : threads) {
        t.join();
    }

    // 验证池可再次分配（无内存泄漏/死锁）
    void* p = pool.alloc();
    assert(p != nullptr);
    std::cout << "并发分配释放测试通过\n" << std::endl;
}

// 测试3：边界测试（块大小为0/极小、块数量为1）
void testEdgeCases() {
    std::cout << "=== 测试3：边界场景 ===" << std::endl;
    // 测试1：块数量为1
    {
        MemoryPool pool(64, 1);
        void* p1 = pool.alloc();
        assert(p1 != nullptr);
        void* p2 = pool.alloc();
        assert(p2 == nullptr); // 仅1个块，第二次分配失败
        pool.free(p1);
        void* p3 = pool.alloc();
        assert(p3 != nullptr); // 释放后可再次分配
    }

    // 测试2：块数据大小极小（1字节）
    {
        MemoryPool pool(1, 5);
        for (int i = 0; i < 5; ++i) {
            void* p = pool.alloc();
            assert(p != nullptr);
            *(char*)p = 'a' + i;
        }
        void* p6 = pool.alloc();
        assert(p6 == nullptr);
    }
    std::cout << "边界场景测试通过\n" << std::endl;
}

// 测试4：内存数据正确性（写入后读取验证）
void testDataIntegrity() {
    std::cout << "=== 测试4：内存数据完整性 ===" << std::endl;
    MemoryPool pool(8, 5); // 8字节数据块，5个

    // 分配并写入测试数据
    std::vector<void*> blocks;
    for (int i = 0; i < 5; ++i) {
        void* p = pool.alloc();
        assert(p != nullptr);
        *(uint64_t*)p = 0x123456789ABCDEF0ULL + i; // 写入64位数据
        blocks.push_back(p);
    }

    // 验证数据正确性
    for (int i = 0; i < 5; ++i) {
        uint64_t val = *(uint64_t*)blocks[i];
        assert(val == 0x123456789ABCDEF0ULL + i);
    }

    // 释放后重新分配，验证数据可覆盖
    pool.free(blocks[0]);
    void* p = pool.alloc();
    *(uint64_t*)p = 0xFFFFFFFFFFFFFFFFULL;
    assert(*(uint64_t*)p == 0xFFFFFFFFFFFFFFFFULL);
    std::cout << "内存数据完整性测试通过\n" << std::endl;
}

int main(void) {
    try {
        testBasicAllocFree();
        testConcurrentAllocFree();
        testEdgeCases();
        testDataIntegrity();
        std::cout << "所有测试全部通过！" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "测试失败：" << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "测试失败：未知异常" << std::endl;
        return 1;
    }
    return 0;
}