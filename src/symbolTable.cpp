#include "symbolTable.hpp"
#include <string>
#include <vector>

vector<variable>::iterator koopaIR::search(string _ident) {
  // 遍历符号表栈
  for (auto i = symbolTableManager.rbegin(); i != symbolTableManager.rend();
       i++) {
    // 从当前符号表中查找符号
    for (auto j = i->begin(); j != i->end(); j++) {
      if (j->name.find(_ident) == 0) {
        return j;
      }
    }
  }
  cout << "Failed to find variable through ident" << endl;
  assert(false);
}

vector<variable>::iterator koopaIR::search(int reg) {
  for (auto i = symbolTableManager.rbegin(); i != symbolTableManager.rend();
       i++) {
    for (auto j = i->begin(); j != i->end(); j++) {
      if (j->inner.reg == reg) {
        return j;
      }
    }
  }
  cout << "Failed to find variable through reg" << endl;
  assert(false);
}

vector<variable>::iterator koopaIR::search_global(string _ident) {
  for (auto i = globalTable.begin(); i != globalTable.end(); i++) {
    if (i->name == _ident) {
      return i;
    }
  }
  cout << "Failed to find global variable through ident" << endl;
  assert(false);
}

void koopaIR::new_symbol(
    string ident, string type, bool _is_const,
    bool _is_fparam) { // 向符号表加入一个新变量，但是不包括初始值
  struct variable t;
  t.name = ident;
  t.type = type;
  t.inner.reg = -1;
  t._is_const = _is_const;
  t._is_fparam = _is_fparam;
  symbolTableManager.back().push_back(t);
}

string koopaIR::symbolLabel(string label) { // 为新变量分配地址名
  var_num += 1;
  return label += "_" + to_string(var_num);
}

string koopaIR::blockLabel(string label, int token) {
  // token = 0 代表是if-else块的标签
  // token = 1 代表是while块的标签
  if (token == 0)
    return label += "_" + to_string(labelManager.if_stack.back());
  else
    return label += "_" + to_string(labelManager.while_stack.back());
  ;
}

instack koopaIR::get_instack() { // 返回valueStack顶部的元素并pop
  assert(!valueStack.empty());   // 不能在空的时候弹出
  instack t = valueStack.back();
  valueStack.pop_back();
  return t;
}

// 指令操作
void koopaIR::ins(string cmd, instack _1, instack _2) {
  string reg1, reg2, s;
  reg1 = _1.reg == -1 ? to_string(_1.val) : "%" + to_string(_1.reg);
  reg2 = _2.reg == -1 ? to_string(_2.val) : "%" + to_string(_2.reg);
  s = "  %" + to_string(reg_len) + " = " + cmd + " " + reg1 + ", " + reg2 +
      "\n";
  output.push_back(s);
  reg_len += 1;
}

void koopaIR::ret(instack _ret) {
  string s = "  ret ";
  if (_ret.reg == -1) {
    cout << "value" << endl;
    s += to_string(_ret.val) + "\n";
  } else {
    cout << "ident" << endl;
    s += "%" + to_string(_ret.reg) + "\n";
  }
  cout << _ret.val << endl;
  // assert(valueStack.empty());
  output.push_back(s);
}

// 从地址加载到寄存器
void koopaIR::load(vector<variable>::iterator var) {
  string s;
  if (var->_is_fparam == IS_FPARAM)
    s = "  %" + to_string(reg_len) + " = load %" + var->name + "\n";
  else
    s = "  %" + to_string(reg_len) + " = load @" + var->name + "\n";
  output.push_back(s);
  var->inner.reg = reg_len;
  reg_len += 1;
}

void koopaIR::alloc(string var, bool is_fparam) {
  string s;
  if (is_fparam)
    s = "  %" + var + " = alloc i32\n";
  else
    s = "  @" + var + " = alloc i32\n";
  output.push_back(s);
}

// 把值存到地址
void koopaIR::store(variable var) {
  string s;
  if (var.inner.reg == -1)
    s = "  store " + to_string(var.inner.val) + ", @" + var.name + "\n";
  else
    s = "  store %" + to_string(var.inner.reg) + ", @" + var.name + "\n";
  output.push_back(s);
}

// 用于给形参分配地址空间的store
void koopaIR::store(string _ident) {
  string s;
  s = "  store @" + _ident + ", %" + _ident + "\n";
  output.push_back(s);
}

void koopaIR::br(instack t, string block1, string block2, int token) {
  string s;
  block1 = blockLabel(block1, token);
  block2 = blockLabel(block2, token);
  if (t.reg == -1)
    s = "  br " + to_string(t.val) + ", " + block1 + ", " + block2 + "\n";
  else
    s = "  br %" + to_string(t.reg) + ", " + block1 + ", " + block2 + "\n";
  output.push_back(s);
}

void koopaIR::call(string _ident, vector<unique_ptr<ExpAST>> *exp) {
  string s;
  // 函数是否有返回值
  auto t = search_global(_ident);
  if (t->type != "") { // 如果函数有返回值就需要分配一个寄存器
    s += "  %" + to_string(reg_len) + " = ";
    t->inner.reg = reg_len;
    reg_len += 1;
  } else {
    s += "  ";
  }

  // 函数是否需要参数
  if (exp == nullptr) { // 如果没有参数列表
    s += "call @" + _ident + "()\n";
  } else { // 如果有参数列表
    s += "call @" + _ident + "(";
    int size = exp->size();
    vector<instack> v;
    for (auto i = 0; i < size; i++) { // 参数列表的顺序是反的，要正过来
      instack t = get_instack();
      v.push_back(t);
    }

#ifdef DEBUG
    cout << endl << "The funcrparams: " << endl;
#endif
    for (auto i = 0; i < size; i++) {
#ifdef DEBUG
      cout << v.back().val << endl;
#endif
      // 如果参数列表中使用了变量则需要放入寄存器而不是立即数
      string t = v.back().reg == -1 ? to_string(v.back().val)
                                    : "%" + to_string(v.back().reg);
      if (i == 0)
        s += t;
      else
        s += "," + t;
      v.pop_back();
    }

    s += ")\n";
  }

  // 如果函数有返回值就需要给valueStack push回去
  if (t->type != "")
    valueStack.push_back({0, reg_len - 1});
  output.push_back(s);
}

void koopaIR::decl(string _ident, string type) {
  // output.push_back(_ident);
  globalTable.push_back({_ident, type});
}

const char *koopaIR::show() {
  for (vector<string>::iterator i = output.begin(); i != output.end(); i++)
    IR += *i;
  cout << IR << endl;
  return IR.c_str();
}
