#include "container.h"

#include <atomic>
#include <memory.h>

#include "defines.h"
#include "log.h"

namespace container {
	// NOTE(zgayford): if false sharing becomes a problem then
	// prob need to finagle alignment.
	typedef struct alignas(8) RingBuffer {
		u64 elementNum;
		u64 elementSize;
		std::atomic<u64> head;
		std::atomic<u64> tail;
		std::atomic<u64>* epoch;
		u8* data;
	} RingBuffer;

	RingBuffer* ring_buffer_create(allocator::Arena* arena, u64 elementNum, u64 elementSize) {
		u64 dataSize        = elementNum*elementSize;
		u64 epochNumberSize = elementNum*sizeof(std::atomic<u64>);
		u64 totalSize       = sizeof(RingBuffer) + dataSize + epochNumberSize;
		RingBuffer* buffer = (RingBuffer*)allocator::arena_allocate(arena, totalSize);
		if (!buffer) {
			log_fatal("failed to create ring buffer, out of memory");
			return NULL;
		}
		buffer->elementNum = elementNum;
		buffer->elementSize = elementSize;
		buffer->head = 0;
		buffer->tail = 0;
		buffer->epoch = (std::atomic<u64>*)(((u8*)buffer) + sizeof(RingBuffer));
		buffer->data  = (u8*)              (((u8*)buffer) + sizeof(RingBuffer) + epochNumberSize);

		for (u64 i = 0; i < buffer->elementNum; i++)
			buffer->epoch[i] = i;

		return buffer;
	}

	void ring_buffer_print(RingBuffer* buffer) {
		u64 head = atomic_load(&buffer->head);
		u64 tail = atomic_load(&buffer->tail);
		log_info("ring buffer:");
		log_info("\telementNum: %d", buffer->elementNum);
		log_info("\telementSize: %d", buffer->elementSize);
		log_info("\thead: %d", head);
		log_info("\ttail: %d", tail);
		log_info("\tepoch:");
		u64 epochNumber = 0;
		for (u64 i = 0; i < buffer->elementNum; i++) {
			epochNumber = atomic_load(&buffer->epoch[i]);
			log_info("\t\t[%d]: %d", i, epochNumber);
		}
	}

	bool ring_buffer_try_push(RingBuffer* buffer, void* element) {
		u64 tail, i, epoch;
		do {
			tail = atomic_load(&buffer->tail);
			i = tail % buffer->elementNum;
			epoch = atomic_load(&buffer->epoch[i]);

			// element is free
			if (epoch == tail) { 
				u64 oldTail = tail;
				u64 newTail = tail + 1;

				if (atomic_compare_exchange_weak(&buffer->tail, &oldTail, newTail)) {
					memcpy(&buffer->data[tail*buffer->elementSize], element, buffer->elementSize);
					atomic_store(&buffer->epoch[i], tail + 1);
					return true;
				} else
					continue;
			}

			// another producer has already written to this slot 
			// and it has not been consumed yet
			// i.e. buffer is full
			if (epoch < tail)
				return false;
		} while (epoch > tail);

		unreachable();
		return false;
	}

	bool ring_buffer_try_pop(RingBuffer* buffer, OUT void* element) {
		u64 head, i, epoch;
		do {
			head = atomic_load(&buffer->head);
			i = head % buffer->elementNum;
			epoch = atomic_load(&buffer->epoch[i]);

			// element is ready for consumption
			if (epoch == head + 1) {
				u64 oldHead = head;
				u64 newHead = head + 1;

				if (atomic_compare_exchange_weak(&buffer->head, &oldHead, newHead)) {
					memcpy(element, &buffer->data[oldHead*buffer->elementSize], buffer->elementSize);
					atomic_store(&buffer->epoch[i], oldHead + buffer->elementNum);
					return true;
				} else
					continue;
			}

			// this element has not been written to
			// i.e. buffer is empty
			if (epoch < head + 1)
				return false;
		} while (epoch > head + 1);

		unreachable();
		return false;
	}
}
