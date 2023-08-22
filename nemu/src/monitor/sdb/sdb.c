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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/paddr.h>

//是否处于批处理模式
static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */

static char* rl_gets() {
  //上一次读取的内容行
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  // 使用 readline 函数读取用户输入，提示符为 "(nemu) "
  line_read = readline("(nemu) ");
  // 如果用户输入了内容，将其添加到历史记录中
  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}


static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

// static int cmd_w(char *args);

// static int cmd_d(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si","Exec step by step",cmd_si},
  {"info","Print the information of registers or watchpoints",cmd_info},
  {"x","Scan the memory",cmd_x},
  {"p","Calculate the value of the expression",cmd_p},
  // {"w","Set a watchpoint",cmd_w},
  // {"d","Delete a watchpoint",cmd_d},
  // {"bt","Print the stack frame chain",cmd_bt},
  // {"cache","Print the cache information",cmd_cache},
  // {"tlb","Print the tlb information",cmd_tlb},
  // {"page","Print the page information",cmd_page},
  // {"set","Set the value of the register",cmd_set},
  /* TODO: Add more commands */
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args){
  char *arg = strtok(NULL," ");
  int n = 1;
  if(arg != NULL){
    sscanf(arg,"%d",&n);
  }
  cpu_exec(n);
  return 0;
}

static int cmd_info(char *args){
  char *arg = strtok(NULL," ");
  if(arg == NULL){
    printf("Please input the argument!\n");
    return 0;
  }
  if(strcmp(arg,"r") == 0){
    isa_reg_display();
  }
  else if(strcmp(arg,"w") == 0){
    //print_wp();
  }
  else{
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_x(char *args)
{
  char *arg = strtok(NULL," ");
  if(arg == NULL){
    printf("Please input the argument!\n");
    return 0;
  }
  int n;
  paddr_t addr;
  sscanf(arg,"%d",&n);
  arg = strtok(NULL," ");
  if(arg == NULL){
    printf("Please input the argument!\n");
    return 0;
  }
  sscanf(arg,"%x",&addr);
  for(int i = 0;i < n;i++){
    if(likely(addr + i * 4))
      printf("0x%08x: 0x%08x\n",addr + i * 4,paddr_read(addr + i * 4,4));
    else{
      printf("0x%08x out of memory in [0x%08x,0x%08x]\n",addr+ i * 4,PMEM_LEFT,PMEM_RIGHT);
      return -1;
    }
  }
  return 0;
}

static int cmd_p(char *args)
{
  char *arg = strtok(NULL," ");
  if(arg == NULL){
    printf("Please input the argument!\n");
    return 0;
  }
  bool success = true;
  word_t result = expr(arg,&success);
  if(success){
    printf("%d\n",result);
  }
  else{
    printf("Invalid expression!\n");
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    // 将剩余的字符串视为命令的参数
    char *args = cmd + strlen(cmd) + 1;
    printf("args: %s\n", args);
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    // 寻找匹配命令，并放入参数
    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
