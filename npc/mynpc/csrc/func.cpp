#include<stdio.h>
#include "verilated.h"

extern "C" void print(__uint32_t val)
{
    printf("val = %d %c\n", val,val);
    return;
}


extern "C" void End_Sim()
{
    Verilated::gotFinish(true);
    return;
}