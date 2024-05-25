/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee
 * a personal to use and modify the Licensed Source Code for
 * the sole purpose of studying during attending the course CO2018.
 */
// #ifdef CPU_TLB
/*
 * CPU TLB
 * TLB module cpu/cpu-tlb.c
 */

#include "mm.h"
#include <stdlib.h>
#include <stdio.h>

int tlb_change_all_page_tables_of(struct pcb_t *proc, struct tlb_struct *mp)
{
  /* TODO update all page table directory info
   *      in flush or wipe TLB (if needed)
   */
  TLBMEMPHY_dump(mp);
  for (int i = 0; i < mp->maxsz; i++)
  {
    mp->tlbd[i] = 0;
  }

  return 0;
}

int tlb_flush_tlb_of(struct pcb_t *proc, struct tlb_struct *mp)
{
  int pid = proc->pid;
  for (int i = 0; i < mp->maxsz; i++)
  {
    if (pid != GET_TLB_PID(mp->tlbd[i]))
      mp->tlbd[i] = 0;
  }
  return 0;
}

/*tlballoc - CPU TLB-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int addr, val;
  /* By default using vmaid = 0 */
  val = __alloc(proc, 0, reg_index, size, &addr);
  /* TODO update TLB CACHED frame num of the new allocated page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  printf("alloc %d page\n", PAGING_PAGE_ALIGNSZ(size) / PAGING_PAGESZ);
  for (int i = addr; i < addr + PAGING_PAGE_ALIGNSZ(size); i += PAGING_PAGESZ)
  {
    tlb_cache_write(proc->tlb, proc->pid,
                    PAGING_PGN(i),
                    PAGING_FPN_v2(proc->mm->pgd[PAGING_PGN(i)]));
  }
  return val;
}

/*pgfree - CPU TLB-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlbfree_data(struct pcb_t *proc, uint32_t reg_index)
{
  struct vm_rg_struct temp = proc->mm->symrgtbl[reg_index]; 
  for (int i = temp.rg_start; i < temp.rg_end; i += PAGING_PAGESZ)
  {
    int pgn = PAGING_PGN(i);
    // printf("pgn %d", pgn);
    // printf("maxsz %d", proc->tlb->maxsz);
    // printf("maxsz %d", proc->tlb->maxsz);
    if (proc->tlb->tlbd[pgn % proc->tlb->maxsz] != 0)
    {
      printf("free TLB pgnum=%d, fpn_num=%d,pid=%d\n",
             GET_TLB_PGN(proc->tlb->tlbd[pgn % proc->tlb->maxsz]),
             GET_TLB_FPN(proc->tlb->tlbd[pgn % proc->tlb->maxsz]),
             GET_TLB_PID(proc->tlb->tlbd[pgn % proc->tlb->maxsz]));
      int pidCheck = GET_TLB_PID(proc->tlb->tlbd[pgn % proc->tlb->maxsz]);
      if (pidCheck == proc->pid)
        proc->tlb->tlbd[pgn % proc->tlb->maxsz] = 0;

      TLBMEMPHY_dump(proc->tlb);
    }
  }
  __free(proc, 0, reg_index);

  return 0;
}

/*tlbread - CPU TLB-based read a region memory
 *@proc: Process executing the instruction
 *@source: index of source register
 *@offset: source address = [source] + [offset]
 *@destination: destination storage
 */
int tlbread(struct pcb_t *proc, uint32_t source,
            uint32_t offset, uint32_t destination)
{
  /* TODO retrieve TLB CACHED frame num of accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  /* frmnum is return value of tlb_cache_read/write value*/
  BYTE data;
  int pgnum = PAGING_PGN(proc->mm->symrgtbl[source].rg_start); // ko cong offfset
  uint16_t frnum;
  int return_value = tlb_cache_read(proc->tlb, proc->pid, pgnum, &frnum);
  proc->tlb->total_access++;
#ifdef IODUMP
  if (return_value >= 0)
  {
    printf("TLB hit at read region=%d offset=%d\n",
           source, offset); //?offset
  }
  else
  {
    printf("TLB miss at read region=%d offset=%d\n",
           source, offset);
  }
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); 
#endif
  MEMPHY_dump(proc->mram);
#endif
  int val;
  {
    if (return_value >= 0)
    {
      proc->tlb->hit++;
      int phyaddr = (frnum << PAGING_ADDR_FPN_LOBIT) + offset;
      // printf("phyaddr %d\n", phyaddr);
      // printf("rdmflg %d\n", proc->mram->rdmflg);
      // printf("mram->size %d\n", proc->mram->maxsz);
      val = MEMPHY_read(proc->mram, phyaddr, &data);
      printf("read data pgn=%d, fpn_num=%d,pid=%d, data=%d, phyaddr=%d\n",
             GET_TLB_PGN(proc->tlb->tlbd[pgnum % proc->tlb->maxsz]),
             GET_TLB_FPN(proc->tlb->tlbd[pgnum % proc->tlb->maxsz]),
             GET_TLB_PID(proc->tlb->tlbd[pgnum % proc->tlb->maxsz]),
             data, phyaddr);
    }
    else
    {
      //__read
      val = __read(proc, 0, source, offset, &data);
      tlb_cache_write(proc->tlb, proc->pid, pgnum, PAGING_FPN_v2(proc->mm->pgd[pgnum]));
      printf("read data pgn=%d, fpn_num=%d,pid=%d, data=%d\n",
             GET_TLB_PGN(proc->tlb->tlbd[pgnum % proc->tlb->maxsz]),
             GET_TLB_FPN(proc->tlb->tlbd[pgnum % proc->tlb->maxsz]),
             GET_TLB_PID(proc->tlb->tlbd[pgnum % proc->tlb->maxsz]),
             data);
    }
  }
  destination = (uint32_t)data;
  /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  return val;
}

/*tlbwrite - CPU TLB-based write a region memory
 *@proc: Process executing the instruction
 *@data: data to be wrttien into memory
 *@destination: index of destination register
 *@offset: destination address = [destination] + [offset]
 */
int tlbwrite(struct pcb_t *proc, BYTE data,
             uint32_t destination, uint32_t offset)
{
  int val;
  //  BYTE frmnum = -1;

  int pgnum = PAGING_PGN(proc->mm->symrgtbl[destination].rg_start);
  uint16_t frnum;
  int return_value = tlb_cache_read(proc->tlb, proc->pid, pgnum, &frnum);
  proc->tlb->total_access++;
  /* TODO retrieve TLB CACHED frame num of accessing page(s))*/
  /* by using tlb_cache_read()/tlb_cache_write()
  frmnum is return value of tlb_cache_read/write value*/

  {

    if (return_value >= 0)
    {
      proc->tlb->hit++;

      int phyaddr = (frnum << PAGING_ADDR_FPN_LOBIT) + offset;
      // printf("phyaddr %d\n", phyaddr);
      // printf("rdmflg %d\n", proc->mram->rdmflg);
      // printf("mram->size %d\n", proc->mram->maxsz);
      printf("write data pgn=%d, fpn_num=%d, pid=%d, data=%d, phyaddr=%d\n",
             GET_TLB_PGN(proc->tlb->tlbd[pgnum % proc->tlb->maxsz]),
             GET_TLB_FPN(proc->tlb->tlbd[pgnum % proc->tlb->maxsz]),
             GET_TLB_PID(proc->tlb->tlbd[pgnum % proc->tlb->maxsz]),
             data, phyaddr);
      val = MEMPHY_write(proc->mram, phyaddr, data);
      // printf("data %d\n", data);
    }
    else
    {
      //__write
      val = __write(proc, 0, destination, offset, data);

      tlb_cache_write(proc->tlb, proc->pid, pgnum, PAGING_FPN_v2(proc->mm->pgd[pgnum]));
      printf("write data pgn=%d, fpn_num=%d, pid=%d, data=%d\n",
             GET_TLB_PGN(proc->tlb->tlbd[pgnum % proc->tlb->maxsz]),
             GET_TLB_FPN(proc->tlb->tlbd[pgnum % proc->tlb->maxsz]),
             GET_TLB_PID(proc->tlb->tlbd[pgnum % proc->tlb->maxsz]),
             data);
    }
  }
#ifdef IODUMP
  if (return_value >= 0)
    printf("TLB hit at write region=%d offset=%d value=%d\n",
           destination, offset, data);
  else
    printf("TLB miss at write region=%d offset=%d value=%d\n",
           destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); // print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif
  /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  return val;
}

// #endif
