#ifndef agile_sort_header
#define agile_sort_header

int issort(void* data, int size, int esize, int(*compare)(const void*,const void*));

void test_agile_sort();

#endif
