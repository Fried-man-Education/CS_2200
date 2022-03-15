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
// The system has 16KB (2^14) pages and a 24-bit virtual address space.
// (8)????????(10)NNNNNNNNNN(14)OOOOOOOOOOOOOO

static inline vpn_t vaddr_vpn(vaddr_t addr) {
    return addr / PAGE_SIZE;
}

static inline uint16_t vaddr_offset(vaddr_t addr) {
    return addr % PAGE_SIZE;
}

#pragma GCC diagnostic
