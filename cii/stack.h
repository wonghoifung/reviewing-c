#ifndef STACK_HEADER
#define STACK_HEADER

#define T Stack_T

typedef struct T* T;

extern T Stack_new(void);
extern int Stack_empty(T stk);
extern void Stack_push(T stk, void* x);
extern void* Stack_pop(T stk);
extern void Stack_free(T* stk);

#undef T

#endif

