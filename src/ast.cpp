#include "ast.hpp"


koopaIR ir;

void CompUnitAST::Dump() const  {
    func_def->Dump();
}

void FuncDefAST::Dump() const {
    // fprintf(yyout, "fun @%s(): ", ident.c_str());
    ir.append("fun @");
    ir.append(ident.c_str());
    ir.append("(): ");
    func_type->Dump();
    block->Dump();
  }

  void FuncTypeAST::Dump() const {
    // fprintf(yyout, "%s {\n", type.c_str());
    ir.append(type.c_str());
    ir.append(" {\n");
  }

void BlockAST::Dump() const {
    //fprintf(yyout, "\%s:\n  ", entry.c_str());
    ir.append(entry.c_str());
    ir.append(":\n  ");
    stmt->Dump();
}

void StmtAST::Dump() const {
    // fprintf(yyout,"%s ", ret.c_str());    
    ir.append(ret.c_str());
    ir.append(" ");
    number->Dump();
    // fprintf(yyout, "}");
    ir.append("}");
  }

void NumberAST::Dump() const {
    // fprintf(yyout, "%d\n", number) ;
    ir.append(number);
    ir.append("\n");
  }