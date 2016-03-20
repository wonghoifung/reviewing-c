// #include "arith.h"
//#include "stack.h"
#include "atom.h"
#include <stdio.h>

int main() {

	const char* atom1 = Atom_string("hello");
	const char* atom2 = Atom_int(12345);
	printf("atom1:%s, atom2:%s\n", atom1, atom2);
	printf("%d, %d\n", Atom_length(atom1), Atom_length(atom2));

#if 0
	int x = -13;
	int y = 5;
	printf("%d\n", Arith_div(x,y));
	printf("%d\n", x/y);
	
	int N = 10;
	printf("%d\n", Arith_mod(10,N));
	printf("%d\n", Arith_mod(11,N));
	printf("%d\n", Arith_mod(0-1,N));

	printf("%d\n", (10%N));
	printf("%d\n", (11%N));
	printf("%d\n", ((0-1)%N));
#endif

#if 0
	Stack_T st = Stack_new();
	printf("empty:%d\n", Stack_empty(st));
	Stack_push(st, (void*)123);
	Stack_push(st, (void*)456);
	printf("empty:%d\n", Stack_empty(st));
	printf("%ld\n", (long)Stack_pop(st));
	printf("%ld\n", (long)Stack_pop(st));
	Stack_free(&st);
#endif


	return 0;
}
