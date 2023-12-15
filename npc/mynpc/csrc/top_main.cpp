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

				while(!contextp->gotFinish()){
					
					top->eval();
                    if(top->halt==1)
                        break;
					tfp->dump(contextp->time());
					contextp->timeInc(1);
				}
				delete top;
				tfp->close();
				delete contextp;
				return 0;
}

