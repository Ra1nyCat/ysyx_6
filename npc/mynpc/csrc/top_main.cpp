#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include"VYPC.h"
#include"verilated.h"
#include"verilated_vcd_c.h"

int main(int argc,char** argv)
{
				VerilatedContext *contextp=new VerilatedContext;
				contextp->commandArgs(argc,argv);
				VYPC *top=new VYPC{contextp};
				
				VerilatedVcdC* tfp=new VerilatedVcdC;
				contextp->traceEverOn(true);
				top->trace(tfp,0);
				tfp->open("wave.vcd");

                for (int i=0;i<10;i++)
                {
                    
                    top->clk=1;
                    top->eval();
                    tfp->dump(i*10);
                    printf("This is :%d\n",top->ret);
                    printf("halt:%d\n",top->halt);
                    if(top->halt==1)
                        break;

                    top->clk=0;
                    top->eval();
                    tfp->dump(i*10+5);
                }

				delete top;
				tfp->close();
                delete tfp;
				delete contextp;
				return 0;
}

