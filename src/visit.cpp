// 函数声明略
// ...
#include "visit.hpp"
#include "koopa.hpp"
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
  /*
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

  for (vector<symbol>::iterator i = rv.addrStack.begin();
       i != rv.addrStack.end(); i++) {
    printf("value: %d offset: %d addr: %d\n", i->value, i->offset,
           i->addr->kind.data.integer.value);
  }

  rv.prologue(func);
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
    Visit(kind.data.binary, value);
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
    Visit(kind.data.ret, value);
    break;

  case KOOPA_RVT_STORE:
    // 访问 store 指令
    Visit(kind.data.store, value);
    break;

  default:
    // 其他类型暂时遇不到
    break;
  }
  rv.append("\n");
}

// 访问对应类型指令的函数定义略
// 视需求自行实现
// ...

// 访问alloc
void Visit() {}

// 访问load
void Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value) {
  koopa_raw_value_t src = load.src;
  if (src->kind.tag == KOOPA_RVT_ALLOC) {
    rv.lw(0, src);
  } else {
  }
  // load完后要把变量存到新的地址
  rv.sw(0, value, 1);
}

// 访问store
void Visit(const koopa_raw_store_t &store, const koopa_raw_value_t &value) {
  koopa_raw_value_t _val = store.value, _d = store.dest;

  if (_val->kind.tag == KOOPA_RVT_INTEGER) {
    int val = Visit(_val->kind.data.integer);

    rv.REG[0] = val;
    rv.li(0, val);
    cout << "_val: " << val << " _reg: " << 0 << " dest: " << endl;
  } else {
    rv.lw(0, _val);
  }
  if (_d->kind.tag == KOOPA_RVT_ALLOC) { // 如果是分配内存就要加offset
    cout << "STORE ALLOC" << endl;
    rv.sw(0, _d, 0);
  } else {
    cout << "NOT STORE ALLOC" << endl;
  }
}

// 访问 return
void Visit(const koopa_raw_return_t &ret, const koopa_raw_value_t &value) {
  cout << "Visit return" << endl;
  if (ret.value != nullptr) {
    koopa_raw_value_t ret_value = ret.value;
    // 特判return一个整数情况
    if (ret_value->kind.tag == KOOPA_RVT_INTEGER) {
      int i = Visit(ret_value->kind.data.integer);
      rv.li(7, i);
    } else {
      // rv.lw(7, ret_value);
      rv.append("  lw    a0, ");
      rv.append(rv.offset - 4);
      rv.append("(sp)\n");
    }
  }
  rv.append("  addi sp, sp, ");
  rv.append(rv.offset);
  rv.append("\n  ret");
}

// 访问 Integer
int Visit(const koopa_raw_integer_t &int_t) {
  // 如果数字不在寄存器中就添加到新的寄存器
  // rv.li(rv.R, int_t.value);
  return int_t.value;
}

// 访问 binary
void Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value) {
  // 获取操作数
  cout << "New binary op" << endl;
  koopa_raw_value_t lhs = binary.lhs, rhs = binary.rhs;

  int reg1, reg2, l, r, t;
  // LHS
  if (lhs->kind.tag ==
      KOOPA_RVT_INTEGER) { // 如果是从未用过的不在寄存器里的数字就要先放进寄存器
    cout << "Visit binary.lhs" << endl;
    l = Visit(lhs->kind.data.integer);
    rv.REG[0] = l;
    rv.li(0, l);
  } else { // 否则要先从地址中取出
    rv.lw(0, lhs);
    vector<symbol>::iterator i = rv.search(lhs);
    l = i->value;
  }
  reg1 = 0;
  cout << "l: " << l << endl;

  // RHS
  if (rhs->kind.tag == KOOPA_RVT_INTEGER) {
    cout << "Visit binary.rhs" << endl;
    r = Visit(rhs->kind.data.integer);
    rv.REG[1] = r;
    rv.li(1, r);
  } else {
    rv.lw(1, rhs);
    vector<symbol>::iterator i = rv.search(rhs);
    r = i->value;
  }
  reg2 = 1;
  cout << "r: " << r << endl;

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
    rv.tripleReg("slt", reg1, reg1, reg2);
    rv.doubleReg("seqz", reg1, reg1);
    t = (l >= r) ? 1 : 0;

    break;

  case KOOPA_RBO_LE: // <=
    rv.tripleReg("sgt", reg1, reg1, reg2);
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
  cout << "T: " << t << endl;
  rv.sw(reg1, value, 1);
}