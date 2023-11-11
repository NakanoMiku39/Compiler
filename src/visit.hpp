#include "koopa.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <iostream>

void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t &ret);
int Visit(const koopa_raw_integer_t &int_t);

class riscv {
private:
  std::string rv;
public:
  riscv() {}

  void append(std::string str) {
    rv += str;
  }

  void append(int num) {
    rv += std::to_string(num);
  }

  const char* show() {
    std::cout << rv << std::endl;
    return rv.c_str();
  }

};
