#ifndef buffer_header
#define buffer_header

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct BufferContext {
		void* pBuf;
		size_t size;
		bool (*processor)(struct BufferContext* p);
	} BufferContext;

	bool buffer(BufferContext* pThis);
	void* allocate_buffer(BufferContext* pThis, size_t size);

#ifdef __cplusplus
}
#endif

#endif
