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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>


// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 1024] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"#include <signal.h>\n"
"#include <stdlib.h>\n"
"void handle_divede_by_zero(int sign){"
"  exit(1);"
"} "
"int main() { "
"  signal(SIGFPE, handle_divede_by_zero);"
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

uint32_t choose(uint32_t n)
{
  return rand()%n;
}

void gen(char c)
{
  buf[strlen(buf)] = c;
}

void gen_num()
{
  uint32_t num = choose(100);
  sprintf(buf+strlen(buf),"%u",num);
  buf[strlen(buf)] = 'U';
}

void gen_rand_op() {
  switch(choose(7))
  {
    case 0:gen('+');break;
    case 1:gen('-');break;
    case 2:gen('*');break;
    case 3:gen('=');gen('=');break;
    case 4:gen('&');gen('&');break;
    case 5:gen('|');gen('|');break;
    case 6:gen('!');gen('=');break;
    default:gen('/');break;
  }
}

void randspace()
{
  if(choose(100)%2)gen(' ');
}

static void gen_rand_expr(int dep,int *status) {
  if(dep>50)
  {
    *status = 0;
    return;
  }
  randspace();
  switch(choose(3))
  {
    case 0:gen_num();break;
    case 1:gen('(');gen_rand_expr(dep+1,status);gen(')');break;
    default:gen_rand_expr(dep+1,status);gen_rand_op();gen_rand_expr(dep+1,status);break;
  }
  randspace();
}

void clear()
{
  memset(buf,0,sizeof(buf));
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    clear();
    int status = 1;
    gen_rand_expr(0,&status);

    if(status==0||strlen(buf)>sizeof(buf)-1){ //避免表达式溢出
      i--;
      continue;
    }

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc -Werror /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    uint32_t result;
    ret = fscanf(fp, "%u", &result);

    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
