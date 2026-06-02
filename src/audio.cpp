#include "audio.h"

#include "defines.h"
#include "log.h"

namespace audio {
	void* main(void* data) {
		log_debug("Hello from the audio thread!");
		return NULL;
	}
}
