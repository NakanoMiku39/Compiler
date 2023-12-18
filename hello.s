  .text
  .globl main
main:
  addi sp, sp, -16

  sw    t0, 0(sp)
  lw    t0, 0(sp)
  sw    t0, 4(sp)

  li    t0, 1
  lw    t1, 4(sp)
  add    t0, t0, t1
  sw    t0, 8(sp)

  lw    t0, 8(sp)
  sw    t0, 0(sp)

  lw    t0, 0(sp)
  sw    t0, 12(sp)

  lw    a0, 12(sp)
  addi sp, sp, 16
  ret


