/*********************                                                        */
/*! \file command.cpp
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: none
 ** Minor contributors (to current version): dejan
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010, 2011  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Implementation of command objects.
 **
 ** Implementation of command objects.
 **/

#include <iostream>
#include <vector>
#include <utility>
#include <iterator>
#include <sstream>

#include "expr/command.h"
#include "smt/smt_engine.h"
#include "smt/bad_option_exception.h"
#include "util/output.h"
#include "util/sexpr.h"
#include "expr/node.h"
#include "printer/printer.h"

using namespace std;

namespace CVC4 {

std::ostream& operator<<(std::ostream& out, const Command& c) {
  c.toStream(out,
             Node::setdepth::getDepth(out),
             Node::printtypes::getPrintTypes(out),
             Node::setlanguage::getLanguage(out));
  return out;
}

ostream& operator<<(ostream& out, const Command* c) {
  if(c == NULL) {
    out << "null";
  } else {
    out << *c;
  }
  return out;
}

/* class Command */

void Command::invoke(SmtEngine* smtEngine, std::ostream& out) {
  invoke(smtEngine);
  printResult(out);
}

std::string Command::toString() const {
  std::stringstream ss;
  toStream(ss);
  return ss.str();
}

void Command::toStream(std::ostream& out, int toDepth, bool types,
                       OutputLanguage language) const {
  Printer::getPrinter(language)->toStream(out, this, toDepth, types);
}

void Command::printResult(std::ostream& out) const {
}

/* class EmptyCommand */

EmptyCommand::EmptyCommand(std::string name) :
  d_name(name) {
}

std::string EmptyCommand::getName() const {
  return d_name;
}

void EmptyCommand::invoke(SmtEngine* smtEngine) {
  /* empty commands have no implementation */
}

Command* EmptyCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  return new EmptyCommand(d_name);
}

/* class AssertCommand */

AssertCommand::AssertCommand(const BoolExpr& e) :
  d_expr(e) {
}

BoolExpr AssertCommand::getExpr() const {
  return d_expr;
}

void AssertCommand::invoke(SmtEngine* smtEngine) {
  smtEngine->assertFormula(d_expr);
}

Command* AssertCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  return new AssertCommand(d_expr.exportTo(exprManager, variableMap));
}

/* class PushCommand */

void PushCommand::invoke(SmtEngine* smtEngine) {
  smtEngine->push();
}

Command* PushCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  return new PushCommand;
}

/* class PopCommand */

void PopCommand::invoke(SmtEngine* smtEngine) {
  smtEngine->pop();
}

/* class CheckSatCommand */

CheckSatCommand::CheckSatCommand() :
  d_expr() {
}

Command* PopCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  return new PopCommand();
}

/* class CheckSatCommand */

CheckSatCommand::CheckSatCommand(const BoolExpr& expr) :
  d_expr(expr) {
}

BoolExpr CheckSatCommand::getExpr() const {
  return d_expr;
}

void CheckSatCommand::invoke(SmtEngine* smtEngine) {
  d_result = smtEngine->checkSat(d_expr);
}

Result CheckSatCommand::getResult() const {
  return d_result;
}

void CheckSatCommand::printResult(std::ostream& out) const {
  out << d_result << endl;
}

Command* CheckSatCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  CheckSatCommand* c = new CheckSatCommand(d_expr.exportTo(exprManager, variableMap));
  c->d_result = d_result;
  return c;
}

/* class QueryCommand */

QueryCommand::QueryCommand(const BoolExpr& e) :
  d_expr(e) {
}

BoolExpr QueryCommand::getExpr() const {
  return d_expr;
}

void QueryCommand::invoke(SmtEngine* smtEngine) {
  d_result = smtEngine->query(d_expr);
}

Result QueryCommand::getResult() const {
  return d_result;
}

void QueryCommand::printResult(std::ostream& out) const {
  out << d_result << endl;
}

Command* QueryCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  QueryCommand* c = new QueryCommand(d_expr.exportTo(exprManager, variableMap));
  c->d_result = d_result;
  return c;
}

/* class QuitCommand */

QuitCommand::QuitCommand() {
}

void QuitCommand::invoke(SmtEngine* smtEngine) {
  Dump("benchmark") << *this;
}

Command* QuitCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  return new QuitCommand();
}

/* class CommentCommand */

CommentCommand::CommentCommand(std::string comment) : d_comment(comment) {
}

std::string CommentCommand::getComment() const {
  return d_comment;
}

void CommentCommand::invoke(SmtEngine* smtEngine) {
  Dump("benchmark") << *this;
}

Command* CommentCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  return new CommentCommand(d_comment);
}

/* class CommandSequence */

CommandSequence::CommandSequence() :
  d_index(0) {
}

CommandSequence::~CommandSequence() {
  for(unsigned i = d_index; i < d_commandSequence.size(); ++i) {
    delete d_commandSequence[i];
  }
}

void CommandSequence::addCommand(Command* cmd) {
  d_commandSequence.push_back(cmd);
}

void CommandSequence::invoke(SmtEngine* smtEngine) {
  for(; d_index < d_commandSequence.size(); ++d_index) {
    d_commandSequence[d_index]->invoke(smtEngine);
    delete d_commandSequence[d_index];
  }
}

void CommandSequence::invoke(SmtEngine* smtEngine, std::ostream& out) {
  for(; d_index < d_commandSequence.size(); ++d_index) {
    d_commandSequence[d_index]->invoke(smtEngine, out);
    delete d_commandSequence[d_index];
  }
}

CommandSequence::const_iterator CommandSequence::begin() const {
  return d_commandSequence.begin();
}

Command* CommandSequence::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  CommandSequence* seq = new CommandSequence();
  for(iterator i = begin(); i != end(); ++i) {
    seq->addCommand((*i)->exportTo(exprManager, variableMap));
  }
  seq->d_index = d_index;
  return seq;
}

CommandSequence::const_iterator CommandSequence::end() const {
  return d_commandSequence.end();
}

CommandSequence::iterator CommandSequence::begin() {
  return d_commandSequence.begin();
}

CommandSequence::iterator CommandSequence::end() {
  return d_commandSequence.end();
}

/* class DeclarationSequenceCommand */

/* class DeclarationDefinitionCommand */

DeclarationDefinitionCommand::DeclarationDefinitionCommand(const std::string& id) :
  d_symbol(id) {
}

std::string DeclarationDefinitionCommand::getSymbol() const {
  return d_symbol;
}

/* class DeclareFunctionCommand */

DeclareFunctionCommand::DeclareFunctionCommand(const std::string& id, Type t) :
  DeclarationDefinitionCommand(id),
  d_type(t) {
}

Type DeclareFunctionCommand::getType() const {
  return d_type;
}

void DeclareFunctionCommand::invoke(SmtEngine* smtEngine) {
  Dump("declarations") << *this << endl;
}

Command* DeclareFunctionCommand::exportTo(ExprManager* exprManager,
                                          ExprManagerMapCollection& variableMap) {
  return new DeclareFunctionCommand(d_symbol,
                                    d_type.exportTo(exprManager, variableMap));
}

/* class DeclareTypeCommand */

DeclareTypeCommand::DeclareTypeCommand(const std::string& id, size_t arity, Type t) :
  DeclarationDefinitionCommand(id),
  d_arity(arity),
  d_type(t) {
}

size_t DeclareTypeCommand::getArity() const {
  return d_arity;
}

Type DeclareTypeCommand::getType() const {
  return d_type;
}

void DeclareTypeCommand::invoke(SmtEngine* smtEngine) {
  Dump("declarations") << *this << endl;
}

Command* DeclareTypeCommand::exportTo(ExprManager* exprManager,
                                      ExprManagerMapCollection& variableMap) {
  return new DeclareTypeCommand(d_symbol, d_arity,
                                d_type.exportTo(exprManager, variableMap));
}

/* class DefineTypeCommand */

DefineTypeCommand::DefineTypeCommand(const std::string& id,
                                     Type t) :
  DeclarationDefinitionCommand(id),
  d_params(),
  d_type(t) {
}

DefineTypeCommand::DefineTypeCommand(const std::string& id,
                                     const std::vector<Type>& params,
                                     Type t) :
  DeclarationDefinitionCommand(id),
  d_params(params),
  d_type(t) {
}

const std::vector<Type>& DefineTypeCommand::getParameters() const {
  return d_params;
}

Type DefineTypeCommand::getType() const {
  return d_type;
}

void DefineTypeCommand::invoke(SmtEngine* smtEngine) {
  Dump("declarations") << *this << endl;
}

Command* DefineTypeCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  vector<Type> params;
  transform(d_params.begin(), d_params.end(), back_inserter(params),
            ExportTransformer(exprManager, variableMap));
  Type type = d_type.exportTo(exprManager, variableMap);
  return new DefineTypeCommand(d_symbol, params, type);
}

/* class DefineFunctionCommand */

DefineFunctionCommand::DefineFunctionCommand(const std::string& id,
                                             Expr func,
                                             Expr formula) :
  DeclarationDefinitionCommand(id),
  d_func(func),
  d_formals(),
  d_formula(formula) {
}

DefineFunctionCommand::DefineFunctionCommand(const std::string& id,
                                             Expr func,
                                             const std::vector<Expr>& formals,
                                             Expr formula) :
  DeclarationDefinitionCommand(id),
  d_func(func),
  d_formals(formals),
  d_formula(formula) {
}

Expr DefineFunctionCommand::getFunction() const {
  return d_func;
}

const std::vector<Expr>& DefineFunctionCommand::getFormals() const {
  return d_formals;
}

Expr DefineFunctionCommand::getFormula() const {
  return d_formula;
}

void DefineFunctionCommand::invoke(SmtEngine* smtEngine) {
  //Dump("declarations") << *this << endl; -- done by SmtEngine
  if(!d_func.isNull()) {
    smtEngine->defineFunction(d_func, d_formals, d_formula);
  }
}

Command* DefineFunctionCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  Expr func = d_func.exportTo(exprManager, variableMap);
  vector<Expr> formals;
  transform(d_formals.begin(), d_formals.end(), back_inserter(formals),
            ExportTransformer(exprManager, variableMap));
  Expr formula = d_formula.exportTo(exprManager, variableMap);
  return new DefineFunctionCommand(d_symbol, func, formals, formula);
}

/* class DefineNamedFunctionCommand */

DefineNamedFunctionCommand::DefineNamedFunctionCommand(const std::string& id,
                                                       Expr func,
                                                       const std::vector<Expr>& formals,
                                                       Expr formula) :
  DefineFunctionCommand(id, func, formals, formula) {
}

void DefineNamedFunctionCommand::invoke(SmtEngine* smtEngine) {
  this->DefineFunctionCommand::invoke(smtEngine);
  if(!d_func.isNull() && d_func.getType().isBoolean()) {
    smtEngine->addToAssignment(d_func.getExprManager()->mkExpr(kind::APPLY,
                                                               d_func));
  }
}

Command* DefineNamedFunctionCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  Expr func = d_func.exportTo(exprManager, variableMap);
  vector<Expr> formals;
  transform(d_formals.begin(), d_formals.end(), back_inserter(formals),
            ExportTransformer(exprManager, variableMap));
  Expr formula = d_formula.exportTo(exprManager, variableMap);
  return new DefineNamedFunctionCommand(d_symbol, func, formals, formula);
}

/* class Simplify */

SimplifyCommand::SimplifyCommand(Expr term) :
  d_term(term) {
}

Expr SimplifyCommand::getTerm() const {
  return d_term;
}

void SimplifyCommand::invoke(SmtEngine* smtEngine) {
  d_result = smtEngine->simplify(d_term);
}

Expr SimplifyCommand::getResult() const {
  return d_result;
}

void SimplifyCommand::printResult(std::ostream& out) const {
  out << d_result << endl;
}

Command* SimplifyCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  SimplifyCommand* c = new SimplifyCommand(d_term.exportTo(exprManager, variableMap));
  c->d_result = d_result.exportTo(exprManager, variableMap);
  return c;
}

/* class GetValueCommand */

GetValueCommand::GetValueCommand(Expr term) :
  d_term(term) {
}

Expr GetValueCommand::getTerm() const {
  return d_term;
}

void GetValueCommand::invoke(SmtEngine* smtEngine) {
  d_result = d_term.getExprManager()->mkExpr(kind::TUPLE, d_term,
                                             smtEngine->getValue(d_term));
}

Expr GetValueCommand::getResult() const {
  return d_result;
}

void GetValueCommand::printResult(std::ostream& out) const {
  out << d_result << endl;
}

Command* GetValueCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  GetValueCommand* c = new GetValueCommand(d_term.exportTo(exprManager, variableMap));
  c->d_result = d_result.exportTo(exprManager, variableMap);
  return c;
}

/* class GetAssignmentCommand */

GetAssignmentCommand::GetAssignmentCommand() {
}

void GetAssignmentCommand::invoke(SmtEngine* smtEngine) {
  d_result = smtEngine->getAssignment();
}

SExpr GetAssignmentCommand::getResult() const {
  return d_result;
}

void GetAssignmentCommand::printResult(std::ostream& out) const {
  out << d_result << endl;
}

Command* GetAssignmentCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  GetAssignmentCommand* c = new GetAssignmentCommand;
  c->d_result = d_result;
  return c;
}

/* class GetProofCommand */

GetProofCommand::GetProofCommand() {
}

void GetProofCommand::invoke(SmtEngine* smtEngine) {
  d_result = smtEngine->getProof();
}

Proof* GetProofCommand::getResult() const {
  return d_result;
}

void GetProofCommand::printResult(std::ostream& out) const {
  d_result->toStream(out);
}

Command* GetProofCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  GetProofCommand* c = new GetProofCommand;
  c->d_result = d_result;
  return c;
}

/* class GetAssertionsCommand */

GetAssertionsCommand::GetAssertionsCommand() {
}

void GetAssertionsCommand::invoke(SmtEngine* smtEngine) {
  stringstream ss;
  const vector<Expr> v = smtEngine->getAssertions();
  copy( v.begin(), v.end(), ostream_iterator<Expr>(ss, "\n") );
  d_result = ss.str();
}

std::string GetAssertionsCommand::getResult() const {
  return d_result;
}

void GetAssertionsCommand::printResult(std::ostream& out) const {
  out << d_result;
}

Command* GetAssertionsCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  GetAssertionsCommand* c = new GetAssertionsCommand;
  c->d_result = d_result;
  return c;
}

/* class SetBenchmarkStatusCommand */

SetBenchmarkStatusCommand::SetBenchmarkStatusCommand(BenchmarkStatus status) :
  d_status(status) {
}

BenchmarkStatus SetBenchmarkStatusCommand::getStatus() const {
  return d_status;
}

void SetBenchmarkStatusCommand::invoke(SmtEngine* smtEngine) {
  stringstream ss;
  ss << d_status;
  SExpr status = ss.str();
  try {
    smtEngine->setInfo(":status", status);
    //d_result = "success";
  } catch(ModalException&) {
    d_result = "error";
  } catch(BadOptionException&) {
    // should not happen
    d_result = "error";
  }
}

Command* SetBenchmarkStatusCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  SetBenchmarkStatusCommand* c = new SetBenchmarkStatusCommand(d_status);
  c->d_result = d_result;
  return c;
}

/* class SetBenchmarkLogicCommand */

SetBenchmarkLogicCommand::SetBenchmarkLogicCommand(std::string logic) :
  d_logic(logic) {
}

std::string SetBenchmarkLogicCommand::getLogic() const {
  return d_logic;
}

void SetBenchmarkLogicCommand::invoke(SmtEngine* smtEngine) {
  try {
    smtEngine->setLogic(d_logic);
    //d_result = "success";
  } catch(ModalException&) {
    d_result = "error";
  }
}

Command* SetBenchmarkLogicCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  SetBenchmarkLogicCommand* c = new SetBenchmarkLogicCommand(d_logic);
  c->d_result = d_result;
  return c;
}

/* class SetInfoCommand */

SetInfoCommand::SetInfoCommand(std::string flag, const SExpr& sexpr) :
  d_flag(flag),
  d_sexpr(sexpr) {
}

std::string SetInfoCommand::getFlag() const {
  return d_flag;
}

SExpr SetInfoCommand::getSExpr() const {
  return d_sexpr;
}

void SetInfoCommand::invoke(SmtEngine* smtEngine) {
  try {
    smtEngine->setInfo(d_flag, d_sexpr);
    //d_result = "success";
  } catch(ModalException&) {
    d_result = "error";
  } catch(BadOptionException&) {
    d_result = "unsupported";
  }
}

std::string SetInfoCommand::getResult() const {
  return d_result;
}

void SetInfoCommand::printResult(std::ostream& out) const {
  if(d_result != "") {
    out << d_result << endl;
  }
}

Command* SetInfoCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  SetInfoCommand* c = new SetInfoCommand(d_flag, d_sexpr);
  c->d_result = d_result;
  return c;
}

/* class GetInfoCommand */

GetInfoCommand::GetInfoCommand(std::string flag) :
  d_flag(flag) {
}

std::string GetInfoCommand::getFlag() const {
  return d_flag;
}

void GetInfoCommand::invoke(SmtEngine* smtEngine) {
  try {
    stringstream ss;
    ss << smtEngine->getInfo(d_flag);
    d_result = ss.str();
  } catch(BadOptionException&) {
    d_result = "unsupported";
  }
}

std::string GetInfoCommand::getResult() const {
  return d_result;
}

void GetInfoCommand::printResult(std::ostream& out) const {
  if(d_result != "") {
    out << d_result << endl;
  }
}

Command* GetInfoCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  GetInfoCommand* c = new GetInfoCommand(d_flag);
  c->d_result = d_result;
  return c;
}

/* class SetOptionCommand */

SetOptionCommand::SetOptionCommand(std::string flag, const SExpr& sexpr) :
  d_flag(flag),
  d_sexpr(sexpr) {
}

std::string SetOptionCommand::getFlag() const {
  return d_flag;
}

SExpr SetOptionCommand::getSExpr() const {
  return d_sexpr;
}

void SetOptionCommand::invoke(SmtEngine* smtEngine) {
  try {
    smtEngine->setOption(d_flag, d_sexpr);
    //d_result = "success";
  } catch(ModalException&) {
    d_result = "error";
  } catch(BadOptionException&) {
    d_result = "unsupported";
  }
}

std::string SetOptionCommand::getResult() const {
  return d_result;
}

void SetOptionCommand::printResult(std::ostream& out) const {
  if(d_result != "") {
    out << d_result << endl;
  }
}

Command* SetOptionCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  SetOptionCommand* c = new SetOptionCommand(d_flag, d_sexpr);
  c->d_result = d_result;
  return c;
}

/* class GetOptionCommand */

GetOptionCommand::GetOptionCommand(std::string flag) :
  d_flag(flag) {
}

std::string GetOptionCommand::getFlag() const {
  return d_flag;
}

void GetOptionCommand::invoke(SmtEngine* smtEngine) {
  try {
    d_result = smtEngine->getOption(d_flag).getValue();
  } catch(BadOptionException&) {
    d_result = "unsupported";
  }
}

std::string GetOptionCommand::getResult() const {
  return d_result;
}

void GetOptionCommand::printResult(std::ostream& out) const {
  if(d_result != "") {
    out << d_result << endl;
  }
}

Command* GetOptionCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  GetOptionCommand* c = new GetOptionCommand(d_flag);
  c->d_result = d_result;
  return c;
}

/* class DatatypeDeclarationCommand */

DatatypeDeclarationCommand::DatatypeDeclarationCommand(const DatatypeType& datatype) :
  d_datatypes() {
  d_datatypes.push_back(datatype);
}

DatatypeDeclarationCommand::DatatypeDeclarationCommand(const std::vector<DatatypeType>& datatypes) :
  d_datatypes(datatypes) {
}

const std::vector<DatatypeType>&
DatatypeDeclarationCommand::getDatatypes() const {
  return d_datatypes;
}

void DatatypeDeclarationCommand::invoke(SmtEngine* smtEngine) {
  Dump("declarations") << *this << endl;
}

Command* DatatypeDeclarationCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  std::cout << "We currently do not support exportTo with Datatypes" << std::endl;
  exit(1);
  return NULL;
}
/* output stream insertion operator for benchmark statuses */
std::ostream& operator<<(std::ostream& out,
                         BenchmarkStatus status) {
  switch(status) {

  case SMT_SATISFIABLE:
    return out << "sat";

  case SMT_UNSATISFIABLE:
    return out << "unsat";

  case SMT_UNKNOWN:
    return out << "unknown";

  default:
    return out << "SetBenchmarkStatusCommand::[UNKNOWNSTATUS!]";
  }
}

}/* CVC4 namespace */
