  .text
  .globl main
main:
  addi sp, sp, -12
  li    t0, 11
  li    t1, 0
  xor    t0, t0, t1
  snez    t0, t0
  sw    t0, 0(sp)

  li    t0, 0
  li    t1, 0
  xor    t0, t0, t1
  snez    t0, t0
  sw    t0, 4(sp)

  lw    t0, 0(sp)
  lw    t1, 4(sp)
  or    t0, t0, t1
  sw    t0, 8(sp)

  lw    a0, 8(sp)
  addi sp, sp, 12
  ret


