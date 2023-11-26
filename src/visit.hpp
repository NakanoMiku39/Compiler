#include "koopa.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <iostream>
using namespace std;

void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_binary_t &binary);
void Visit(const koopa_raw_function_t &func);
int Visit(const koopa_raw_integer_t &int_t);
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_value_t &value);


class riscv {
private:
  string rv;
public:
  int REG[14];  // 寄存器，0-6是t，7-14是a
  int x0 = -1;  // x0寄存器，代表0
  int R = -1;   // 当前被使用寄存器的数量

  riscv() {}

  void append(string str) {
    rv += str;
  }

  void append(int num) {
    rv += to_string(num);
  }
  
  // 结果展示
  const char* show() {
    cout << rv << endl;
    return rv.c_str();
  }

  // 查找寄存器里的数字，如果没有找到就放入到新的寄存器
  int search(int num) {
    for(int i = 0; i <= R; i++) {
      if(REG[i] == num)
        return i;
    }
    return _new(num);
    // return -1;
  }

  // 把寄存器从数字表示转换成字符串
  string translate(int num) {
    string reg;
    if(num == x0)
      return "x0";

    if(num <= 6) {
      reg = "t" + to_string(num);
    }
    else {
      reg = "a" + to_string(num - 7);
    }
    return reg;
  }

  // 把数字放到一个新的寄存器
  int _new(int _num) {
    R += 1;
    REG[R] = _num;

    li(R, _num);
    cout << "Putting new number " << _num << " at " << R << endl;
    return R;
  }

  // 把立即数移入寄存器
  void li(int _reg, int _num) {
    // 找出目标寄存器
    string reg = translate(_reg);
    rv = rv + "  li    " + reg + ", " 
      + to_string(_num) + "\n";
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


  void ge(int _reg1, int _reg2) {
    string reg1 = translate(_reg1);
    string reg2 = translate(_reg2);

    rv = rv + "  ge   " + reg1 + ", " + reg2 + "\n";
  }
  void le(int _reg1, int _reg2) {
    string reg1 = translate(_reg1);
    string reg2 = translate(_reg2);

    rv = rv + "  le   " + reg1 + ", " + reg2 + "\n";
  }
  void gt(int _reg1, int _reg2) {
    string reg1 = translate(_reg1);
    string reg2 = translate(_reg2);

    rv = rv + "  gt   " + reg1 + ", " + reg2 + "\n";
  }
  void lt(int _reg1, int _reg2) {
    string reg1 = translate(_reg1);
    string reg2 = translate(_reg2);

    rv = rv + "  lt   " + reg1 + ", " + reg2 + "\n";
  }

  void add(int _reg1, int _reg2) {
    string reg1 = translate(_reg1);
    string reg2 = translate(_reg2);

    rv = rv + "  add   " + reg1 + ", " + reg2 + "\n";
  }

  void sub(int _reg1, int _reg2) {
    string reg1 = translate(_reg1);
    string reg2 = translate(_reg2);

    rv = rv + "  sub   " + reg1 + ", " + reg2 + "\n";
  }

  void mul(int _reg1, int _reg2) {
    string reg1 = translate(_reg1);
    string reg2 = translate(_reg2);

    rv = rv + "  mul   " + reg2 + ", " + reg1 + ", " + reg2 + "\n";
  }

  void div(int _reg1, int _reg2) {
    string reg1 = translate(_reg1);
    string reg2 = translate(_reg2);

    rv = rv + "  div   " + reg1 + ", " + reg2 + "\n";
  }

  void _and(int _reg, int num) {
    rv = rv + "  and  ";
  }

  void _or(int _reg, int num) {
    rv = rv + "  or  ";
  }

  void _xor(int _reg1, int _reg2) {
    string reg1 = translate(_reg1);
    string reg2 = translate(_reg2);

    rv = rv + "  xor   " + reg1 + ", " + reg1 + ", "+ reg2 + "\n";
  }
  void ret(int _reg1) {
    string reg1 = translate(_reg1);
    //mv("a0", reg1);
    rv += "  ret";
  }

  void mv(int _reg1, int _reg2) {
    string reg1 = translate(_reg1);
    string reg2 = translate(_reg2);

    rv = rv + "  mv   " + reg1 + ", " + reg2 + "\n";
  }

};
