  .text
  .globl main
main:
  li    t0, 1
  li    t1, 0
  xor    t0, t0, t1
  snez    t0, t0
  li    t2, 1
  li    t3, 0
  xor    t0, t0, t1
  snez    t0, t0
  and    t0, t0, t0
  li    t4, 1
  li    t5, 0
  xor    t0, t0, t1
  snez    t0, t0
  li    t6, 22
  li    a0, 0
  xor    t6, t6, t1
  snez    t6, t6
  and    t0, t0, t0
  li    a1, 0
  xor    t0, t0, t1
  snez    t0, t0
  li    a2, 0
  xor    t0, t0, t1
  snez    t0, t0
  or    t0, t0, t0
  li    a3, 0
  li    a4, 0
  xor    t1, t1, t1
  snez    t1, t1
  li    a5, 10
  li    a6, 0
  xor    a5, a5, t1
  snez    a5, a5
  or    t0, t0, t1
  li    a7, 0
  xor    t0, t0, t1
  snez    t0, t0
  li    t0, 0
  xor    t0, t0, t1
  snez    t0, t0
  and    t0, t0, t0
  li    a0, 1
  ret

