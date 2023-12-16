#include<stdio.h>


extern "C" void print(__uint32_t val)
{
    printf("val = %d\n", val);
    return;
}