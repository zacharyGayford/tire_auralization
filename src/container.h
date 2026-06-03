#ifndef CONTAINER_H
#define CONTAINER_H

#include <atomic>

#include "defines.h"
#include "allocator.h"

namespace container {
	typedef struct RingBuffer RingBuffer;
	RingBuffer* ring_buffer_create(allocator::Arena* arena, u64 elementNum, u64 elementSize);
	void ring_buffer_print(RingBuffer* buffer);
	bool ring_buffer_try_push(RingBuffer* buffer, void* element);
	bool ring_buffer_try_pop(RingBuffer* buffer, OUT void* element);
}

#endif // CONTAINER_H
