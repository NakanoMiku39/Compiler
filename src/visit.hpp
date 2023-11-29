#include "koopa.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <stack>
using namespace std;

void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_binary_t &binary);
void Visit(const koopa_raw_function_t &func);
int Visit(const koopa_raw_integer_t &int_t);
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_value_t &value);

struct symbol {
  string name;
  int value;
};

class riscv {
private:
  string rv;
public:
  vector<int> STACK; // 栈
  struct symbol symbolTable;   // 符号表
  int REG[15];  // 寄存器，-1是x0，0-6是t0-t6，7-14是a0-a7
  int x0 = -1;  // x0寄存器，值为0
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
    // 如果找不到数字就把数字放进一个新的寄存器里
    return _new(num);
  }

  // 把数字放到一个新的寄存器
  int _new(int _num) {
    // cout << "new number " << _num  << endl;
    R += 1;
    REG[R] = _num;
    // STACK.push_back(_num);

    li(R, _num);
    cout << "Putting new number " << _num << " at " << R << endl;
    return R;
  }

  // 把寄存器从数字表示转换成字符串
  string translate(int num) {
    string reg;
    num %= 15;
    // if(num == x0)
    //   return "x0";

    if(num <= 6) {
      reg = "t" + to_string(num);
    }
    else {
      reg = "a" + to_string((num - 7));
    }
    return reg;
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

};
