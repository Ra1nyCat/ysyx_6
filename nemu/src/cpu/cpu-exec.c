/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>
#include "../monitor/sdb/sdb.h"
/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
// 当执行的指令数量少于此值时，只有执行的指令的汇编代码才会输出到屏幕。
#define MAX_INST_TO_PRINT 10
#define RING_BUF_SIZE 1024
#define FTRACE_BUF_SIZE 4096

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0; //执行的指令数量
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = true; //是否打印每一步的执行信息

//--------------------ring buffer--------------------
static char g_ring_buffer[RING_BUF_SIZE];
static int g_read_index = 0;
static int g_write_index = 0;

void write_ring_buffer(char ch){
  g_ring_buffer[g_write_index] = ch;
  g_write_index = (g_write_index + 1) % RING_BUF_SIZE;
}

char read_ring_buffer(){
  char ch = g_ring_buffer[g_read_index];
  g_read_index = (g_read_index + 1) % RING_BUF_SIZE;
  return ch;
}
//--------------------ring buffer--------------------


//--------------------FTRACE BUFFER------------------

// static char g_ftrace_buffer[FTRACE_BUF_SIZE];
// static int g_f_read_index = 0;
// static int g_f_write_index = 0;

#include<elf.h>
extern const char *strtab;
extern Elf32_Sym *symtab;
extern int symtab_size;

// void decode_inst(Decode *_this)
// {
//   //解析指令的地址

// }

size_t hex2val(char* str)
{
  int i=0;
  if(str[0]=='0'&&(str[1]=='x'||str[1]=='X'))i=2;
  size_t val=0;
  for(;str[i];i++){
    if(str[i]>='a'&&str[i]<='f'){
      val=val*16+(str[i]-'a')+10;
    }else{
      val=val*16+(str[i]-'0');
    }
  }
  return val;
}

Elf32_Addr hex2Elf32Addr(unsigned int str)
{
  // char* endPtr;
  // Elf32_Addr val=strtoul(str,&endPtr,16);
  // return val;
  return str;
}

int find_func(Elf32_Addr value)
{
  for(int i=0;i<symtab_size;i++){
    if(symtab[i].st_value<=value||symtab[i].st_value+symtab[i].st_size-1>=value){
      return i;
    }
  }
  return -1;
}



void device_update();

static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) { 
    log_write("%s\n", _this->logbuf);
    //写入ring buffer
    for(int i = 0; i < strlen(_this->logbuf); i++){
      write_ring_buffer(_this->logbuf[i]);
    }
    write_ring_buffer('\n'); 
  }
#endif

#ifdef CONFIG_FTRACE //函数追踪

  static int ftrace_dep=0;

  // char vale[35];
  // memset(vale,0,sizeof(char)*35);
  // for(int i=0;_this->logbuf[i]!=':';i++)
  //   vale[i]=_this->logbuf[i];

  
  Elf32_Addr value=hex2Elf32Addr(_this->pc);

  //识别函数调用指令和返回指令
  //调用指令 jal ra,xxxxx
  //返回指令 jalr zero,0(ra)

  //i定位到指令的第一个字符，五个空格
  int cnt=0;
  int i=0;
  for(;cnt<5;i++){
    if(_this->logbuf[i]==' ')cnt++;
  }

  //判断是否为jal指令
  const char* call="jal	ra, ";
  const char* ret="jalr	zero, 0(ra)";
  printf("%s\n",_this->logbuf+i);
  if(strncmp(_this->logbuf+i,call,strlen(call))==0){
    int idx=find_func(value);
    char* func_name=(char*)(strtab+symtab[idx].st_name);
    printf(FMT_WORD ":" ,_this->pc);
    for(int k=0;k<=ftrace_dep;k++)printf(" ");
    ftrace_dep++;
    printf("call [%s@%s]\n",func_name,_this->logbuf+i+strlen(call));
  }else if(strncmp(_this->logbuf+i,ret,strlen(ret))==0){
    //返回指令
    //从函数调用栈中弹出函数
    int idx=find_func(value);
    char* func_name=(char*)(strtab+symtab[idx].st_name);
    printf(FMT_WORD ":" ,_this->pc);
    for(int k=0;k<ftrace_dep;k++)printf(" ");
    printf("ret [%s]\n",func_name);
  }

#endif


  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));

#ifdef CONFIG_WATCHPOINT  //监测点
  if (watchpoint_check()) {
    if(nemu_state.state == NEMU_RUNNING)
      nemu_state.state = NEMU_STOP;
    // nemu_state.halt_pc = _this->pc;
    // nemu_state.halt_ret = 0;
  }
#endif

}

static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc;
  isa_exec_once(s);
  cpu.pc = s->dnpc;

  //指令追踪
#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst.val; //可以让程序以字节的方式访问 isa.inst.val 所在的内存区域，以进行字节级别的操作
  for (i = ilen - 1; i >= 0; i --) {
    p += snprintf(p, 4, " %02x", inst[i]);
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

#ifndef CONFIG_ISA_loongarch32r
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen);
#else
  p[0] = '\0'; // the upstream llvm does not support loongarch32r
#endif
#endif
}

static void execute(uint64_t n) {
  Decode s;
  for (;n > 0; n --) {
    exec_once(&s, cpu.pc);
    g_nr_guest_inst ++;
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING) break;
    IFDEF(CONFIG_DEVICE, device_update());
  }
}

static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
  isa_reg_display();
  statistic();
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;

    case NEMU_END: 
    case NEMU_ABORT:
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          nemu_state.halt_pc);
      if(nemu_state.halt_ret != 0) {
        //打印环形缓冲区中的内容
        printf("ring buffer:\n");
        while(g_read_index != g_write_index){
          printf("%c", read_ring_buffer());
        }
      }
      // fall through
    case NEMU_QUIT: statistic();
  }
}
