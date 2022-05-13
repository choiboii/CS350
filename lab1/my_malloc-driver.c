//
//  my_malloc-driver.c
//  Lab1: Malloc
//

#include "my_malloc.h"
#include <stdio.h>

int main(int argc, const char * argv[])
{
    int *p;
    char *c;
    double *d;

    p = my_malloc(sizeof(uint32_t));  // allocate heap memory for storing an int
    c = my_malloc(64 * sizeof(char));
    d = my_malloc(sizeof(double));

    c = "The wise heart seeks knowledge.";
    *d = 15.233;
    *p = 6;   // the heap memory p points to gets the value 6
    printf("pointer address: %p\n", p);
    printf("pointer value: %d", *p);
    printf("pointer address: %p\n", c);
    printf("pointer value: %s", c);
    printf("pointer address: %p\n", d);
    printf("pointer value: %f", *d);

    my_free(p);
    my_free(c);
    my_free(d);
    
    return 0;
}
