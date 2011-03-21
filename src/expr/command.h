/*********************                                                        */
/*! \file command.h
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: cconway, dejan
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Implementation of the command pattern on SmtEngines.
 **
 ** Implementation of the command pattern on SmtEngines.  Command
 ** objects are generated by the parser (typically) to implement the
 ** commands in parsed input (see Parser::parseNextCommand()), or by
 ** client code.
 **/

#include "cvc4_public.h"

#ifndef __CVC4__COMMAND_H
#define __CVC4__COMMAND_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "expr/expr.h"
#include "expr/type.h"
#include "expr/variable_type_map.h"
#include "util/result.h"
#include "util/sexpr.h"

namespace CVC4 {

class SmtEngine;
class Command;

std::ostream& operator<<(std::ostream&, const Command&) CVC4_PUBLIC;
std::ostream& operator<<(std::ostream&, const Command*) CVC4_PUBLIC;

/** The status an SMT benchmark can have */
enum BenchmarkStatus {
  /** Benchmark is satisfiable */
  SMT_SATISFIABLE,
  /** Benchmark is unsatisfiable */
  SMT_UNSATISFIABLE,
  /** The status of the benchmark is unknown */
  SMT_UNKNOWN
};

std::ostream& operator<<(std::ostream& out,
                         BenchmarkStatus status) CVC4_PUBLIC;

class CVC4_PUBLIC Command {
  // intentionally not permitted
  Command(const Command&) CVC4_UNDEFINED;
  Command& operator=(const Command&) CVC4_UNDEFINED;

public:

  virtual void invoke(SmtEngine* smtEngine) = 0;
  virtual void invoke(SmtEngine* smtEngine, std::ostream& out);
  Command() { }
  virtual ~Command() { }
  virtual void toStream(std::ostream&) const = 0;
  std::string toString() const;
  virtual void printResult(std::ostream& out) const;
  /**
   * Maps this Command into one for a different ExprManager, using
   * variableMap for the translation and extending it with any new
   * mappings.
   */
  virtual Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap) = 0;

protected:
  class ExportTransformer {
    ExprManager* d_exprManager;
    VariableTypeMap& d_variableMap;
  public:
    ExportTransformer(ExprManager* exprManager, VariableTypeMap& variableMap) :
      d_exprManager(exprManager),
      d_variableMap(variableMap) {
    }
    Expr operator()(Expr e) {
      return e.exportTo(d_exprManager, d_variableMap);
    }
  };/* class Command::ExportTransformer */

};/* class Command */

/**
 * EmptyCommands (and its subclasses) are the residue of a command
 * after the parser handles them (and there's nothing left to do).
 */
class CVC4_PUBLIC EmptyCommand : public Command {
protected:
  std::string d_name;
public:
  EmptyCommand(std::string name = "");
  void invoke(SmtEngine* smtEngine);
  void toStream(std::ostream& out) const;
  Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap);
};/* class EmptyCommand */

class CVC4_PUBLIC AssertCommand : public Command {
protected:
  BoolExpr d_expr;
public:
  AssertCommand(const BoolExpr& e);
  void invoke(SmtEngine* smtEngine);
  void toStream(std::ostream& out) const;
  Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap);
};/* class AssertCommand */

class CVC4_PUBLIC PushCommand : public Command {
public:
  void invoke(SmtEngine* smtEngine);
  void toStream(std::ostream& out) const;
  Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap);
};/* class PushCommand */

class CVC4_PUBLIC PopCommand : public Command {
public:
  void invoke(SmtEngine* smtEngine);
  void toStream(std::ostream& out) const;
  Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap);
};/* class PopCommand */

class CVC4_PUBLIC DeclarationCommand : public EmptyCommand {
protected:
  std::vector<std::string> d_declaredSymbols;
  Type d_type;
public:
  DeclarationCommand(const std::string& id, Type t);
  DeclarationCommand(const std::vector<std::string>& ids, Type t);
  void toStream(std::ostream& out) const;
  Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap);
};/* class DeclarationCommand */

class CVC4_PUBLIC DefineFunctionCommand : public Command {
protected:
  Expr d_func;
  std::vector<Expr> d_formals;
  Expr d_formula;
public:
  DefineFunctionCommand(Expr func,
                        const std::vector<Expr>& formals,
                        Expr formula);
  void invoke(SmtEngine* smtEngine);
  void toStream(std::ostream& out) const;
  Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap);
};/* class DefineFunctionCommand */

/**
 * This differs from DefineFunctionCommand only in that it instructs
 * the SmtEngine to "remember" this function for later retrieval with
 * getAssignment().  Used for :named attributes in SMT-LIBv2.
 */
class CVC4_PUBLIC DefineNamedFunctionCommand : public DefineFunctionCommand {
public:
  DefineNamedFunctionCommand(Expr func,
                             const std::vector<Expr>& formals,
                             Expr formula);
  void invoke(SmtEngine* smtEngine);
  void toStream(std::ostream& out) const;
  Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap);
};/* class DefineNamedFunctionCommand */

class CVC4_PUBLIC CheckSatCommand : public Command {
protected:
  BoolExpr d_expr;
  Result d_result;
public:
  CheckSatCommand(const BoolExpr& expr);
  void invoke(SmtEngine* smtEngine);
  Result getResult() const;
  void printResult(std::ostream& out) const;
  void toStream(std::ostream& out) const;
  Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap);
};/* class CheckSatCommand */

class CVC4_PUBLIC QueryCommand : public Command {
protected:
  BoolExpr d_expr;
  Result d_result;
public:
  QueryCommand(const BoolExpr& e);
  void invoke(SmtEngine* smtEngine);
  Result getResult() const;
  void printResult(std::ostream& out) const;
  void toStream(std::ostream& out) const;
  Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap);
};/* class QueryCommand */

class CVC4_PUBLIC GetValueCommand : public Command {
protected:
  Expr d_term;
  Expr d_result;
public:
  GetValueCommand(Expr term);
  void invoke(SmtEngine* smtEngine);
  Expr getResult() const;
  void printResult(std::ostream& out) const;
  void toStream(std::ostream& out) const;
  Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap);
};/* class GetValueCommand */

class CVC4_PUBLIC GetAssignmentCommand : public Command {
protected:
  SExpr d_result;
public:
  GetAssignmentCommand();
  void invoke(SmtEngine* smtEngine);
  SExpr getResult() const;
  void printResult(std::ostream& out) const;
  void toStream(std::ostream& out) const;
  Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap);
};/* class GetAssignmentCommand */

class CVC4_PUBLIC GetAssertionsCommand : public Command {
protected:
  std::string d_result;
public:
  GetAssertionsCommand();
  void invoke(SmtEngine* smtEngine);
  std::string getResult() const;
  void printResult(std::ostream& out) const;
  void toStream(std::ostream& out) const;
  Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap);
};/* class GetAssertionsCommand */

class CVC4_PUBLIC SetBenchmarkStatusCommand : public Command {
protected:
  std::string d_result;
  BenchmarkStatus d_status;
public:
  SetBenchmarkStatusCommand(BenchmarkStatus status);
  void invoke(SmtEngine* smtEngine);
  void toStream(std::ostream& out) const;
  Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap);
};/* class SetBenchmarkStatusCommand */

class CVC4_PUBLIC SetBenchmarkLogicCommand : public Command {
protected:
  std::string d_result;
  std::string d_logic;
public:
  SetBenchmarkLogicCommand(std::string logic);
  void invoke(SmtEngine* smtEngine);
  void toStream(std::ostream& out) const;
  Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap);
};/* class SetBenchmarkLogicCommand */

class CVC4_PUBLIC SetInfoCommand : public Command {
protected:
  std::string d_flag;
  SExpr d_sexpr;
  std::string d_result;
public:
  SetInfoCommand(std::string flag, SExpr& sexpr);
  void invoke(SmtEngine* smtEngine);
  void toStream(std::ostream& out) const;
  std::string getResult() const;
  void printResult(std::ostream& out) const;
  Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap);
};/* class SetInfoCommand */

class CVC4_PUBLIC GetInfoCommand : public Command {
protected:
  std::string d_flag;
  std::string d_result;
public:
  GetInfoCommand(std::string flag);
  void invoke(SmtEngine* smtEngine);
  void toStream(std::ostream& out) const;
  std::string getResult() const;
  void printResult(std::ostream& out) const;
  Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap);
};/* class GetInfoCommand */

class CVC4_PUBLIC SetOptionCommand : public Command {
protected:
  std::string d_flag;
  SExpr d_sexpr;
  std::string d_result;
public:
  SetOptionCommand(std::string flag, SExpr& sexpr);
  void invoke(SmtEngine* smtEngine);
  void toStream(std::ostream& out) const;
  std::string getResult() const;
  void printResult(std::ostream& out) const;
  Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap);
};/* class SetOptionCommand */

class CVC4_PUBLIC GetOptionCommand : public Command {
protected:
  std::string d_flag;
  std::string d_result;
public:
  GetOptionCommand(std::string flag);
  void invoke(SmtEngine* smtEngine);
  void toStream(std::ostream& out) const;
  std::string getResult() const;
  void printResult(std::ostream& out) const;
  Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap);
};/* class GetOptionCommand */

class CVC4_PUBLIC CommandSequence : public Command {
private:
  /** All the commands to be executed (in sequence) */
  std::vector<Command*> d_commandSequence;
  /** Next command to be executed */
  unsigned int d_index;
public:
  CommandSequence();
  ~CommandSequence();
  void invoke(SmtEngine* smtEngine);
  void invoke(SmtEngine* smtEngine, std::ostream& out);
  void addCommand(Command* cmd);
  void toStream(std::ostream& out) const;

  typedef std::vector<Command*>::iterator iterator;
  typedef std::vector<Command*>::const_iterator const_iterator;

  const_iterator begin() const { return d_commandSequence.begin(); }
  const_iterator end() const { return d_commandSequence.end(); }

  iterator begin() { return d_commandSequence.begin(); }
  iterator end() { return d_commandSequence.end(); }

  Command* exportTo(ExprManager* exprManager, VariableTypeMap& variableMap);
};/* class CommandSequence */

}/* CVC4 namespace */

#endif /* __CVC4__COMMAND_H */
