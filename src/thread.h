#ifndef THREAD_H
#define THREAD_H

#include "defines.h"
#include "allocator.h"

namespace thread {
	typedef struct Thread Thread;
	typedef void* (*ThreadFunction)(void*);

	#define THREAD_NAME_MAX_LENGTH 128
	Thread* create(allocator::Arena* arena, const char* name, ThreadFunction fcn);
	void destroy(Thread* thread);
	void run(Thread* thread);
	void join(Thread* thread);
	char* name_get();

	// TODO(zgayford): going to need this later most likely
	/*
	typedef struct Mutex Mutex;
	Mutex* mutex_create(allocator::Arena* arena);
	*/
}

#endif // THREAD_H
