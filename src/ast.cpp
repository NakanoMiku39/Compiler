#include "ast.hpp"
#include <assert.h>
#include <cstddef>
#include <string>
#include <vector>

koopaIR ir;

void CompUnitAST::Dump() const {
  cout << "CompUnit called" << endl;
  ir.output.push_back("decl @getint(): i32\n");
  ir.output.push_back("decl @getch(): i32\n");
  ir.output.push_back("decl @getarray(*i32): i32\n");
  ir.output.push_back("decl @putint(i32)\n");
  ir.output.push_back("decl @putch(i32)\n");
  ir.output.push_back("decl @putarray(i32, *i32)\n");
  ir.output.push_back("decl @starttime()\n");
  ir.output.push_back("decl @stoptime()\n");
  ir.output.push_back("\n");
  ir.decl("getint", "int");
  ir.decl("getch", "int");
  ir.decl("getarray", "int");
  ir.decl("putint", "void");
  ir.decl("putch", "void");
  ir.decl("putarray", "void");
  ir.decl("starttime", "void");
  ir.decl("stoptime", "void");

  ir.symbolTableManager.push_back(ir._st);
  for (auto &compunititem : *compunitnode) {
    compunititem->Dump();
  }
#ifdef DEBUG
  for (int i = 0; i < ir.symbolTableManager.back().size(); i++)
    cout << ir.symbolTableManager.back()[i].name << " "
         << ir.symbolTableManager.back()[i].inner.val << endl;
  ir.symbolTableManager.pop_back();
#endif
}

void CompUnitItemAST::Dump() const {
  cout << "CompUnitItem called" << endl;
  if (tag == FUNCDEF) {
    funcdef->Dump();
#ifdef DEBUG
    cout << endl << "Current global table" << endl;
    for (auto i = ir.globalTable.begin(); i != ir.globalTable.end(); i++) {
      cout << "Name: " << i->name << endl << "Type: " << i->type << endl;
    }
    cout << endl;
#endif
  } else if (tag == DECL) {
    decl->Dump();
  }
}
void FuncDefAST::Dump() const {
  cout << "FuncDef called" << endl;
  string s;

  // 因为是一个函数，所以要添加到全局变量表中
  ir.globalTable.push_back({ident, btype->btype});
  // 因为是个新函数，reg_len需要清零
  ir.reg_len = 0;

  // 对形式参数的一些处理
  if (tag == FUNCFPARAMS) { // 形式参数
                            // 如果有形参列表就加个符号表
    ir.symbolTableManager.push_back(ir._st);

    funcfparams->Dump();

    s = "fun @" + ident + "(";
    for (int i = 0; i < ir.symbolTableManager.back().size(); i++)
      if (i == 0)
        s += "@" + ir.symbolTableManager.back()[i].name + ": i32";
      else
        s += ", @" + ir.symbolTableManager.back()[i].name + ": i32";
    s += ")";
  } else {
    s += "fun @" + ident + "()";
  }

  // 注意函数的类型为void时不要输出两个空格
  if (btype->btype == "void")
    s += " {\n";
  else if (btype->btype == "int")
    s += ": i32 {\n";

  ir.output.push_back(s);

  // 函数类型
  btype->Dump();

  ir.output.push_back("%entry:\n");

  // 如果有参数列表，则需要先alloc
  if (tag == FUNCFPARAMS) {
    for (int i = 0; i < ir.symbolTableManager.back().size(); i++) {
      string ident = ir.symbolTableManager.back()[i].name;
      ir.alloc(ir.symbolTableManager.back()[i]);
      ir.store(ident);
    }
  }

  // 处理函数体
  block->Dump();

  // 如果有形参列表就去掉形参列表的符号表
  if (tag == FUNCFPARAMS)
    ir.symbolTableManager.pop_back();

  // 如果最后一句不是返回指令则添加返回
  if (ir.output.back().substr(0, 5) != "  ret")
    ir.output.push_back("  ret\n");

  ir.output.push_back("}\n\n");
}

void FuncTypeAST::Dump() const { cout << "FuncType called" << endl; }

void FuncFParamsAST::Dump() const {
  cout << "FuncFParams called" << endl;
  int len = funcfparamnode.size();
  for (int i = 0; i < len; i++) {
    funcfparamnode[i]->Dump();
  }
}

void FuncFParamAST::Dump() const {
  cout << "FuncParam called" << endl;
  ir.new_symbol(ident, btype->btype, NOT_CONST, IS_FPARAM, UNINITIALIZED);
}

void FuncRParamsAST::Dump() const {
  cout << "FuncRParams called" << endl;
  int n = exp.size();
  for (int i = 0; i < n; ++i) {
    exp[i]->Dump();
  }
}

void BlockAST::Dump() const {
  cout << "Block called" << endl;
  // 进入了新的作用域就要新加一个符号表
  ir.symbolTableManager.push_back(ir._st);

  int n = blockitemnode.size();
  for (int i = 0; i < n; ++i) {
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
  if (ir.output.back().substr(0, 6) == "  jump" ||
      ir.output.back().substr(0, 5) == "  ret")
    return;
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
    if (ir.valueStack.empty()) {
      ir.output.push_back("  ret %" + to_string(ir.reg_len - 1) + "\n");
    } else {
      instack _ret = ir.get_instack();
      ir.ret(_ret);
    }
  } else if (tag == RETURN) { // 空返回
    ir.output.push_back("  ret \n");
    cout << "Returning ";
  } else if (tag == IF) { // If
    ir.labelManager.max += 1;
    ir.labelManager.if_stack.push_back(ir.labelManager.max); // 记录当前if的嵌套
    cout << "IF" << endl;
    exp->Dump(); // If 语句的condition

    ir.br(ir.get_instack(), "%then", "%end", 0);

    // If true，then块
    ir.output.push_back(ir.blockLabel("%then", 0) + ":\n");
    if (stmt1->tag != BLOCK)
      ir.symbolTableManager.push_back(ir._st);
    stmt1->Dump(); // 条件为true时运行
    if (stmt1->tag != BLOCK)
      ir.symbolTableManager.pop_back();
    if (ir.output.back().substr(0, 5) != "  ret" &&
        ir.output.back().substr(0, 6) !=
            "  jump") { // 如果没有直接返回则需要跳转到end
      ir.output.push_back("  jump " + ir.blockLabel("%end", 0) + "\n");
    }

    // 当前end块
    ir.output.push_back(ir.blockLabel("%end", 0) + ":\n");

    // if结束的一些处理
    ir.labelManager.if_stack.pop_back();
    ir.labelManager.max += 1; // 下一次if开始的号
  } else if (tag == IFELSE) { // If-else
    ir.labelManager.max += 1;
    ir.labelManager.if_stack.push_back(ir.labelManager.max);
    cout << "IF ELSE" << endl;
    exp->Dump(); // If 语句的condition

    // 获取condition
    ir.br(ir.get_instack(), "%then", "%else", 0);

    // If true，then块
    ir.output.push_back(ir.blockLabel("%then", 0) + ":\n");
    if (stmt1->tag != BLOCK)
      ir.symbolTableManager.push_back(ir._st);
    stmt1->Dump(); // 条件为true时运行
    if (stmt1->tag != BLOCK)
      ir.symbolTableManager.pop_back();
    if (ir.output.back().substr(0, 5) != "  ret" &&
        ir.output.back().substr(0, 6) !=
            "  jump") { // 如果没有直接返回则需要跳转到end
      ir.output.push_back("  jump " + ir.blockLabel("%end", 0) + "\n");
    }

    // If false，else块
    ir.output.push_back(ir.blockLabel("%else", 0) + ":\n");
    if (stmt2->tag != BLOCK)
      ir.symbolTableManager.push_back(ir._st);
    stmt2->Dump(); // 条件为false时运行
    if (stmt2->tag != BLOCK)
      ir.symbolTableManager.pop_back();
    if (ir.output.back().substr(0, 5) != "  ret" &&
        ir.output.back().substr(0, 6) !=
            "  jump") { // 如果没有直接返回则需要跳转到end
      ir.output.push_back("  jump " + ir.blockLabel("%end", 0) + "\n");
    }

    ir.output.push_back(ir.blockLabel("%end", 0) + ":\n"); // 当前end块

    // if结束的一些处理
    ir.labelManager.if_stack.pop_back();
    ir.labelManager.max += 1; // 下一次if-else开始的号
  } else if (tag == WHILE) {  // while
    ir.labelManager.max += 1;
    ir.labelManager.while_stack.push_back(ir.labelManager.max);
    cout << "WHILE" << endl;

    // while entry
    ir.output.push_back("  jump " + ir.blockLabel("%while_entry", 1) + "\n");
    ir.output.push_back(ir.blockLabel("%while_entry", 1) + ":\n");
    exp->Dump(); // While 语句的condition

    // 判断条件
    ir.br(ir.get_instack(), "%while_body", "%end", 1);
    ir.output.push_back(ir.blockLabel("%while_body", 1) + ":\n");

    // while body
    if (stmt1->tag == BREAK || stmt1->tag == CONTINUE) {
      stmt1->Dump();
    } else {
      if (stmt1->tag != BLOCK)
        ir.symbolTableManager.push_back(ir._st);
      stmt1->Dump();
      if (stmt1->tag != BLOCK)
        ir.symbolTableManager.pop_back();
      if (ir.output.back().substr(0, 5) != "  ret" &&
          ir.output.back().substr(0, 6) != "  jump") {
        ir.output.push_back("  jump " + ir.blockLabel("%while_entry", 1) +
                            "\n"); // 循环体结束后要回到while_entry
      }
    }

    // end块
    ir.output.push_back(ir.blockLabel("%end", 1) + ":\n");

    // while结束的一些处理
    ir.labelManager.while_stack.pop_back();
    ir.labelManager.max += 1;
  } else if (tag == BREAK) {
    cout << "Break called" << endl;
    ir.output.push_back("  jump " + ir.blockLabel("%end", 1) + "\n");
  } else if (tag == CONTINUE) {
    cout << "Continue called" << endl;
    ir.output.push_back("  jump " + ir.blockLabel("%while_entry", 1) + "\n");
  }
}

void PrimaryExpAST::Dump() const {
  cout << "PrimaryExp called" << endl;
  if (tag == EXP)
    exp->Dump();
  else if (tag == LVAL)
    lval->Dump();
  else if (tag == NUMBER)
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
#ifdef DEPRECATED
  if (op == "add") {
    t.val = t2.val + t1.val;
  } else {
    t.val = t2.val - t1.val;
  }
#endif
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

  instack t, t1, t2;
  t1 = ir.get_instack();
  t2 = ir.get_instack();

  ir.ins(op, t2, t1);
  t.reg = ir.reg_len - 1;

#ifdef DEPRECATED
  // 计算并存储结果
  if (op == "mul") {
    t.val = t2.val * t1.val;
  } else if (op == "div") {
    t.val = t2.val / t1.val;
  } else if (op == "mod") {
    t.val = t2.val % t1.val;
  }
#endif
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

#ifdef DEPRECATED
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
#endif
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

#ifdef DEPRECATED

  if (op == "eq") {
    t.val = (t1.val == t2.val) ? 1 : 0;
  } else if (op == "ne") {
    t.val = (t1.val != t2.val) ? 1 : 0;
  }
#endif
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
#ifdef DEPRECATED
  _t2.val = (_t2.val != 0) ? 1 : 0;
#endif
  _t2.reg = ir.reg_len - 1;
  ir.valueStack.push_back(_t2);
  ir.ins("ne", _t1, {0, -1});
#ifdef DEPRECATED
  _t1.val = (_t1.val != 0) ? 1 : 0;
#endif
  _t1.reg = ir.reg_len - 1;
  ir.valueStack.push_back(_t1);

  t1 = ir.get_instack();
  t2 = ir.get_instack();

  ir.ins("and", t2, t1);
  t.reg = ir.reg_len - 1;

// 再进行与操作
#ifdef DEPRECATED

  t.val = t1.val && t2.val;
#endif
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
#ifdef DEPRECATED
  _t2.val = (_t2.val != 0) ? 1 : 0;
#endif
  _t2.reg = ir.reg_len - 1;

  ir.valueStack.push_back(_t2);
  ir.ins("ne", _t1, {0, -1});
#ifdef DEPRECATED
  _t1.val = (_t1.val != 0) ? 1 : 0;
#endif
  _t1.reg = ir.reg_len - 1;
  ir.valueStack.push_back(_t1);

  t1 = ir.get_instack();
  t2 = ir.get_instack();

  ir.ins("or", t2, t1);
  t.reg = ir.reg_len - 1;

#ifdef DEPRECATED
  // 再进行或操作
  t.val = t1.val || t2.val;
#endif
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
  else if (tag == UNARYEXP) {
    // 一元运算需要先输出数字再弄运算
    unaryexp->Dump();
    unaryop->Dump();
  } else if (tag == NOFUNCRPARAMS) {
    ir.call(ident, nullptr);
  } else if (tag == FUNCRPARAMS) {
    funcrparams->Dump();
    ir.call(ident, &funcrparams->exp);
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
#ifdef DEPRECATED
    t.val = -t.val;

#endif
    t.reg = ir.reg_len - 1;
    ir.valueStack.push_back(t);

    break;

  case '!':
    // 进行计算并存储
    ir.ins("eq", t, zero);
#ifdef DEPRECATED
    t.val = !t.val;
#endif
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
    string name = ir.symbolLabel(constdefnode[i]->ident);
    ir.new_symbol(name, type, IS_CONST, NOT_FPARAM, INITIALIZED);
    constdefnode[i]->Dump();
  }
}

void ConstDefAST::Dump() const {
  cout << "ConstDef called" << endl;
  constinitval->Dump();
}

void ConstInitValAST::Dump() const {
  cout << "ConstInitVal called" << endl;

  constexp->Dump();
  ir.symbolTableManager.back().back().inner = ir.get_instack();
  ir.alloc(ir.symbolTableManager.back().back());
  if (!ir.symbolTableManager.back().back()._is_global) // 全局变量不能store
    ir.store(ir.symbolTableManager.back().back());
}

void VarDeclAST::Dump() const {
  cout << "VarDecl called" << endl;
  btype->Dump();
  string type = btype->ret_btype();

  int n = vardefnode.size();
  for (int i = 0; i < n; ++i) {

    // 把变量放进符号表
    string name = ir.symbolLabel(vardefnode[i]->ident);
    if (vardefnode[i]->tag == VarDefAST::INITVAL)
      ir.new_symbol(name, type, NOT_CONST, NOT_FPARAM, INITIALIZED);
    else
      ir.new_symbol(name, type, NOT_CONST, NOT_FPARAM, UNINITIALIZED);
    vardefnode[i]->Dump();
  }
}

void VarDefAST::Dump() const {
  cout << "VarDef called" << endl;
  if (tag == INITVAL) {
    initval->Dump();
  } else {
    ir.alloc(ir.symbolTableManager.back().back());
  }
}

void InitValAST::Dump() const {
  cout << "InitVal called" << endl;
  exp->Dump();
  // 然后把变量值存到地址上
  auto t = ir.symbolTableManager.back().back();
  t.inner = ir.get_instack();
  ir.alloc(t);
  if (!t._is_global) // 全局变量不能store
    ir.store(t);
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
