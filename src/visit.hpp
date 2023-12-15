#include "koopa.hpp"
#include <cassert>
#include <iostream>
#include <stack>
#include <string>
#include <vector>
using namespace std;

void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_binary_t &binary);
void Visit(const koopa_raw_function_t &func);
int Visit(const koopa_raw_integer_t &int_t);
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value);
void Visit(const koopa_raw_store_t &store);

struct symbol {
  int value, offset;
  koopa_raw_value_t addr;
};

class riscv {
private:
  string rv;

public:
  vector<int> valueStack;   // 栈
  vector<symbol> addrStack; // 符号表
  int REG[15];              // 寄存器，-1是x0，0-6是t0-t6，7-14是a0-a7
  int x0 = -1;              // x0寄存器，值为0
  int R = 0;                // 当前被使用寄存器的数量
  int sp, offset = 0;

  riscv() {}

  void append(string str) { rv += str; }

  void append(int num) { rv += to_string(num); }

  // 结果展示
  const char *show() {
    cout << rv << endl;
    return rv.c_str();
  }

  // 查找寄存器里的数字，如果没有找到就放入到新的寄存器
  int search(int num) {
    for (int i = 0; i <= R; i++) {
      if (REG[i] == num)
        return i;
    }
    // 如果找不到数字就把数字放进一个新的寄存器里
  }

  // 根据所谓的“地址”查找栈帧里存放的变量
  vector<symbol>::iterator search(koopa_raw_value_t addr) {
    vector<symbol>::iterator i = addrStack.begin();
    for (; i != addrStack.end(); i++) {
      if (i->addr->kind.data.integer.value == addr->kind.data.integer.value) {
        return i;
      }
    }
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
    REG[R] = _num;
    valueStack.push_back(_num);
    string reg = translate(_reg);
    rv = rv + "  li    " + reg + ", " + to_string(_num) + "\n";
    R += 1;
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

  void lw(int _reg, koopa_raw_value_t addr) {
    string reg = translate(_reg);
    vector<symbol>::iterator i = search(addr);
    rv = rv + "  lw    " + reg + ", " + to_string(i->offset) + "(sp)" + "\n";
    // offset += 4;
  }

  void sw(int _reg, koopa_raw_value_t addr) {
    string reg = translate(_reg);
    symbol t = {REG[_reg], offset, addr};
    addrStack.push_back(t);
    rv = rv + "  sw    " + reg + ", " + to_string(t.offset) + "(sp)" + "\n";
    offset += 4;
  }
};
