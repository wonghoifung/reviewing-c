#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "assert.h"
#include "seq.h"
#include "array.h"
#include "arrayrep.h"
#include "mem.h"

#define T Seq_T

struct T {
	struct Array_T array;
	int length;
	int head;
};

// static functions
static void expand(T seq) {
	int n = seq->array.length;
	Array_resize(&seq->array, 2 * n);
	if (seq->head > 0)
	// slide tail down
	{
		void** old = &((void**)seq->array.array)[seq->head];
		memcpy(old + n, old, (n - seq->head) * sizeof (void*));
		seq->head += n;
	}
}

// functions
T Seq_new(int hint) {
	T seq;
	assert(hint >= 0);
	NEW0(seq);
	if (hint == 0)
		hint = 16;
	ArrayRep_init(&seq->array, hint, sizeof (void*), ALLOC(hint * sizeof (void*)));
	return seq;
}

T Seq_seq(void* x, ...) {
	va_list ap;
	T seq = Seq_new(0);
	va_start(ap, x);
	for (; x; x=va_arg(ap, void*))
		Seq_addhi(seq, x);
	va_end(ap);
	return seq;
}

void Seq_free(T* seq) {
	assert(seq && *seq);
	assert((void*)*seq == (void*)&(*seq)->array);
	Array_free((Array_T*)seq);
}

int Seq_length(T seq) {
	assert(seq);
	return seq->length;
}

void* Seq_get(T seq, int i) {
	assert(seq);
	assert(i >= 0 && i < seq->length);
	// seq[i]
	return ((void**)seq->array.array)[(seq->head + i) % seq->array.length];
}

void* Seq_put(T seq, int i, void* x) {
	void* prev;
	assert(seq);
	assert(i >= 0 && i < seq->length);
	prev = ((void**)seq->array.array)[(seq->head + i) % seq->array.length];
	((void**)seq->array.array)[(seq->head + i) % seq->array.length] = x;
	return prev;
}

void* Seq_remhi(T seq) {
	int i;
	assert(seq);
	assert(seq->length > 0);
	i = --seq->length;
	return ((void**)seq->array.array)[(seq->head + i) % seq->array.length];
}

void* Seq_remlo(T seq) {
	int i = 0;
	void* x;
	assert(seq);
	assert(seq->length > 0);
	x = ((void**)seq->array.array)[(seq->head + i) % seq->array.length];
	seq->head = (seq->head + 1) % seq->array.length;
	--seq->length;
	return x;
}

void* Seq_addhi(T seq, void* x) {
	int i;
	assert(seq);
	if (seq->length == seq->array.length)
		expand(seq);
	i = seq->length++;
	return ((void**)seq->array.array)[(seq->head + i) % seq->array.length] = x;
}

void* Seq_addlo(T seq, void* x) {
	int i = 0;
	assert(seq);
	if (seq->length == seq->array.length)
		expand(seq);
	if (--seq->head < 0)
		seq->head = seq->array.length - 1;
	seq->length++;
	return ((void**)seq->array.array)[(seq->head + i) % seq->array.length] = x;
}

#include <stdio.h>

void test_seq() {
	int i;
	Seq_T seq;
	
	seq = Seq_seq("a", "b", "c", "d", "hello", "world", NULL);
	
	for (i=0; i<Seq_length(seq); i++) {
		printf("%s\n", (char*)Seq_get(seq, i));
	}

	void* r = Seq_remlo(seq);

	for (i=0; i<Seq_length(seq); i++) {
		printf("%s\n", (char*)Seq_get(seq, i));
	}

	printf("r: %s\n", (char*)r);

	Seq_free(&seq);
}


