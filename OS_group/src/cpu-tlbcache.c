/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee
 * a personal to use and modify the Licensed Source Code for
 * the sole purpose of studying during attending the course CO2018.
 */
// #ifdef MM_TLB
/*
 * Memory physical based TLB Cache
 * TLB cache module tlb/tlbcache.c
 *
 * TLB cache is physically memory phy
 * supports random access
 * and runs at high speed
 */

#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
static pthread_mutex_t tlbcache_lock = PTHREAD_MUTEX_INITIALIZER;
// #define init_tlbcache(mp,sz,...) init_memphy(mp, sz, (1, ##__VA_ARGS__))

/*
 *  tlb_cache_read read TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */
int tlb_cache_read(struct tlb_struct *mp, int pid, int pgnum, uint16_t *fpn_num)
{
   /* TODO: the identify info is mapped to
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    */
   pthread_mutex_lock(&tlbcache_lock);
   if (!mp || !fpn_num)
   {
      pthread_mutex_unlock(&tlbcache_lock);

      return -1;
   }
   int index = pgnum % mp->maxsz;
   if (GET_VALID_TLB(mp->tlbd[index]) == 1)
   {
      int pidCheck = GET_TLB_PID(mp->tlbd[index]);
      int pgCheck = GET_TLB_PGN(mp->tlbd[index]);
      if (pidCheck == pid && pgCheck == pgnum)
      {

         *fpn_num = GET_TLB_FPN(mp->tlbd[index]);
         pthread_mutex_unlock(&tlbcache_lock);

         return 0;
      }
   }
   pthread_mutex_unlock(&tlbcache_lock);

   return -1;
}

/*
 *  tlb_cache_write write TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */
int tlb_cache_write(struct tlb_struct *mp, int pid, int pgnum, uint16_t fpn_num)
{
   /* TODO: the identify info is mapped to
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    */
   pthread_mutex_lock(&tlbcache_lock);

   if (!mp)
   {
      pthread_mutex_unlock(&tlbcache_lock);

      return -1;
   }
   printf("write TLB pgnum=%d, fpn_num=%d, pid=%d\n", pgnum, fpn_num, pid);
   int index = pgnum % mp->maxsz;
   SET_VALID_TLB(mp->tlbd[index]);
   SET_TLB_PID(mp->tlbd[index], pid);
   SET_TLB_PGN(mp->tlbd[index], pgnum);
   SET_TLB_FPN(mp->tlbd[index], fpn_num);
   TLBMEMPHY_dump(mp);
   pthread_mutex_unlock(&tlbcache_lock);

   return 0;
}

/*
 *  TLBMEMPHY_read natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @value: obtained value
 */
// int TLBMEMPHY_read(struct tlb_struct *mp, int addr, BYTE *value)
// {
//    if (mp == NULL)
//       return -1;

//    /* TLB cached is random access by native */
//    *value = mp->storage[addr];

//    return 0;
// }

/*
 *  TLBMEMPHY_write natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data
 */
// int TLBMEMPHY_write(struct tlb_struct *mp, int addr, BYTE data)
// {
//    if (mp == NULL)
//       return -1;
//    /* TLB cached is random access by native */
//    mp->storage[addr] = data;

//    return 0;
// }

/*
 *  TLBMEMPHY_format natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 */

int TLBMEMPHY_dump(struct tlb_struct *mp)
{
   /*TODO dump memphy contnt mp->storage
    *     for tracing the memory content
    */
   printf("\n");
   printf("print TLB\n");
   for (int i = 0; i < mp->maxsz; i++)
   {
      if (GET_VALID_TLB(mp->tlbd[i]) == 1)
      {
         printf("%08x ->", mp->tlbd[i]);
         printf("pgnum:%d, fpn_num:%d, pid:%d\n", GET_TLB_PGN(mp->tlbd[i]), GET_TLB_FPN(mp->tlbd[i]), GET_TLB_PID(mp->tlbd[i]));

         // printf("pgnum:%d\n", GET_TLB_PGN(mp->tlbd[i]));
      }
   }
   // printf("\n");

   return 0;
}

/*
 *  Init TLBMEMPHY struct
 */
int init_tlbmemphy(struct tlb_struct *mp, int max_size)
{
   max_size /= 4;
   mp->maxsz = max_size;
   mp->tlbd = (uint32_t *)malloc(max_size * sizeof(uint32_t));
   for (int i = 0; i < max_size; i++)
   {
      mp->tlbd[i] = 0;
   }
   mp->hit = 0;
   mp->total_access = 0;
   return 0;
}

// #endif
