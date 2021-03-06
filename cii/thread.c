#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include </usr/include/signal.h>
#include <sys/time.h>
#include "assert.h"
#include "mem.h"
#include "thread.h"
#include "sem.h"

void _MONITOR(void) {}
extern void _ENDMONITOR(void);

#define T Thread_T

// macros
#define isempty(q) ((q) == NULL)

// types
struct T {
	unsigned long* sp; // must be first
	// fields
	T link;
	T* inqueue;
	T handle;
	Except_Frame* estack;
	int code;
	T join;
	T next;
	int alerted;
};

// data
static T ready = NULL;
static T current;
static int nthreads;
static struct Thread_T root;
static T join0;
static T freelist;
const Except_T Thread_Alerted = { "thread alerted" };
const Except_T Thread_Failed = { "thread creation failed" };
static int critical;

// prototypes
extern void _swtch(T from, T to);

// static functions
static void put(T t, T* q) {
	assert(t);
	assert(t->inqueue == NULL && t->link == NULL);
	if (*q) {
		t->link = (*q)->link;
		(*q)->link = t;
	} else {
		t->link = t;
	}
	*q = t;
	t->inqueue = q;
}

static T get(T* q) {
	T t;
	assert(!isempty(*q));
	t = (*q)->link;
	if (t == *q)
		*q = NULL;
	else 
		(*q)->link = t->link;
	assert(t->inqueue == q);
	t->link = NULL;
	t->inqueue = NULL;
	return t;
}

static void delete(T t, T* q) {
	T p;
	assert(t->link && t->inqueue == q);
	assert(!isempty(*q));
	for (p = *q; p->link != t; p = p->link)
		;
	if (p == t)
		*q = NULL;
	else {
		p->link = t->link;
		if (*q == t)
			*q = p;
	}
	t->link = NULL;
	t->inqueue = NULL;
}

static void run(void) {
	T t = current;
	current = get(&ready);
	t->estack = Except_stack;
	Except_stack = current->estack;
	// printf("switch from %p to %p...\n", t, current);
	_swtch(t, current);
	// printf("switch done, current: %p...\n", current);
}

static void testalert(void) {
	if (current->alerted) {
		current->alerted = 0;
		RAISE(Thread_Alerted);
	}
}

static void release(void) {
	T t;

	// begin critical region
	do { critical++;

	while ((t = freelist) != NULL) {
		freelist = t->next;
		FREE(t);
	}

	// end critical region
	critical--; } while (0);
}

#if linux && __x86_64__
// #include <execinfo.h>
static int interrupt(int sig, struct sigcontext scp) {
	printf("interrupt rip: %p, current: %p, start:%p, end:%p\n", scp.rip, current, _MONITOR, _ENDMONITOR);
	if (scp.rip == 0) {
		// TODO why this happens?
		/*
		   man sigaction... and then I get
		   
		   Undocumented
		       Before  the  introduction of SA_SIGINFO it was also possible to get some additional information, namely by using a sa_handler with second argument of type struct sigcontext.
		       See the relevant kernel sources for details.  This use is obsolete now.
		*/
		printf("current thread:%p, rip is 0\n", current);
		return 0;
	}
	if (critical || scp.rip >= (unsigned long)_MONITOR && scp.rip <= (unsigned long)_ENDMONITOR)
		return 0;
	put(current, &ready);
	sigsetmask(scp.oldmask);
	run();
	return 0;
}
#elif linux && i386
#include <asm/sigcontext.h>
static int interrupt(int sig, struct sigcontext_struct sc) {
	printf("interrupt eip: %p, current: %p, start:%p, end:%p\n", sc.eip, current, _MONITOR, _ENDMONITOR);
	if (critical || sc.eip >= (unsigned long)_MONITOR && sc.eip <= (unsigned long)_ENDMONITOR)
		return 0;
	put(current, &ready);
	do { critical++;
	sigsetmask(sc.oldmask);
	critical--; } while (0);
	run();
	return 0;
}
#else
static int interrupt(int sig, int code, struct sigcontext* scp) {
	if (critical || scp->sc_pc >= (unsigned long)_MONITOR && scp->sc_pc <= (unsigned long)_ENDMONITOR)
		return 0;
	put(current, &ready);
	sigsetmask(scp->sc_mask);
	run();
	return 0;
}
#endif

static int is_in_correct_region(void* f) {
	return f > _MONITOR && f < _ENDMONITOR;
}
static void interface_addresses() {
	printf("Thread_init: %d\n", is_in_correct_region(Thread_init));
	printf("Thread_new: %d\n", is_in_correct_region(Thread_new));
	printf("Thread_exit: %d\n", is_in_correct_region(Thread_exit));
	printf("Thread_alert: %d\n", is_in_correct_region(Thread_alert));
	printf("Thread_self: %d\n", is_in_correct_region(Thread_self));
	printf("Thread_join: %d\n", is_in_correct_region(Thread_join));
	printf("Thread_pause: %d\n", is_in_correct_region(Thread_pause));
	printf("Sem_init: %d\n", is_in_correct_region(Sem_init));
	printf("Sem_new: %d\n", is_in_correct_region(Sem_new));
	printf("Sem_wait: %d\n", is_in_correct_region(Sem_wait));
	printf("Sem_signal: %d\n", is_in_correct_region(Sem_signal));
}

// thread functions
int Thread_init(int preempt, ...) {
	// interface_addresses();
	assert(preempt == 0 || preempt == 1);
	assert(current == NULL);
	root.handle = &root;
	current = &root;
	nthreads = 1;
	if (preempt) {
		// initialize preemptive scheduling
		{
			struct sigaction sa;
			memset(&sa, '\0', sizeof sa);
			sa.sa_handler = (void (*)())interrupt;
			if (sigaction(SIGVTALRM, &sa, NULL) < 0)
				return 0;
		}
		{
			struct itimerval it;
			it.it_value.tv_sec     =  0;
			it.it_value.tv_usec    = 50;
			it.it_interval.tv_sec  =  0;
			it.it_interval.tv_usec = 50;
			if (setitimer(ITIMER_VIRTUAL, &it, NULL) < 0)
				return 0;
		}
	}
	return 1;
}

T Thread_self(void) {
	assert(current);
	return current;
}

void Thread_pause(void) {
	assert(current);
	put(current, &ready);
	run();
}

int Thread_join(T t) {
	assert(current && t != current);
	testalert();
	if (t) {
		// wait for thread t to terminate
		if (t->handle == t) {
			put(current, &t->join);
			run();
			testalert();
			return current->code;
		} else {
			return -1;
		}
	} else {
		// wait for all threads to terminate
		assert(isempty(join0));
		if (nthreads > 1) {
			put(current, &join0);
			run();
			testalert();
		}

		return 0;
	}
}

void Thread_exit(int code) {
	assert(current);
	printf("Thread_exit: %p, nthreads: %d\n", current, nthreads);
	release();
	if (current != &root) {
		current->next = freelist;
		freelist = current;
	}
	current->handle = NULL;

	// resume threads waiting for current's termination
	while (!isempty(current->join)) {
		T t = get(&current->join);
		t->code = code;
		put(t, &ready);
	}
	if (!isempty(join0) && nthreads == 2) {
		assert(isempty(ready));
		put(get(&join0), &ready);
	}

	// run another thread or exit
	if (--nthreads == 0)
		exit(code);
	else
		run();
}

void Thread_alert(T t) {
	assert(current);
	assert(t && t->handle == t);
	t->alerted = 1;
	if (t->inqueue) {
		delete(t, t->inqueue);
		put(t, &ready);
	}
}

#if __x86_64__
#define K 32
#define K_1 31
#define K_1U 31U
#else
#define K 16
#define K_1 15
#define K_1U 15U
#endif
T Thread_new(int apply(void*), void* args, int nbytes, ...) {
	T t;
	assert(current);
	assert(apply);
	assert(args && nbytes >= 0 || args == NULL);
	if (args == NULL) nbytes = 0;
	
	// allocate resources for a new thread
	{
		int stacksize = (K * 1024 + sizeof (*t) + nbytes + K_1) & ~K_1;
		release();

		// begin critical region
		do { critical++;

		TRY
			t = ALLOC(stacksize);
			memset(t, '\0', sizeof *t);
		EXCEPT(Mem_Failed)
			t = NULL;
		END_TRY;

		// end critical region
		critical--; } while (0);

		if (t == NULL)
			RAISE(Thread_Failed);

		// initialize's stack pointer
		t->sp = (void*)((char*)t + stacksize);
		while (((unsigned long)t->sp) & K_1)
			t->sp--;
	}

	t->handle = t;
	
	// initialize t's state
	if (nbytes > 0) {
		t->sp -= ((nbytes + K_1U) & ~K_1) / sizeof (*t->sp);

		// begin critical region
		do { critical++;

		memcpy(t->sp, args, nbytes);

		// end critical region
		critical--; } while (0);

		args = t->sp;
	}
	#if linux && i386
	{
	  extern void _thrstart(void);
	  // t->sp -= 4/4;
	  t->sp -= 16/4;	/* keep stack aligned to 16-byte boundaries */
	  *t->sp = (unsigned long)_thrstart;
	  t->sp -= 16/4;
	  t->sp[4/4]  = (unsigned long)apply;
	  t->sp[8/4]  = (unsigned long)args;
	  t->sp[12/4] = (unsigned long)t->sp + (4+16)/4; 
	}
	#elif linux && __x86_64__
	{
	  // printf("Thread_new: linux && __x86_64__\n");
	  extern void _thrstart(void);
	  // t->sp -= 8/8;
	  t->sp -= 32/8;	// TODO
	  *t->sp = (unsigned long)_thrstart;
	  // t->sp -= 32/8;
	  // t->sp[8/8]  = (unsigned long)apply;
	  // t->sp[16/8]  = (unsigned long)args;
	  // t->sp[24/8] = (unsigned long)t->sp + (8+32)/8; 
	  t->sp -= 96/8;
	  t->sp[8/8]  = (unsigned long)apply;
	  t->sp[16/8]  = (unsigned long)args;
	  t->sp[24/8] = (unsigned long)t->sp + (8+96)/8; 
	}
	#else
	unsupported platform
	#endif

	nthreads++;
	// printf("new thread: %p\n", t);
	put(t, &ready);
	return t;
}

#undef T

#define T Sem_T

// sem functions
T* Sem_new(int count) {
	T* s;
	NEW(s);
	Sem_init(s, count);
	return s;
}

void Sem_init(T* s, int count) {
	assert(current);
	assert(s);
	s->count = count;
	s->queue = NULL;
}

void Sem_wait(T* s) {
	assert(current);
	assert(s);
	testalert();
	if (s->count <= 0) {
		put(current, (Thread_T*)&s->queue);
		run();
		testalert();
	} else {
		--s->count;
	}
}

void Sem_signal(T* s) {
	assert(current);
	assert(s);
	if (s->count == 0 && !isempty(s->queue)) {
		Thread_T t = get((Thread_T*)&s->queue);
		assert(!t->alerted);
		put(t, &ready);
	} else {
		++s->count;
	}
}

#undef T
