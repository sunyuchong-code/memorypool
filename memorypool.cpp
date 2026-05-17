#include "memorypool.h"
#include <thread>
MemoryPool::MemoryPool(size_t blockSize, size_t blockNum):block_size{blockSize},block_num{blockNum}
{
	size_t real_block_size = sizeof(Block) + blockSize;
	start_ = malloc(real_block_size * block_num);
	free_list_ = nullptr;
	char* p = (char*)start_;
	for (int i = 0; i < block_num; i++)
	{
		Block* block = (Block*)p;
		block->next = free_list_;
		free_list_ = block;
		p += real_block_size;
	}
}
MemoryPool::~MemoryPool()
{
	std::lock_guard<std::mutex> lock{ mutex_ };
	//注意调用全局free函数(::)
	::free(start_);
}
void* MemoryPool::alloc()
{
	std::lock_guard<std::mutex>lock{ mutex_ };
	if (free_list_ != nullptr)
	{
		Block* temp=free_list_;
		free_list_ = free_list_->next;
		return (char*)temp+sizeof(Block);
	}
	else
	{
		return nullptr;
	}
}
void MemoryPool::free(void* ptr)
{
	std::lock_guard<std::mutex>lock{ mutex_ };
	if (ptr != nullptr)
	{
		Block* temp = (Block*)((char*)ptr - sizeof(Block));
		temp->next = free_list_;
		free_list_ = temp;
	}
	else
	{
		return;
	}
}