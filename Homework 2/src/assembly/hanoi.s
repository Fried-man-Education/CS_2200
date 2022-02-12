!============================================================
! CS 2200 Homework 2 Part 2: Tower of Hanoi
!
! Apart from initializing the stack,
! please do not edit mains functionality.
!============================================================

main:
    lea     $sp, stack              ! load the label address into $sp
    lw      $sp, 0($sp)             ! $sp to load val into $sp
    lea     $fp, stack              ! load the label address into $fp
    lw      $fp, 0($fp)             ! $fp to load val into $fp

    lea     $at, hanoi              ! loads address of hanoi label into $at

    lea     $a0, testNumDisks2      ! loads address of number into $a0
    lw      $a0, 0($a0)             ! loads value of number into $a0

    jalr    $ra, $at                ! jump to hanoi, set $ra to return addr
    halt                            ! when we return, just halt

hanoi:
    ! perform post-call portion of the calling convention. Make sure to save any registers you will be using!
    addi    $sp, $sp, -1            ! allocate space for og frame pointer
    sw      $fp, 0($sp)             ! store $fp into mem location at current $sp
    addi    $fp, $sp, 0             ! store $sp into $fp

    ! ($a0 == 1) ? GOTO base : GOTO else;
    addi    $t0, $zero, 1           ! $t0 = 1
    bgt     $a0, $t0, else          ! if ($t0 > 1) GOTO else
    br       base                   ! GOTO else if ($a0 == 1)

else:
    ! perform recursion after decrementing the parameter by 1. Remember, $a0 holds the parameter value.
    addi    $a0, $a0, -1            ! n--
    lea     $at, hanoi              ! load the address of hanoi
    addi    $sp, $sp, -2            ! push 2 spots on stack
    sw      $ra, -1($fp)            ! store RA in stack[-1]
    sw      $a0, -2($fp)            ! store arg 0 in stack[-2]
    jalr    $ra, $at                ! hanoi()
    lw      $ra, -1($fp)            ! load RA from the stack
    addi    $sp, $sp, 2             ! pop 2 spots off stack

    ! $v0 = 2 * $v0 + 1; RETURN $v0;
    add     $v0, $v0, $v0           ! $v0 *= 2
    addi    $v0, $v0, 1             ! $v0++
    br      teardown                ! GOTO teardown

base:
    addi     $v0, $v0, 1            ! return 1

teardown:
    ! perform pre-return portion of the calling convention
    lw      $fp, 0($fp)             ! restore old frame pointer
    addi    $sp, $sp, 1             ! pop stack

    jalr    $zero, $ra              ! return to caller



stack: .word 0xFFFF                 ! the stack begins here


! Words for testing \/

! 1
testNumDisks1:
    .word 0x0001

! 10
testNumDisks2:
    .word 0x000a

! 20
testNumDisks3:
    .word 0x0014
