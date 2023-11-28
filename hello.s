  .text
  .globl main
main:
  li    t0, 6
  xor    t0, t0, x0
  seqz    t0, t0
  li    t1, 1
  sub    x0, x0, t1
  li    t2, 1
  sub    x0, x0, t1
  ret

