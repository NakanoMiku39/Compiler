  .text
  .globl main
main:
  addi sp, sp, -112

  li    t0, 10
  sw    t0, 0(sp) // a = 10

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

  lw    t0, 12(sp) // a
  li    t1, 1 // 1
  sub    t0, t0, t1 // a - 1
  sw    t0, 16(sp)

  lw    t0, 0(sp) 
  sw    t0, 20(sp)

  lw    t0, 16(sp)
  sw    t0, 0(sp) // a = a - 1


  li    t0, 5
  sw    t0, 24(sp)

  lw    t0, 24(sp)
  sw    t0, 28(sp)

  li    t0, 0
  li    t1, 1
  sub    t0, t0, t1
  sw    t0, 32(sp)

  lw    t0, 28(sp)
  lw    t1, 32(sp)
  slt    t0, t0, t1
  sw    t0, 36(sp)

  lw    t0, 36(sp)
  bnez    t0, then_2
  j    else_2

end_1:
  lw    t0, 0(sp)
  sw    t0, 40(sp)

  lw    t0, 40(sp)
  li    t1, 9
  xor    t0, t0, t1
  seqz    t0, t0
  sw    t0, 44(sp)

  lw    t0, 44(sp)
  bnez    t0, then_5
  j    end_5

then_2:
  li    a0, 10
  addi sp, sp, 48
  ret
else_2:

  sw    t0, 48(sp)
  lw    t0, 48(sp)
  sw    t0, 52(sp)

  li    t0, 98
  sw    t0, 48(sp)


then_5:
// int b = a - 1
  lw    t0, 0(sp) // 9
  sw    t0, 56(sp)

  lw    t0, 56(sp) // a
  li    t1, 1 // 1
  sub    t0, t0, t1 // a - 1
  sw    t0, 60(sp) // b = a - 1

  lw    t0, 60(sp)
  sw    t0, 64(sp) // b = 8

 // int a = b - 1
  lw    t0, 64(sp) // 8
  sw    t0, 68(sp)

  lw    t0, 68(sp)
  li    t1, 1
  sub    t0, t0, t1 // 7
  sw    t0, 72(sp)

  lw    t0, 72(sp)
  sw    t0, 76(sp)
//
  lw    t0, 76(sp) 
  sw    t0, 80(sp) // 7

  lw    t0, 64(sp)
  sw    t0, 84(sp) // 8

  lw    t0, 80(sp)
  lw    t1, 84(sp)
  xor    t0, t0, t1
  snez    t0, t0
  sw    t0, 88(sp)

  lw    t0, 88(sp)
  bnez    t0, then_6
  j    else_6

end_5:
  li    t0, 0
  li    t1, 1
  sub    t0, t0, t1
  sw    t0, 92(sp)

  lw    a0, 92(sp)
  addi sp, sp, 96
  ret
then_6:

else_6:
  lw    t0, 76(sp)
  sw    t0, 96(sp)

  lw    t0, 96(sp)
  li    t1, 0
  xor    t0, t0, t1
  seqz    t0, t0
  sw    t0, 100(sp)

  lw    t0, 100(sp)
  bnez    t0, then_7
  j    end_7

then_7:
  lw    t0, 64(sp)
  sw    t0, 104(sp)

  lw    a0, 104(sp)
  addi sp, sp, 108
  ret
end_7:
  li    a0, 0
  addi sp, sp, 108
  ret
  lw    t0, 76(sp)
  sw    t0, 108(sp)

  lw    a0, 108(sp)
  addi sp, sp, 112
  ret


