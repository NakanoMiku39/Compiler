  .text
  .globl main
main:
  addi sp, sp, -68
entry:

  li    t0, 10
  sw    t0, 0(sp)

  lw    t0, 0(sp)
  sw    t0, 4(sp)

  lw    t0, 4(sp)
  li    t1, 1
  sgt    t0, t0, t1
  sw    t0, 8(sp)

  lw    t0, 8(sp)
  bnez    t0, then_1
  j    end_1

then_1:
  lw    t0, 0(sp)
  sw    t0, 12(sp)

  lw    t0, 12(sp)
  li    t1, 2
  sgt    t0, t0, t1
  sw    t0, 16(sp)

  lw    t0, 16(sp)
  bnez    t0, then_2
  j    end_2

end_1:
  li    t0, 0
  li    t1, 1
  sub    t0, t0, t1
  sw    t0, 20(sp)

  lw    a0, 20(sp)
  addi sp, sp, 24
  ret
then_2:
  lw    t0, 0(sp)
  sw    t0, 24(sp)

  lw    t0, 24(sp)
  li    t1, 3
  slt    t0, t0, t1
  sw    t0, 28(sp)

  lw    t0, 28(sp)
  bnez    t0, then_3
  j    else_3

end_2:
  j    end_1

then_3:
  lw    t0, 0(sp)
  sw    t0, 32(sp)

  lw    a0, 32(sp)
  addi sp, sp, 36
  ret
else_3:
  lw    t0, 0(sp)
  sw    t0, 36(sp)

  lw    t0, 36(sp)
  li    t1, 4
  sgt    t0, t0, t1
  sw    t0, 40(sp)

  lw    t0, 40(sp)
  bnez    t0, then_4
  j    end_4

then_4:
  lw    t0, 0(sp)
  sw    t0, 44(sp)

  lw    t0, 44(sp)
  li    t1, 5
  slt    t0, t0, t1
  sw    t0, 48(sp)

  lw    t0, 48(sp)
  bnez    t0, then_5
  j    else_5

end_4:
  j    end_3

then_5:
  lw    t0, 0(sp)
  sw    t0, 52(sp)

  lw    t0, 52(sp)
  li    t1, 1
  add    t0, t0, t1
  sw    t0, 56(sp)

  lw    a0, 56(sp)
  addi sp, sp, 60
  ret
else_5:
  lw    t0, 0(sp)
  sw    t0, 60(sp)

  lw    t0, 60(sp)
  li    t1, 2
  add    t0, t0, t1
  sw    t0, 64(sp)

  lw    a0, 64(sp)
  addi sp, sp, 68
  ret
end_3:
  j    end_2



