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
 // The system has 16KB pages and a 24-bit virtual address space.
 // addr is int32
 // (8)????????(10)NNNNNNNNNN(14)OOOOOOOOOOOOOO
static inline vpn_t vaddr_vpn(vaddr_t addr) {
    // get rid of left 8 bits
    // move 10 bits to far right (get rid of right 14 bits)
    return addr << 8 >> 22; // 2^14 = 16kb
}

static inline uint16_t vaddr_offset(vaddr_t addr) {
    // get rid of left 18 bits
    // move 14 bits to far right
    return addr << 18 >> 18; // 32 - 14 = 18
}

#pragma GCC diagnostic
