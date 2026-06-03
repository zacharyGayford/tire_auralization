#include "allocator.h"

#include <atomic>
#include <stdlib.h>
#include <memory.h>

#include "defines.h"

namespace allocator {
	typedef struct Arena {
		u64 capacity;
		std::atomic<u64> position;
		u8 allocation[];
	} Arena;

	Arena* arena_create(u64 capacity) { 
		u64 size = sizeof(Arena) + capacity;
		Arena* arena = (Arena*)malloc(size);
		// contractually zero initialized
		memset(arena, 0, size);
		if (!arena) return NULL;
		arena->capacity = capacity;
		arena->position = 0;
		return arena;
	}

	void arena_destroy(Arena* arena) {
		free(arena);
	}

	void* arena_allocate(Arena* arena, u64 size) {
		while(1) {
			u64 oldPosition = atomic_load(&arena->position);
			u64 proposedPosition = oldPosition + size;

			if (proposedPosition > arena->capacity) return NULL;

			if (atomic_compare_exchange_weak(&arena->position, &oldPosition, proposedPosition))
				return &arena->allocation[oldPosition];
		}
	}

	void arena_flush(Arena* arena) {
		atomic_store(&arena->position, 0);
	}
}
