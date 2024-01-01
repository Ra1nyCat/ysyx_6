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

#include <memory/host.h>
#include <memory/paddr.h>
#include <device/mmio.h>
#include <isa.h>

//根据编译配置来决定物理内存的实现方式
#if   defined(CONFIG_PMEM_MALLOC)
static uint8_t *pmem = NULL;
#else // CONFIG_PMEM_GARRAY
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};
#endif

//客户端物理地址转换为主机物理地址，客户端模拟的起始地址为CONFIG_MBASE  主机地址起始地址为pmem
uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

//读取物理内存physical memory_read
static word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

//写物理内存physical memory_write
static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
}

//地址越界检查
static void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);
}

//初始化内存
void init_mem() {
#if   defined(CONFIG_PMEM_MALLOC)
  pmem = malloc(CONFIG_MSIZE);
  assert(pmem);
#endif
#ifdef CONFIG_MEM_RANDOM
  uint32_t *p = (uint32_t *)pmem;
  int i;
  for (i = 0; i < (int) (CONFIG_MSIZE / sizeof(p[0])); i ++) {
    p[i] = rand();
  }
#endif
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);
}

void mtrace_print(paddr_t addr, int len, bool is_write) {
  static FILE *mfp = NULL;
  if (mfp == NULL) {
    // fp = fopen("mem_trace.log", "w");
    mfp=stdout;
    assert(mfp);
  }
  paddr_t right_addr=addr+len-1;
  fprintf(mfp, "%c " FMT_PADDR " " FMT_PADDR " %d\n", is_write ? 'W' : 'R', addr, right_addr, len);
}

bool memory_print_enable(paddr_t addr) {
  paddr_t really_addr=addr-0x80000000;
  return MUXDEF(CONFIG_MTRACE, (really_addr >= CONFIG_MTRACE_START) &&
         (really_addr <= CONFIG_MTRACE_END), false);
}

word_t paddr_read(paddr_t addr, int len) {

  #ifdef CONFIG_MTRACE //内存追踪 
    if(memory_print_enable(addr))
      mtrace_print(addr, len, false);
  #endif

  if (likely(in_pmem(addr))) return pmem_read(addr, len);
  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));
  out_of_bound(addr);
  return 0;
}

void paddr_write(paddr_t addr, int len, word_t data) {

  #ifdef CONFIG_MTRACE
    if(memory_print_enable(addr))
      mtrace_print(addr, len, true);
  #endif

  if (likely(in_pmem(addr))) { pmem_write(addr, len, data); return; }
  IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
  out_of_bound(addr);
}
