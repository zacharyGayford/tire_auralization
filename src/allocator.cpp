#include "allocator.h"

#include <atomic>
#include <stdlib.h>
#include <memory.h>

#include "defines.h"
#include "log.h"

namespace allocator {
	typedef struct Arena {
		u64 capacity;
		bool ownsMemory;
		std::atomic<u64> position;
		u8 allocation[];
	} Arena;

	// NOTE(zgayford): creates a "sub"-arena of a 
	// bigger arena that does not own its own 
	// memory; essentially a bump buffer
	Arena* arena_create(u64 capacity) { 
		u64 size = sizeof(Arena) + capacity;
		Arena* arena = (Arena*)malloc(size);
		// contractually zero initialized
		memset(arena, 0, size);
		if (!arena) return NULL;
		arena->ownsMemory = true;
		arena->capacity = capacity;
		arena->position = 0;
		return arena;
	}

	Arena* arena_create(Arena* arena, u64 capacity) {
		Arena* subarena = (Arena*)arena_allocate(arena, sizeof(Arena) + capacity);
		subarena->capacity = capacity;
		subarena->ownsMemory = false;
		subarena->position = 0;
		return subarena;
	}

	void arena_destroy(Arena* arena) {
		if (!arena->ownsMemory) {
			log_error("this arena cannot be destroyed because it does not own its memory");
			return;
		}
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

	u64 arena_bytes_written_get(Arena* arena) {
		return atomic_load(&arena->position);
	}

	void arena_flush(Arena* arena) {
		atomic_store(&arena->position, 0);
	}
}
