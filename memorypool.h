#pragma once
#include <mutex>
struct Block
{
	Block* next;
};

class MemoryPool
{
public:
	MemoryPool(size_t blockSize,size_t blockNum);
	~MemoryPool();
	void* alloc();
	void free(void* ptr);
private:
	void* start_;
	Block* free_list_;
	size_t block_size;
	size_t block_num;
	std::mutex mutex_;
};


