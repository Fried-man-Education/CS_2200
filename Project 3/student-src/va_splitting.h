#pragma once

#include "mmu.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * --------------------------------- PROBLEM 1 --------------------------------------
 * Checkout PDF Section 3 For this Problem
 *
 * Split the virtual address into its virtual page number and offset.
 *
 * HINT:
 *      -Examine the global defines in pagesim.h, which will be necessary in
 *      implementing these functions.
 * ----------------------------------------------------------------------------------
 */
int numBits = sizeof(vaddr_t)*8; // vaddr_t is 32 bits

 // The system has 16KB (2^14) pages and a 24-bit virtual address space.
 // (8)????????(10)NNNNNNNNNN(14)OOOOOOOOOOOOOO
static inline vpn_t vaddr_vpn(vaddr_t addr) {
    return addr
      << numBits - VADDR_LEN /* get rid of left (32-24=8) bits */
      >> numBits - (VADDR_LEN - OFFSET_LEN); // move 10 bits to far right
}

static inline uint16_t vaddr_offset(vaddr_t addr) {

    int shift = numBits - OFFSET_LEN; // 18 = 32 - 14
    return addr
      << shift /* get rid of left 18 bits */
      >> shift; // move 14 bits to far right
}

#pragma GCC diagnostic
