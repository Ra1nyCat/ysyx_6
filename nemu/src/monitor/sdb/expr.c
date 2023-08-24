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
#include <memory/vaddr.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,TK_NUMBER ,TK_XNUMBER,TK_REGISTER,TK_NONEQUAL,TK_AND,TK_OR,
  TK_DEREF, //解引用
  TK_NEGATIVE //负号
  /* TODO: Add more token types */
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"-", '-'},           // minus
  {"\\*", '*'},         // multiply
  {"\\/", '/'},         // divide
  {"\\(", '('},         // left bracket
  {"\\)", ')'},         // right bracket
  {"0[xX][0-9a-fA-F]+",TK_XNUMBER},// 16-number
  {"[0-9]+", TK_NUMBER},// 10-number
  {"\\$[a-zA-Z]+[0-9]*",TK_REGISTER},// register
  {"!=",TK_NONEQUAL},// not equal
  {"&&",TK_AND},// and
  {"\\|\\|",TK_OR},// or
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[65536] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static void pre_process()
{
  int i=0;
  for(i=0;i<nr_token;i++)
  {
    //处理负号
    if(tokens[i].type=='-'&&
    (i==0||(tokens[i-1].type!=TK_NUMBER&&tokens[i-1].type!=TK_XNUMBER&&tokens[i-1].type!=TK_REGISTER&&tokens[i-1].type!=')')))
    {
      tokens[i].type=TK_NEGATIVE;
    }

    //处理解引用
    if(tokens[i].type=='*'&&
    (i==0||(tokens[i-1].type!=TK_NUMBER&&tokens[i-1].type!=TK_XNUMBER&&tokens[i-1].type!=TK_REGISTER&&tokens[i-1].type!=')')))
    {
      tokens[i].type=TK_DEREF;
    }
  }
}

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //     i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE: break;
          case TK_NUMBER: 
            tokens[nr_token].type = rules[i].token_type;
            memset(tokens[nr_token].str,0,sizeof(tokens[nr_token].str));
            strncpy(tokens[nr_token].str,substr_start,substr_len);
            nr_token++;
            break;
          case TK_XNUMBER:
            tokens[nr_token].type = rules[i].token_type;
            memset(tokens[nr_token].str,0,sizeof(tokens[nr_token].str));
            strncpy(tokens[nr_token].str,substr_start,substr_len);
            nr_token++;
            break;
          case TK_REGISTER:
            tokens[nr_token].type = rules[i].token_type;
            memset(tokens[nr_token].str,0,sizeof(tokens[nr_token].str));
            strncpy(tokens[nr_token].str,substr_start+1,substr_len);
            nr_token++;
            break;
          case TK_AND:
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          case TK_OR:
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          case TK_NONEQUAL:
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          case TK_EQ:
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          case '+':
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          case '-':
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          case '*':
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          case '/':
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          case '(':
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          case ')':
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          default: TODO();
        }
        break;
      }
    }
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

int check_parenthesis(int l,int r)
{
  /*
    1: 最外层括号冗余，可以去除
    0: 括号合法，但是最外层不冗余
    -1:括号不合法
  */
  //首先检查括号是否合法
  int count = 0;
  for(int i=l;i<=r;i++)
  {
    if(tokens[i].type == '(')
      count++;
    if(tokens[i].type == ')')
      count--;
    if(count<0)
      return -1;
  }
  if(count<0)return -1;

  //检查最外层括号是否冗余
  if(tokens[l].type == '(' && tokens[r].type == ')')
  {
    count = 0;
    for(int i=l;i<=r;i++)
    {
      if(tokens[i].type == '(')
        count++;
      if(tokens[i].type == ')')
        count--;
      if(count == 0 && i != r)
        return 0; //不是冗余
    }
    return 1; //是冗余
  }
  //括号合法但不冗余
  return 0;
}

int dominant_operator(int l,int r)
{
  /*
      优先级：
      1. -负号   6 
      2. *      5
      2. * /    4
      3. + -    3
      4. == !=  2
      5. &&     1
      6. ||     0
  */
  int i;
  int count = 0;
  int op = -1;
  int priority = 6;
  for(i=l;i<=r;i++){
    if(tokens[i].type == '('){
      count++;
    }else if(tokens[i].type == ')'){
      count--;
    }
    if(count == 0){
      if(tokens[i].type == TK_NEGATIVE){
        if(priority >= 6){
          priority = 6;
          op = i;
        }
      }else if(tokens[i].type==TK_DEREF){
        if(priority >= 5){
          priority = 5;
          op = i;
        }
      }else if(tokens[i].type == '*'||tokens[i].type == '/'){
        if(priority >= 4){
          priority = 4;
          op = i;
        }
      }else if(tokens[i].type == '+'||tokens[i].type == '-'){
        if(priority >= 3){
          priority = 3;
          op = i;
        }
      }else if(tokens[i].type == TK_EQ||tokens[i].type == TK_NONEQUAL){
        if(priority >= 2){
          priority = 2;
          op = i;
        }
      }else if(tokens[i].type == TK_AND){
        if(priority >= 1){
          priority = 1;
          op = i;
        }
      }else if(tokens[i].type == TK_OR){
        if(priority >= 0){
          priority = 0;
          op = i;
        }
      }
    }
  }
  return op;
}

//将字符串转化为无符号整数
uint32_t str2val(char *str)
{
  int len = strlen(str);
  uint32_t val = 0;
  for(int i=0;i<len;i++){
    val = val*10 + str[i]-'0';
  }
  return val;
}

//将16进制字符串转化为无符号整数
uint32_t str2hval(char *str)
{
  int len = strlen(str);
  uint32_t val = 0;
  for(int i=2;i<len;i++){
    if(str[i]>='0'&&str[i]<='9'){
      val = val*16 + str[i]-'0';
    }else if(str[i]>='a'&&str[i]<='f'){
      val = val*16 + str[i]-'a'+10;
    }else if(str[i]>='A'&&str[i]<='F'){
      val = val*16 + str[i]-'A'+10;
    }
  }
  return val;
}


word_t eval(int l,int r,bool *success)
{
  if(l>r){
    return 0;
  }else if(l==r){
    switch (tokens[l].type)
    {
    case TK_NUMBER:
      return str2val(tokens[l].str);
      break;
    case TK_XNUMBER:
      return str2hval(tokens[l].str);
      break;
    case TK_REGISTER:
      return isa_reg_str2val(tokens[l].str,success);
      break;
    default:
      printf("Invalid expression!\n");
      return -1;
      break;
    }
    return str2val(tokens[l].str);
  }else if(check_parenthesis(l,r) == 1){ //摘掉一个括号
    return eval(l+1,r-1,success);
  }else if(check_parenthesis(l,r)==0){
    int op = dominant_operator(l,r);
    uint32_t val1 = eval(l,op-1,success);
    uint32_t val2=0;
    switch(tokens[op].type){
      case '+':
        val2 = eval(op+1,r,success); 
        return val1 + val2;
      case '-':
        val2 = eval(op+1,r,success); 
        return val1 - val2;
      case '*':
        val2 = eval(op+1,r,success); 
        return val1 * val2;
      case '/': 
        val2 = eval(op+1,r,success);
        if(val2==0){
          printf("Divide 0!\n");
          val2=1;
        }
        return val1 / val2;
      case TK_AND: 
        if(val1==0)return 0;
        val2 = eval(op+1,r,success);
        return val1 && val2;
      case TK_OR:
        if(val1!=0)return 1; 
        val2 = eval(op+1,r,success);
        return val1 || val2;
      case TK_EQ: 
        val2 = eval(op+1,r,success);
        return val1 == val2;
      case TK_NONEQUAL:
        val2 = eval(op+1,r,success); 
        return val1 != val2;
      case TK_DEREF: 
        val2 = eval(op+1,r,success);
        return vaddr_read(val2,4);
      case TK_NEGATIVE: 
        val2 = eval(op+1,r,success);
        return -val2;
      default: assert(0);
    }
  }else{
    *success=false;
    return -1;
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  pre_process();
  /* TODO: Insert codes to evaluate the expression. */
  int begin=0,end=nr_token-1;
  word_t res=eval(begin,end,success);
  return res;
}
