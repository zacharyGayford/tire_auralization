#include "thread.h"

#include <string.h>
#include <pthread.h>

#include "defines.h"
#include "allocator.h"
#include "log.h"

// TODO(zgayford): the use of pthreads universally is temporary.
// if time permits an api that uses winthreads on windows and pthreads on linux 
// is preferred.

namespace thread {
	typedef struct Thread {
		char name[THREAD_NAME_MAX_LENGTH];
		ThreadFunction fcn;
		pthread_t pthread;
		bool running;
		void* data;
	} Thread;

	thread_local Thread* currentThread;

	internal void* thread_function_wrapper(void* threadPtr) {
		currentThread = (Thread*)threadPtr;
		return currentThread->fcn(currentThread->data);
	}

	Thread* create(allocator::Arena* arena, const char* name, ThreadFunction fcn) {
		Thread* thread = (Thread*)allocator::arena_allocate(arena, sizeof(Thread));
		memcpy(thread->name, name, MIN(THREAD_NAME_MAX_LENGTH, strlen(name)));
		thread->fcn = fcn;
		thread->running = false;
		thread->data = NULL;
		return thread;
	}

	void run(Thread* thread, void* data) {
		thread->data = data;
		thread->running = true;
		pthread_create(&thread->pthread, NULL, thread_function_wrapper, thread);
	}

	void join(Thread* thread) {
		if (!thread->running) return;
		pthread_join(thread->pthread, NULL);
		thread->running = false;
	}

	char* name_get() {
		return currentThread->name;
	}
}
