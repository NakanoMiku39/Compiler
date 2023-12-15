// 函数声明略
// ...
#include "visit.hpp"
#include <string>
#include <vector>

riscv rv;

// 访问 raw program
void Visit(const koopa_raw_program_t &program) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有全局变量
  Visit(program.values);
  // 访问所有函数
  Visit(program.funcs);
  // rv.show();
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
    case KOOPA_RSIK_FUNCTION:
      // 访问函数
      Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
      break;
    case KOOPA_RSIK_BASIC_BLOCK:
      // 访问基本块
      Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
      break;
    case KOOPA_RSIK_VALUE:
      // 访问指令
      Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
      break;
    default:
      // 我们暂时不会遇到其他内容, 于是不对其做任何处理
      assert(false);
    }
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func) {
  // std::cout << "Functions" << std::endl;

  // 执行一些其他的必要操作
  // ...
  rv.append("  .text\n");
  rv.append("  .globl " + std::string(func->name + 1) + "\n");
  rv.append(std::string(func->name + 1) + ":\n");

  /*
    //  函数的 prologue
      if(lva.delta)
          rvs.sp(-(int)lva.delta);
      if(lva.R){
          rvs.store("ra", "sp", (int)lva.delta - 4);
      }


      // 找到entry block
      size_t e = 0;
      for(e = 0; e < func->bbs.len; ++e){
          auto ptr =
    reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[e]); if(ptr->name
    && !strcmp(ptr->name, "%entry")){ break;
          }
      }
      // 访问 entry block
      Visit(reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[e]));
  */

  // 访问所有基本块
  for (size_t j = 0; j < func->bbs.len; ++j) {
    assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
    auto ptr = func->bbs.buffer[j];
    Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
  }
  rv.append("\n\n");
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  // std::cout << "blocks" << std::endl;

  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  for (size_t k = 0; k < bb->insts.len; ++k) {
    auto ptr = bb->insts.buffer[k];
    Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
  }
}

// 访问指令
void Visit(const koopa_raw_value_t &value) {
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  switch (kind.tag) {
  case KOOPA_RVT_ALLOC:
    // 访问 alloc 指令
    // Visit("alloc", value);
    break;

  case KOOPA_RVT_BINARY:
    // 访问 binary 指令
    Visit(kind.data.binary);
    break;

  case KOOPA_RVT_BRANCH:
    // 访问 branch 指令
    // Visit(kind.data.branch);
    break;

  case KOOPA_RVT_CALL:
    // 访问 call 指令
    // Visit(kind.data.call, value);
    break;

  case KOOPA_RVT_GET_ELEM_PTR:
    // 访问 getelemptr 指令
    // Visit(kind.data.get_elem_ptr, value);
    break;

  case KOOPA_RVT_GLOBAL_ALLOC:
    // 访问 global alloc 指令
    // Visit(value->kind.data.global_alloc, value);
    break;

  case KOOPA_RVT_INTEGER:
    // 访问 integer 指令
    Visit(kind.data.integer);
    break;

  case KOOPA_RVT_JUMP:
    // 访问 jump 指令
    // Visit(kind.data.jump);
    break;

  case KOOPA_RVT_LOAD:
    // 访问 load 指令
    Visit(kind.data.load, value);
    break;

  case KOOPA_RVT_RETURN:
    // 访问 return 指令
    Visit(kind.data.ret);
    break;

  case KOOPA_RVT_STORE:
    // 访问 store 指令
    Visit(kind.data.store);
    break;

  default:
    // 其他类型暂时遇不到
    break;
  }
}

// 访问对应类型指令的函数定义略
// 视需求自行实现
// ...

// 访问alloc
// void Visit() {}
// 访问load
void Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value) {
  koopa_raw_value_t src = load.src;
  // int val = Visit(value->kind.data.integer);
  // cout << "src: " << src << " value: " << val << endl;
  if (src->kind.tag == KOOPA_RVT_ALLOC) {
    // 栈分配
    rv.lw(rv.R, src);
  } else {
    rv.lw(rv.R, src);
    rv.lw(rv.R, src);
  }
  rv.R += 1;
}

// 访问store
void Visit(const koopa_raw_store_t &store) {
  koopa_raw_value_t _val = store.value, _d = store.dest;

  // 先访问数字
  int val = Visit(_val->kind.data.integer);
  int reg = rv.search(val);
  cout << "_val: " << reg << " dest: " << endl;
  rv.sw(reg, _d);
}

// 访问 return
void Visit(const koopa_raw_return_t &ret) {
  cout << "Visit return" << endl;
  if (ret.value != nullptr) {
    koopa_raw_value_t ret_value = ret.value;
    // 特判return一个整数情况
    if (ret_value->kind.tag == KOOPA_RVT_INTEGER) {
      int i = Visit(ret_value->kind.data.integer);
      rv.li(7, i);
    } else {
      rv.li(7, rv.valueStack.back());
      rv.valueStack.pop_back();
    }
  }
  rv.append("  ret");
}

// 访问 Integer
int Visit(const koopa_raw_integer_t &int_t) {
  // 如果数字不在寄存器中就添加到新的寄存器

  rv.li(rv.R, int_t.value);
  return int_t.value;
}

// 访问 binary
void Visit(const koopa_raw_binary_t &binary) {
  // 获取操作数
  // cout << "New binary op" << endl;
  koopa_raw_value_t lhs = binary.lhs, rhs = binary.rhs;

  int reg1, reg2, l = -10, r = -10, t;
  // LHS
  if (lhs->kind.tag ==
      KOOPA_RVT_INTEGER) { // 如果是从未用过的不在寄存器里的数字就要先放进寄存器
    cout << "Visit binary.lhs" << endl;
    Visit(lhs->kind.data.integer);
  }
  l = rv.valueStack.back();
  cout << "l: " << l << endl;
  reg1 = rv.search(l);
  rv.valueStack.pop_back();

  // RHS
  if (rhs->kind.tag == KOOPA_RVT_INTEGER) {
    cout << "Visit binary.rhs" << endl;
    Visit(rhs->kind.data.integer);
  }
  r = rv.valueStack.back();
  cout << "r: " << r << endl;
  reg2 = rv.search(r);
  rv.valueStack.pop_back();

  // 查找操作数所在的寄存器
  // 判断操作符
  switch (binary.op) {
  case KOOPA_RBO_NOT_EQ: // !=
    rv.tripleReg("xor", reg1, reg1, reg2);
    rv.doubleReg("snez", reg1, reg1);
    t = ((l ^ r) != 0) ? 1 : 0;

    break;

  case KOOPA_RBO_EQ: // ==
    rv.tripleReg("xor", reg1, reg1, reg2);
    rv.doubleReg("seqz", reg1, reg1);
    t = ((l ^ r) == 0) ? 1 : 0;

    break;

  case KOOPA_RBO_GE: // >=
    rv.tripleReg("sgt", reg1, reg1, reg2);
    rv.doubleReg("seqz", reg1, reg1);
    t = (l >= r) ? 1 : 0;

    break;

  case KOOPA_RBO_LE: // <=
    rv.tripleReg("slt", reg1, reg1, reg2);
    rv.doubleReg("seqz", reg1, reg1);
    t = (l <= r) ? 1 : 0;

    break;

  case KOOPA_RBO_GT: // >
    rv.tripleReg("sgt", reg1, reg1, reg2);
    t = (l > r) ? 1 : 0;

    break;

  case KOOPA_RBO_LT: // <
    rv.tripleReg("slt", reg1, reg1, reg2);
    t = (l < r) ? 1 : 0;

    break;

  case KOOPA_RBO_ADD: // +
    rv.tripleReg("add", reg1, reg1, reg2);
    t = l + r;

    break;

  case KOOPA_RBO_SUB: // -
    rv.tripleReg("sub", reg1, reg1, reg2);
    t = l - r;

    break;

  case KOOPA_RBO_MUL: // *
    rv.tripleReg("mul", reg1, reg1, reg2);
    t = l * r;

    break;

  case KOOPA_RBO_DIV: // /
    rv.tripleReg("div", reg1, reg1, reg2);
    t = l / r;

    break;

  case KOOPA_RBO_MOD: // %
    rv.tripleReg("rem", reg1, reg1, reg2);
    t = l % r;

    break;

  case KOOPA_RBO_AND: // &
    rv.tripleReg("and", reg1, reg1, reg2);
    t = l & r;

    break;

  case KOOPA_RBO_OR: // |
    rv.tripleReg("or", reg1, reg1, reg2);
    t = l | r;

    break;

  case KOOPA_RBO_XOR:
    rv.tripleReg("xor", reg1, reg1, reg2);
    t = l ^ r;

    break;

  case KOOPA_RBO_SHL:
    rv.tripleReg("sll", reg1, reg1, reg2);
    t = l << r;

    break;

  case KOOPA_RBO_SHR:
    rv.tripleReg("srl", reg1, reg1, reg2);
    t = l >> r;

    break;

  case KOOPA_RBO_SAR:
    rv.tripleReg("sra", reg1, reg1, reg2);
    t = l >> r;

    break;
  }
  rv.REG[reg1] = t;
  rv.valueStack.push_back(t);
  cout << "T: " << t << endl;
  cout << "valueStack AFTER: ";
  for (int i = 0; i < rv.valueStack.size(); i++)
    cout << rv.valueStack[i] << " ";
  cout << endl << endl;
}