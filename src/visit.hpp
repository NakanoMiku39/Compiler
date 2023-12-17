#include "koopa.hpp"
#include <cassert>
#include <cstddef>
#include <iostream>
#include <stack>
#include <string>
#include <vector>
using namespace std;

void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value);
void Visit(const koopa_raw_function_t &func);
int Visit(const koopa_raw_integer_t &int_t);
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_return_t &ret, const koopa_raw_value_t &value);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value);
void Visit(const koopa_raw_store_t &store, const koopa_raw_value_t &value);

struct symbol {
  int value, offset, reg; // 值，位移，所在的寄存器
  koopa_raw_value_t addr; // 地址（这个变量的唯一标识符）
};

struct instack2 {
  int val, reg;
};

class riscv {
private:
  string rv;

public:
  vector<instack2> valueStack; // 栈
  vector<symbol> addrStack;    // 符号表
  int REG[15]; // 寄存器，-1是x0，0-6是t0-t6，7-14是a0-a7
  int x0 = -1; // x0寄存器，值为0
  int R = 0;   // 当前被使用寄存器的数量
  int offset = 0;

  riscv() {}

  void append(string str) { rv += str; }

  void append(int num) { rv += to_string(num); }

  void prologue(const koopa_raw_function_t &func) {
    rv = "  .text\n  .globl " + std::string(func->name + 1) + "\n" +
         std::string(func->name + 1) + ":\n  addi sp, sp -" +
         to_string(offset) + "\n" + rv;
  }

  // 结果展示
  const char *show() {
    cout << rv << endl;
    return rv.c_str();
  }

  // 查找寄存器里的数字，如果没有找到就放入到新的寄存器
  /*
  int search(int num) {
    for (int i = 0; i <= R; i++) {
      if (REG[i] == num)
        return i;
    }
    // 如果找不到数字就把数字放进一个新的寄存器里
    li(R, num);
    return R;
  }
  */

  void increment() { R = (R == 0) ? 1 : 0; }

  // 根据所谓的“地址”查找栈帧里存放的变量
  vector<symbol>::iterator search(koopa_raw_value_t addr) {
    vector<symbol>::iterator i = addrStack.begin();
    for (; i != addrStack.end(); i++) {
      if (i->addr->kind.data.integer.value == addr->kind.data.integer.value) {
        return i;
      }
    }
    return addrStack.end();
  }

  // 把寄存器从数字表示转换成字符串
  string translate(int num) {
    string reg;
    num %= 15;
    // if(num == x0)
    //  return "x0";

    if (num <= 6) {
      reg = "t" + to_string(num);
    } else {
      reg = "a" + to_string(num - 7);
    }
    return reg;
  }

  // 把立即数移入寄存器
  void li(int _reg, int _num) {
    REG[_reg] = _num;
    string reg = translate(_reg);
    rv = rv + "  li    " + reg + ", " + to_string(_num) + "\n";
    // valueStack.push_back({_num, _reg});
    increment();
  }

  // 二元操作指令
  void doubleReg(string cmd, int _reg1, int _reg2) {
    string reg1 = translate(_reg1);
    string reg2 = translate(_reg2);

    rv = rv + "  " + cmd + "    " + reg1 + ", " + reg2 + "\n";
  }

  // 三元操作指令
  void tripleReg(string cmd, int _reg1, int _reg2, int _reg3) {
    string reg1 = translate(_reg1);
    string reg2 = translate(_reg2);
    string reg3 = translate(_reg3);

    rv = rv + "  " + cmd + "    " + reg1 + ", " + reg2 + ", " + reg3 + "\n";
  }

  // 把地址存的值移到寄存器
  void lw(int _reg, koopa_raw_value_t addr) {
    string reg = translate(_reg);
    vector<symbol>::iterator i = search(addr); // 找到变量
    REG[_reg] = i->value;                      // 把值放到寄存器中
    i->reg = _reg;                             // 更新寄存器
    // valueStack.push_back({i->value, i->reg});
    rv = rv + "  lw    " + reg + ", " + to_string(i->offset) + "(sp)" + "\n";
    increment();
  }

  void sw(int _reg, koopa_raw_value_t addr, int isload) {
    string reg = translate(_reg);
    vector<symbol>::iterator i = search(addr);
    // 判断是不是一定要存入栈中，如果栈中未出现过或者是load就要存放
    if (i == addrStack.end() || isload) {
      symbol t = {REG[0], offset, _reg, addr};
      addrStack.push_back(t);
      rv = rv + "  sw    " + reg + ", " + to_string(t.offset) + "(sp)" + "\n";
      offset += 4;
    } else {
      rv = rv + "  sw    " + reg + ", " + to_string(i->offset) + "(sp)" + "\n";
    }

    increment();
  }
};
