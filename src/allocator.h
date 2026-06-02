#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "defines.h"

namespace allocator {
	typedef struct Arena Arena;
	Arena* arena_create(u64 capacity);
	void arena_destroy(Arena* arena);

	void* arena_allocate(Arena* arena, u64 size);
	void arena_flush(Arena* arena);
}

#endif // ALLOCATOR_H
