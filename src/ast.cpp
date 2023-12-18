#include "ast.hpp"
#include <string>
#include <vector>

koopaIR ir;

void CompUnitAST::Dump() const { func_def->Dump(); }

void FuncDefAST::Dump() const {
  cout << "FuncDef called" << endl;
  ir.append("fun @");
  ir.append(ident.c_str());
  ir.append("(): ");
  func_type->Dump();
  ir.append(" {\n");
  ir.append("%entry:\n");
  block->Dump();
  ir.append("}");
}

void FuncTypeAST::Dump() const {
  cout << "FuncType called" << endl;
  ir.append(type.c_str());
}

void BlockAST::Dump() const {
  cout << "Block called" << endl;

  vector<variable> t;
  ir.symbolTableManager.push_back(t);

  int n = blockitemnode.size();
  for (int i = 0; i < n; ++i) {
    blockitemnode[i]->Dump();
  }
  ir.symbolTableManager.pop_back();
}

void BlockItemAST::Dump() const {
  cout << "BlockItem called" << endl;
  if (tag == DECL)
    decl->Dump();
  else if (tag == STMT)
    stmt->Dump();
}

void StmtAST::Dump() const {
  cout << "Stmt called" << endl;
  if (tag == LVAL) {
    // 赋值操作
    lval->Dump(); // 获取左值

    exp->Dump(); // 获取要赋的值

    instack t1 = ir.valueStack.back();
    ir.valueStack.pop_back();
    instack t2 = ir.valueStack.back();
    ir.valueStack.pop_back();

    vector<variable>::iterator t = ir.search(t2.reg); // 获取变量
    t->inner = t1; // 栈顶数字（也就是要赋的值）给变量
    ir.store(*t);
    t->inner.reg = -1;
  } else if (tag == EXP) {
    exp->Dump();
  } else if (tag == EMPTY) {

  } else if (tag == BLOCK) {
    block->Dump();
  } else if (tag == RETURNEXP) {
    exp->Dump();
    ir.append("  ret ");
    cout << "Returning ";
    instack _ret = ir.valueStack.back();
    if (_ret.reg == -1) {
      cout << "value" << endl;
      ir.append(_ret.val);
    } else {
      cout << "ident" << endl;
      ir.append("%");
      ir.append(_ret.reg);
    }
    cout << _ret.val << endl;
    ir.append("\n");
  } else if (tag == RETURN) {
    ir.append("  ret ");
    cout << "Returning ";
  }
}

void PrimaryExpAST::Dump() const {
  cout << "PrimaryExp called" << endl;
  if (tag == EXP)
    exp->Dump();
  else if (tag == LVAL)
    lval->Dump();
  else
    number->Dump();
}

instack PrimaryExpAST::ret_value() {
  switch (tag) {
  case EXP:
    return exp->ret_value();
  case LVAL:
    return lval->ret_value();
  case NUMBER:
    return number->ret_value();
  }
}

void ConstExpAST::Dump() const {
  cout << "ConstExp called" << endl;
  exp->Dump();
}

void ExpAST::Dump() const {
  cout << "Exp called" << endl;
  lorexp->Dump();
}

instack ExpAST::ret_value() { return lorexp->ret_value(); }

void AddExpAST::Dump() const {
  cout << "Add called" << endl;
  if (tag == MULEXP) {
    mulexp->Dump();
    return;
  }
  addexp->Dump();
  mulexp->Dump();

  // 计算并存储结果，结果保存到寄存器和栈中
  // 从栈中提取操作数
  instack t, t1, t2;
  t1 = ir.valueStack.back();
  ir.valueStack.pop_back();
  t2 = ir.valueStack.back();
  ir.valueStack.pop_back();

  // 输出指令
  ir.ins(op, t2, t1);
  t.reg = ir.reg_len - 1;
  if (op == "add") {
    t.val = t2.val + t1.val;
  } else {
    t.val = t2.val - t1.val;
  }

  cout << t2.val << " and " << t1.val << " Add result: " << t.val << endl;
  ir.valueStack.push_back(t);
}

instack AddExpAST::ret_value() {
  switch (tag) {
  case MULEXP:
    return mulexp->ret_value();
  case ADDEXP:
    return addexp->ret_value();
  }
}

void MulExpAST::Dump() const {
  cout << "Mul called" << endl;
  if (tag == UNARYEXP) {
    unaryexp->Dump();
    return;
  }
  mulexp->Dump();
  unaryexp->Dump();

  // mulexp->ret_value();
  // unaryexp->ret_value();

  instack t, t1, t2;
  t1 = ir.valueStack.back();
  ir.valueStack.pop_back();
  t2 = ir.valueStack.back();
  ir.valueStack.pop_back();

  ir.ins(op, t2, t1);
  t.reg = ir.reg_len - 1;

  // 计算并存储结果
  if (op == "mul") {
    t.val = t2.val * t1.val;
  } else if (op == "div") {
    t.val = t2.val / t1.val;
  } else if (op == "mod") {
    t.val = t2.val % t1.val;
  }
  ir.valueStack.push_back(t);
}

instack MulExpAST::ret_value() {
  switch (tag) {
  case UNARYEXP:
    return unaryexp->ret_value();
  case MULEXP:
    return mulexp->ret_value();
  }
}

void RelExpAST::Dump() const {
  cout << "RelExp called" << endl;
  if (tag == ADDEXP) {
    addexp->Dump();
    return;
  }

  relexp->Dump();
  addexp->Dump();

  instack t, t1, t2;
  t1 = ir.valueStack.back();
  ir.valueStack.pop_back();
  t2 = ir.valueStack.back();
  ir.valueStack.pop_back();

  ir.ins(op, t2, t1);
  t.reg = ir.reg_len - 1;

  // 计算并存储结果
  if (op == "lt") {
    t.val = (t2.val < t1.val) ? 1 : 0;
  } else if (op == "gt") {
    t.val = (t2.val > t1.val) ? 1 : 0;
  } else if (op == "le") {
    t.val = (t2.val <= t1.val) ? 1 : 0;
  } else if (op == "ge") {
    t.val = (t2.val >= t1.val) ? 1 : 0;
  }
  ir.valueStack.push_back(t);
}

instack RelExpAST::ret_value() {
  switch (tag) {
  case ADDEXP:
    return addexp->ret_value();
  case RELEXP:
    return relexp->ret_value();
  }
}

void EqExpAST::Dump() const {
  cout << "EqExp called" << endl;
  if (tag == RELEXP) {
    relexp->Dump();
    return;
  }

  eqexp->Dump();
  relexp->Dump();

  instack t, t1, t2;
  t1 = ir.valueStack.back();
  ir.valueStack.pop_back();
  t2 = ir.valueStack.back();
  ir.valueStack.pop_back();

  ir.ins(op, t2, t1);
  t.reg = ir.reg_len - 1;
  if (op == "eq") {
    t.val = (t1.val == t2.val) ? 1 : 0;
  } else if (op == "ne") {
    t.val = (t1.val != t2.val) ? 1 : 0;
  }
  ir.valueStack.push_back(t);
}

instack EqExpAST::ret_value() {
  switch (tag) {
  case RELEXP:
    return relexp->ret_value();
  case EQEXP:
    return eqexp->ret_value();
  }
}

void LAndExpAST::Dump() const {
  cout << "LAndExp called" << endl;
  if (tag == EQEXP) {
    eqexp->Dump();
    return;
  }

  landexp->Dump();
  eqexp->Dump();

  instack t, t1, t2, _t1, _t2;
  _t1 = ir.valueStack.back();
  ir.valueStack.pop_back();
  _t2 = ir.valueStack.back();
  ir.valueStack.pop_back();

  // 判断0，然后逆序入栈
  ir.ins("ne", _t2, {0, -1});
  _t2.val = (_t2.val != 0) ? 1 : 0;
  _t2.reg = ir.reg_len - 1;
  ir.valueStack.push_back(_t2);
  ir.ins("ne", _t1, {0, -1});
  _t1.val = (_t1.val != 0) ? 1 : 0;
  _t1.reg = ir.reg_len - 1;
  ir.valueStack.push_back(_t1);

  t1 = ir.valueStack.back();
  ir.valueStack.pop_back();
  t2 = ir.valueStack.back();
  ir.valueStack.pop_back();

  ir.ins("and", t2, t1);
  t.reg = ir.reg_len - 1;

  // 首先要判断操作数是不是0
  // t1.val = (t1.val != 0) ? 1 : 0;
  // t2.val = (t2.val != 0) ? 1 : 0;

  // 再进行与操作
  t.val = t1.val && t2.val;
  ir.valueStack.push_back(t);
}

instack LAndExpAST::ret_value() {
  switch (tag) {
  case EQEXP:
    return eqexp->ret_value();
  case LANDEXP:
    return landexp->ret_value();
  }
}

void LOrExpAST::Dump() const {
  cout << "LOrExp called" << endl;
  if (tag == LANDEXP) {
    landexp->Dump();
    return;
  }

  lorexp->Dump();
  landexp->Dump();

  instack t, t1, t2, _t1, _t2;
  _t1 = ir.valueStack.back();
  ir.valueStack.pop_back();
  _t2 = ir.valueStack.back();
  ir.valueStack.pop_back();

  // 先判断两个操作数是不是分别为0，同时逆序入栈
  ir.ins("ne", _t2, {0, -1});
  _t2.val = (_t2.val != 0) ? 1 : 0;
  _t2.reg = ir.reg_len - 1;
  ir.valueStack.push_back(_t2);
  ir.ins("ne", _t1, {0, -1});
  _t1.val = (_t1.val != 0) ? 1 : 0;
  _t1.reg = ir.reg_len - 1;
  ir.valueStack.push_back(_t1);

  t1 = ir.valueStack.back();
  ir.valueStack.pop_back();
  t2 = ir.valueStack.back();
  ir.valueStack.pop_back();

  ir.ins("or", t2, t1);
  t.reg = ir.reg_len - 1;

  // 首先要判断操作数是不是0
  // t1.val = (t1.val != 0) ? 1 : 0;
  // t2.val = (t2.val != 0) ? 1 : 0;

  // 再进行或操作
  t.val = t1.val || t2.val;
  ir.valueStack.push_back(t);
}

instack LOrExpAST::ret_value() {
  switch (tag) {
  case LANDEXP:
    return landexp->ret_value();
  case LOREXP:
    return lorexp->ret_value();
  }
}

void UnaryExpAST::Dump() const {
  cout << "UnaryExp called" << endl;
  if (tag == PRIMARYEXP)
    primaryexp->Dump();
  else {
    // 一元运算需要先输出数字再弄运算
    unaryexp->Dump();
    unaryop->Dump();
  }
}

instack UnaryExpAST::ret_value() {
  switch (tag) {
  case PRIMARYEXP:
    return primaryexp->ret_value();
  case UNARYEXP:
    return unaryexp->ret_value();
  }
}

void UnaryOpAST::Dump() const {
  cout << "UnaryOp called" << endl;
  instack zero;
  zero.val = 0;
  zero.reg = -1;
  instack t = ir.valueStack.back();
  ir.valueStack.pop_back();

  switch (op) {
  case '+':
    ir.valueStack.push_back(t);
    break;

  case '-':
    // 进行计算并存储
    ir.ins("sub", zero, t);
    t.val = -t.val;
    t.reg = ir.reg_len - 1;
    ir.valueStack.push_back(t);

    break;

  case '!':
    // 进行计算并存储
    ir.ins("eq", t, zero);
    t.val = !t.val;
    t.reg = ir.reg_len - 1;
    ir.valueStack.push_back(t);

    break;
  }
}

void DeclAST::Dump() const {
  cout << "Decl called" << endl;
  if (constdecl != nullptr)
    constdecl->Dump();
  else
    vardecl->Dump();
}

void ConstDeclAST::Dump() const {
  cout << "ConstDecl called" << endl;
  btype->Dump();
  string type = btype->ret_btype();

  int n = constdefnode.size();
  for (int i = 0; i < n; ++i) {

    // 把常量放进符号表
    struct variable t;
    t.type = type;
    t.inner.reg = -1;
    // t.addr = "@" + to_string(ir.addr_len + 1);
    t.is_const = true;
    ir.symbolTableManager.back().push_back(t);
    constdefnode[i]->Dump();
  }
}

void ConstDefAST::Dump() const {
  cout << "ConstDef called" << endl;
  string _ident = ident + "_" + to_string(ir.symbolTableManager.size());
  ir.symbolTableManager.back().back().name = _ident;
  ir.alloc(_ident);
  constinitval->Dump();
}

void ConstInitValAST::Dump() const {
  cout << "ConstInitVal called" << endl;

  constexp->Dump();
  ir.symbolTableManager.back().back().inner = ir.valueStack.back();
  ir.store(ir.symbolTableManager.back().back());
  ir.valueStack.pop_back();
}

void VarDeclAST::Dump() const {
  cout << "VarDecl called" << endl;
  btype->Dump();
  string type = btype->ret_btype();

  int n = vardefnode.size();
  for (int i = 0; i < n; ++i) {

    // 把变量放进符号表
    struct variable t;
    t.type = type;
    t.inner.reg = -1;
    // t.addr = "@" + to_string(ir.addr_len + 1);
    t.is_const = false;
    ir.symbolTableManager.back().push_back(t);
    vardefnode[i]->Dump();
  }
}

void VarDefAST::Dump() const {
  cout << "VarDef called" << endl;
  // 往符号表中加入变量
  string _ident = ident + "_" + to_string(ir.symbolTableManager.size());
  ir.symbolTableManager.back().back().name = _ident;
  // 分配内存
  ir.alloc(_ident);
  if (tag == INITVAL) {
    initval->Dump();
  }
}

void InitValAST::Dump() const {
  cout << "InitVal called" << endl;
  exp->Dump();
  // 然后把变量值存到地址上
  ir.symbolTableManager.back().back().inner = ir.valueStack.back();
  ir.store(ir.symbolTableManager.back().back());
  ir.valueStack.pop_back();
}

void BTypeAST::Dump() const { cout << "BType called" << endl; }

string BTypeAST::ret_btype() { return btype; }

void LValAST::Dump() const {
  cout << "LVal called" << endl;

  vector<variable>::iterator _t = ir.search(ident);

  if (_t->inner.reg == -1) { // 如果变量不在寄存器里就要先从地址中加载出来
    ir.load(_t);
  }

  instack t;
  t.val = _t->inner.val;
  t.reg = _t->inner.reg;
  ir.valueStack.push_back(t);
}

instack LValAST::ret_value() {
  // string _ident = ident + "_" + to_string(ir.symbolTableManager.size());
  vector<variable>::iterator _t = ir.search(ident);
  instack t;
  t.val = _t->inner.val;
  t.reg = _t->inner.reg;
  cout << "Returning t: " << ir.valueStack.back().val << endl;

  return t;
}

void NumberAST::Dump() const {
  cout << "Number called " << endl;
  instack t;
  t.val = number;
  t.reg = -1;
  ir.valueStack.push_back(t);
}

instack NumberAST::ret_value() {
  instack t;
  t.reg = -1;
  t.val = number;
  return t;
}
