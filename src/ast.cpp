#include "ast.hpp"
#include <string>
#include <vector>

koopaIR ir;

void CompUnitAST::Dump() const { func_def->Dump(); }

void FuncDefAST::Dump() const {
  cout << "FuncDef called" << endl;
  string s = "fun @" + ident + "(): " + func_type->type + " {\n";
  ir.output.push_back(s);
  ir.output.push_back("%entry:\n");

  func_type->Dump();
  block->Dump();
  if (ir.output.back().substr(0, 5) != "  ret") {
    ir.output.push_back("  ret\n");
  }
  ir.output.push_back("}");
}

void FuncTypeAST::Dump() const {
  cout << "FuncType called" << endl;
  // ir.append(type.c_str());
}

void BlockAST::Dump() const {
  cout << "Block called" << endl;
  // 进入了新的作用域就要新加一个符号表
  ir.symbolTableManager.push_back(ir._st);

  int n = blockitemnode.size();
  for (int i = 0; i < n; ++i) {
    // ir.is_ret = false;
    blockitemnode[i]->Dump();
    if (ir.symbolTableManager.size() == 1 &&
        ir.output.back().substr(0, 5) == "  ret")
      break;
  }

  // 退出了一个作用域就要去掉符号表
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
    exp->Dump();  // 获取要赋的值
    lval->Dump(); // 获取左值

    instack t1 = ir.get_instack();
    instack t2 = ir.get_instack();

    vector<variable>::iterator t = ir.search(t1.reg); // 获取变量
    t->inner = t2; // 栈顶数字（也就是要赋的值）给变量
    ir.store(*t);
    t->inner.reg = -1;
  } else if (tag == EXP) { // 表达式
    exp->Dump();
  } else if (tag == EMPTY) {

  } else if (tag == BLOCK) { // 基本块
    block->Dump();
  } else if (tag == RETURNEXP) { // 返回
    exp->Dump();
    cout << "Returning ";
    instack _ret = ir.get_instack();
    ir.ret(_ret);
    // ir.is_ret = true;
  } else if (tag == RETURN) { // 空返回
    ir.output.push_back("  ret \n");
    cout << "Returning ";
    // ir.is_ret = true;
  } else if (tag == IF) { // If
    ir.ifManager.max += 1;
    ir.ifManager.stack.push_back(ir.ifManager.max);
    cout << "IF" << endl;
    exp->Dump(); // If 语句的condition

    ir.br(ir.get_instack(), "%then", "%end");

    // If true，then块
    ir.output.push_back(ir.blockLabel("%then") + ":\n");
    if (stmt1->tag != BLOCK)
      ir.symbolTableManager.push_back(ir._st);
    stmt1->Dump(); // 条件为true时运行
    if (stmt1->tag != BLOCK)
      ir.symbolTableManager.pop_back();
    if (ir.output.back().substr(0, 5) !=
        "  ret") { // 如果没有直接返回则需要跳转到end
      ir.output.push_back("  jump " + ir.blockLabel("%end") + "\n");
    }

    // 上个end块是否jump到当前end块
    if (ir.output.back().substr(0, 4) == "%end")
      ir.output.push_back("  jump " + ir.blockLabel("%end") + "\n");

    // 当前end块
    ir.output.push_back(ir.blockLabel("%end") + ":\n");

    // if结束的一些处理
    ir.ifManager.stack.pop_back();
    ir.ifManager.max += 1;    // 下一次if开始的号
  } else if (tag == IFELSE) { // If-else
    ir.ifManager.max += 1;
    ir.ifManager.stack.push_back(ir.ifManager.max);
    cout << "IF ELSE" << endl;
    exp->Dump(); // If 语句的condition

    // 获取condition
    ir.br(ir.get_instack(), "%then", "%else");

    // If true，then块
    ir.output.push_back(ir.blockLabel("%then") + ":\n");
    if (stmt1->tag != BLOCK)
      ir.symbolTableManager.push_back(ir._st);
    stmt1->Dump(); // 条件为true时运行
    if (stmt1->tag != BLOCK)
      ir.symbolTableManager.pop_back();
    if (ir.output.back().substr(0, 5) !=
        "  ret") { // 如果没有直接返回则需要跳转到end
      ir.output.push_back("  jump " + ir.blockLabel("%end") + "\n");
    }

    // If false，else块
    ir.output.push_back(ir.blockLabel("%else") + ":\n");
    if (stmt2->tag != BLOCK)
      ir.symbolTableManager.push_back(ir._st);
    stmt2->Dump(); // 条件为false时运行
    if (stmt2->tag != BLOCK)
      ir.symbolTableManager.pop_back();
    if (ir.output.back().substr(0, 5) !=
        "  ret") { // 如果没有直接返回则需要跳转到end
      ir.output.push_back("  jump " + ir.blockLabel("%end") + "\n");
    }

    // end块
    if (ir.output.back().substr(0, 4) ==
        "%end") // 如果上一个end块没有内容就要跳转到当前end块
      ir.output.push_back("  jump " + ir.blockLabel("%end") + "\n");
    ir.output.push_back(ir.blockLabel("%end") + ":\n"); // 当前end块

    // if结束的一些处理
    ir.ifManager.stack.pop_back();
    ir.ifManager.max += 1;   // 下一次if-else开始的号
  } else if (tag == WHILE) { // while
    ir.ifManager.max += 1;
    ir.ifManager.stack.push_back(ir.ifManager.max);
    cout << "WHILE" << endl;

    // while entry
    ir.output.push_back("  jump " + ir.blockLabel("%while_entry") + "\n");
    ir.output.push_back(ir.blockLabel("%while_entry") + ":\n");
    exp->Dump(); // While 语句的condition

    // 判断条件
    ir.br(ir.get_instack(), "%while_body", "%end");
    ir.output.push_back(ir.blockLabel("%while_body") + ":\n");

    // while body
    if (stmt1->tag != BLOCK)
      ir.symbolTableManager.push_back(ir._st);
    stmt1->Dump();
    if (stmt1->tag != BLOCK)
      ir.symbolTableManager.pop_back();
    if (ir.output.back().substr(0, 5) != "  ret") {
      ir.output.push_back("  jump " + ir.blockLabel("%while_entry") + "\n");
    }

    // end块
    ir.output.push_back(ir.blockLabel("%end") + ":\n");

    // if结束的一些处理
    ir.ifManager.stack.pop_back();
    ir.ifManager.max += 1;
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
  t1 = ir.get_instack();
  t2 = ir.get_instack();

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
  t1 = ir.get_instack();
  t2 = ir.get_instack();

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
  t1 = ir.get_instack();
  t2 = ir.get_instack();

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
  t1 = ir.get_instack();
  t2 = ir.get_instack();

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
  _t1 = ir.get_instack();
  _t2 = ir.get_instack();

  // 判断0，然后逆序入栈
  ir.ins("ne", _t2, {0, -1});
  _t2.val = (_t2.val != 0) ? 1 : 0;
  _t2.reg = ir.reg_len - 1;
  ir.valueStack.push_back(_t2);
  ir.ins("ne", _t1, {0, -1});
  _t1.val = (_t1.val != 0) ? 1 : 0;
  _t1.reg = ir.reg_len - 1;
  ir.valueStack.push_back(_t1);

  t1 = ir.get_instack();
  t2 = ir.get_instack();

  ir.ins("and", t2, t1);
  t.reg = ir.reg_len - 1;

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
  _t1 = ir.get_instack();
  _t2 = ir.get_instack();

  // 先判断两个操作数是不是分别为0，同时逆序入栈
  ir.ins("ne", _t2, {0, -1});
  _t2.val = (_t2.val != 0) ? 1 : 0;
  _t2.reg = ir.reg_len - 1;
  ir.valueStack.push_back(_t2);
  ir.ins("ne", _t1, {0, -1});
  _t1.val = (_t1.val != 0) ? 1 : 0;
  _t1.reg = ir.reg_len - 1;
  ir.valueStack.push_back(_t1);

  t1 = ir.get_instack();
  t2 = ir.get_instack();

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
  instack t = ir.get_instack();

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
    // t.depth = ir.symbolTableManager.size();
    t.is_const = true;
    ir.symbolTableManager.back().push_back(t);
    constdefnode[i]->Dump();
  }
}

void ConstDefAST::Dump() const {
  cout << "ConstDef called" << endl;
  string name = ir.symbolLabel(ident);
  ir.symbolTableManager.back().back().name = name;
  ir.alloc(name);
  constinitval->Dump();
}

void ConstInitValAST::Dump() const {
  cout << "ConstInitVal called" << endl;

  constexp->Dump();
  ir.symbolTableManager.back().back().inner = ir.get_instack();
  ir.store(ir.symbolTableManager.back().back());
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
    // t.depth = ir.symbolTableManager.size();
    t.inner.reg = -1;
    t.is_const = false;
    ir.symbolTableManager.back().push_back(t);
    vardefnode[i]->Dump();
  }
}

void VarDefAST::Dump() const {
  cout << "VarDef called" << endl;
  string name = ir.symbolLabel(ident);
  // 往符号表中加入变量
  ir.symbolTableManager.back().back().name = name;
  // 分配内存
  ir.alloc(name);
  if (tag == INITVAL) {
    initval->Dump();
  }
}

void InitValAST::Dump() const {
  cout << "InitVal called" << endl;
  exp->Dump();
  // 然后把变量值存到地址上
  ir.symbolTableManager.back().back().inner = ir.get_instack();
  ;
  ir.store(ir.symbolTableManager.back().back());
}

void BTypeAST::Dump() const { cout << "BType called" << endl; }

string BTypeAST::ret_btype() { return btype; }

void LValAST::Dump() const {
  cout << "LVal called" << endl;

  vector<variable>::iterator _t = ir.search(ident);

  ir.load(_t);
  instack t = _t->inner;
  ir.valueStack.push_back(t);
}

instack LValAST::ret_value() {
  vector<variable>::iterator _t = ir.search(ident);
  instack t = _t->inner;
  cout << "Returning t: " << ir.valueStack.back().val << endl;

  return t;
}

void NumberAST::Dump() const {
  cout << "Number called " << endl;
  instack t = {number, -1};
  ir.valueStack.push_back(t);
}

instack NumberAST::ret_value() {
  instack t = {number, -1};
  return t;
}
