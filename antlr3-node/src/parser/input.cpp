/*********************                                                        */
/** parser.cpp
 ** Original author: mdeters
 ** Major contributors: dejan
 ** Minor contributors (to current version): cconway
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.
 **
 ** Parser implementation.
 **/

#include <iostream>
#include <fstream>
#include <stdint.h>

#include "input.h"
#include "expr/command.h"
#include "expr/expr.h"
#include "expr/kind.h"
#include "expr/type.h"
#include "util/output.h"
#include "util/Assert.h"
#include "parser/parser_exception.h"
#include "parser/smt/smt_input.h"

using namespace std;
using namespace CVC4::kind;

namespace CVC4 {
namespace parser {

void Input::setDone(bool done) {
  d_done = done;
}

bool Input::done() const {
  return d_done;
}

Command* Input::parseNextCommand() throw (ParserException) {
  Debug("parser") << "parseNextCommand()" << std::endl;
  Command* cmd = NULL;
  if(!done()) {
    try {
      cmd = doParseCommand();
      if(cmd == NULL) {
        setDone();
      }
    } catch(ParserException& e) {
      setDone();
      throw ParserException(e.toString());
    }
  }
  Debug("parser") << "parseNextCommand() => " << cmd << std::endl;
  return cmd;
}

Expr Input::parseNextExpression() throw (ParserException) {
  Debug("parser") << "parseNextExpression()" << std::endl;
  Node result;
  if(!done()) {
    try {
      result = doParseExpr();
      if(result.isNull())
        setDone();
    } catch(ParserException& e) {
      setDone();
      throw ParserException(e.toString());
    }
  }
  Debug("parser") << "parseNextExpression() => " << result << std::endl;
  return toExpr(result);
}

Input::~Input() {
}

Input::Input(ExprManager* exprManager, const std::string& filename) :
  d_nodeManager(exprManager->getNodeManager()),
  d_exprManager(exprManager),
  d_nodeManagerScope(d_nodeManager),
  d_filename(filename),
  d_done(false),
  d_checksEnabled(true) {
}

Input* Input::newFileInput(ExprManager* exprManager, InputLanguage lang,
                              const std::string& filename, bool useMmap) {

  Input* parser = 0;

  switch(lang) {
/*  case LANG_CVC4: {
    antlrLexer = new AntlrCvcLexer(*inputBuffer);
    antlrInput = new AntlrCvcInput(*antlrLexer);
    break;
  }*/
  case LANG_SMTLIB:
    parser = new Smt(exprManager,filename,useMmap);
    break;

  default:
    Unhandled("Unknown Input language!");
  }

  return parser;
}

/*
Input* Input::getNewInput(NodeManager* em, InputLanguage lang,
                             istream& input, string filename) {
  antlr::CharBuffer* inputBuffer = new CharBuffer(input);
  return getNewInput(em, lang, inputBuffer, filename);
}
*/

/*
Input* Input::getNewInput(NodeManager* em, InputLanguage lang,
                             std::istream& input, const std::string& name) {
  Input* parser = 0;

  switch(lang) {
  case LANG_CVC4: {
    antlrLexer = new AntlrCvcLexer(*inputBuffer);
    antlrInput = new AntlrCvcInput(*antlrLexer);
    break;
  }
  case LANG_SMTLIB:
    parser = new Smt(em,input,name);
    break;

  default:
    Unhandled("Unknown Input language!");
  }
  return parser;
}
*/

Input* Input::newStringInput(ExprManager* exprManager, InputLanguage lang,
                             const std::string& input, const std::string& name) {
  Input* parser = 0;

  switch(lang) {
/*  case LANG_CVC4: {
    antlrLexer = new AntlrCvcLexer(*inputBuffer);
    antlrInput = new AntlrCvcInput(*antlrLexer);
    break;
  }*/
  case LANG_SMTLIB:
    parser = new Smt(exprManager,input,name);
    break;

  default:
    Unhandled("Unknown Input language!");
  }
  return parser;
}

Node Input::getSymbol(const std::string& name, SymbolType type) {
  Assert( isDeclared(name, type) );


  switch( type ) {

  case SYM_VARIABLE: // Functions share var namespace
  case SYM_FUNCTION:
    return d_varSymbolTable.getObject(name);

  default:
    Unhandled("Unhandled symbol type!");
  }
}

Node Input::getVariable(const std::string& name) {
  return getSymbol(name, SYM_VARIABLE);
}

Node Input::getFunction(const std::string& name) {
  return getSymbol(name, SYM_FUNCTION);
}

const Type*
Input::getType(const std::string& var_name,
                     SymbolType type) {
  Assert( isDeclared(var_name, type) );
  const Type* t = getSymbol(var_name,type).getType();
  return t;
}

const Type* Input::getSort(const std::string& name) {
  Assert( isDeclared(name, SYM_SORT) );
  const Type* t = d_sortTable.getObject(name);
  return t;
}

/* Returns true if name is bound to a boolean variable. */
bool Input::isBoolean(const std::string& name) {
  return isDeclared(name, SYM_VARIABLE) && getType(name)->isBoolean();
}

/* Returns true if name is bound to a function. */
bool Input::isFunction(const std::string& name) {
  return isDeclared(name, SYM_FUNCTION) && getType(name)->isFunction();
}

/* Returns true if name is bound to a function returning boolean. */
bool Input::isPredicate(const std::string& name) {
  return isDeclared(name, SYM_FUNCTION) && getType(name)->isPredicate();
}

Node Input::trueNode() const {
  return d_nodeManager->mkNode(TRUE);
}

Node Input::falseNode() const {
  return d_nodeManager->mkNode(FALSE);
}

Node Input::mkNode(Kind kind, TNode child) {
  Node result = d_nodeManager->mkNode(kind, child);
  Debug("parser") << "mkNode() => " << result << std::endl;
  return result;
}

Node Input::mkNode(Kind kind, TNode child_1, TNode child_2) {
  Node result = d_nodeManager->mkNode(kind, child_1, child_2);
  Debug("parser") << "mkNode() => " << result << std::endl;
  return result;
}

Node Input::mkNode(Kind kind, TNode child_1, TNode child_2,
                         TNode child_3) {
  Node result = d_nodeManager->mkNode(kind, child_1, child_2, child_3);
  Debug("parser") << "mkNode() => " << result << std::endl;
  return result;
}

Node Input::mkNode(Kind kind, const std::vector<Node>& children) {
  Node result = d_nodeManager->mkNode(kind, children);
  Debug("parser") << "mkNode() => " << result << std::endl;
  return result;
}


/* TODO: This could be pushed into NodeManager. */
Node Input::mkDistinct(const std::vector<Node>& args) {
  if( args.size() == 1 ) {
    return trueNode();
  }

  if( args.size() == 2 ) {
    return mkNode(NOT, mkNode(EQUAL,args[0],args[1]));
  }

  /* Explode into pairwise disequalities. */
  std::vector<Expr> diseqs;
  for(unsigned i = 0; i < args.size(); ++i) {
    for(unsigned j = i+1; j < args.size(); ++j) {
      diseqs.push_back(mkNode(NOT, mkNode(EQUAL,args[i],args[j])));
    }
  }
  return mkNode(AND, diseqs);
}



const Expr Input::toExpr(Node node) {
  return Expr(d_exprManager,new Node(node));
}

const Type*
Input::functionType(const Type* domainType,
                          const Type* rangeType) {
  return d_nodeManager->mkFunctionType(domainType,rangeType);
}

const Type*
Input::functionType(const std::vector<const Type*>& argTypes,
                          const Type* rangeType) {
  Assert( argTypes.size() > 0 );
  return d_nodeManager->mkFunctionType(argTypes,rangeType);
}

const Type*
Input::functionType(const std::vector<const Type*>& sorts) {
  Assert( sorts.size() > 0 );
  if( sorts.size() == 1 ) {
    return sorts[0];
  } else {
    std::vector<const Type*> argTypes(sorts);
    const Type* rangeType = argTypes.back();
    argTypes.pop_back();
    return functionType(argTypes,rangeType);
  }
}

const Type* Input::predicateType(const std::vector<const Type*>& sorts) {
  if(sorts.size() == 0) {
    return d_nodeManager->booleanType();
  } else {
    return d_nodeManager->mkFunctionType(sorts, d_nodeManager->booleanType());
  }
}

Node
Input::mkVar(const std::string& name, const Type* type) {
  Debug("parser") << "mkVar(" << name << "," << *type << ")" << std::endl;
  Node expr = d_nodeManager->mkVar(type, name);
  defineVar(name,expr);
  return expr;
}

const std::vector<Node>
Input::mkVars(const std::vector<std::string> names,
                    const Type* type) {
  std::vector<Node> vars;
  for(unsigned i = 0; i < names.size(); ++i) {
    vars.push_back(mkVar(names[i], type));
  }
  return vars;
}

void
Input::defineVar(const std::string& name, const Node& val) {
  Assert(!isDeclared(name));
  d_varSymbolTable.bindName(name,val);
  Assert(isDeclared(name));
}

void
Input::undefineVar(const std::string& name) {
  Assert(isDeclared(name));
  d_varSymbolTable.unbindName(name);
  Assert(!isDeclared(name));
}

void
Input::setLogic(const std::string& name) {
  if( name == "QF_UF" ) {
    newSort("U");
  } else {
    Unhandled("setLogic: " + name);
  }
}

const Type*
Input::newSort(const std::string& name) {
  Debug("parser") << "newSort(" << name << ")" << std::endl;
  Assert( !isDeclared(name, SYM_SORT) ) ;
  const Type* type = d_nodeManager->mkSort(name);
  d_sortTable.bindName(name, type);
  Assert( isDeclared(name, SYM_SORT) ) ;
  return type;
}

const std::vector<const Type*>
Input::newSorts(const std::vector<std::string>& names) {
  std::vector<const Type*> types;
  for(unsigned i = 0; i < names.size(); ++i) {
    types.push_back(newSort(names[i]));
  }
  return types;
}

const BooleanType* Input::booleanType() {
  return d_nodeManager->booleanType();
}

const KindType* Input::kindType() {
  return d_nodeManager->kindType();
}

unsigned int Input::minArity(Kind kind) {
  switch(kind) {
  case FALSE:
  case SKOLEM:
  case TRUE:
  case VARIABLE:
    return 0;

  case AND:
  case NOT:
  case OR:
    return 1;

  case APPLY:
  case EQUAL:
  case IFF:
  case IMPLIES:
  case PLUS:
  case XOR:
    return 2;

  case ITE:
    return 3;

  default:
    Unhandled("kind in minArity");
  }
}

unsigned int Input::maxArity(Kind kind) {
  switch(kind) {
  case FALSE:
  case SKOLEM:
  case TRUE:
  case VARIABLE:
    return 0;

  case NOT:
    return 1;

  case EQUAL:
  case IFF:
  case IMPLIES:
  case XOR:
    return 2;

  case ITE:
    return 3;

  case AND:
  case APPLY:
  case PLUS:
  case OR:
    return UINT_MAX;

  default:
    Unhandled("kind in maxArity");
  }
}

bool Input::isDeclared(const std::string& name, SymbolType type) {
  switch(type) {
  case SYM_VARIABLE: // Functions share var namespace
  case SYM_FUNCTION:
    return d_varSymbolTable.isBound(name);
  case SYM_SORT:
    return d_sortTable.isBound(name);
  default:
    Unhandled("Unhandled symbol type!");
  }
}

void Input::checkDeclaration(const std::string& varName,
                                   DeclarationCheck check,
                                   SymbolType type)
    throw (ParserException) {
  if(!d_checksEnabled) {
    return;
  }

  switch(check) {
  case CHECK_DECLARED:
    if( !isDeclared(varName, type) ) {
      parseError("Symbol " + varName + " not declared");
    }
    break;

  case CHECK_UNDECLARED:
    if( isDeclared(varName, type) ) {
      parseError("Symbol " + varName + " previously declared");
    }
    break;

  case CHECK_NONE:
    break;

  default:
    Unhandled("Unknown check type!");
  }
}

void Input::checkFunction(const std::string& name)
  throw (ParserException) {
  if( d_checksEnabled && !isFunction(name) ) {
    parseError("Expecting function symbol, found '" + name + "'");
  }
}

void Input::checkArity(Kind kind, unsigned int numArgs)
  throw (ParserException) {
  if(!d_checksEnabled) {
    return;
  }

  unsigned int min = minArity(kind);
  unsigned int max = maxArity(kind);

  if( numArgs < min || numArgs > max ) {
    stringstream ss;
    ss << "Expecting ";
    if( numArgs < min ) {
      ss << "at least " << min << " ";
    } else {
      ss << "at most " << max << " ";
    }
    ss << "arguments for operator '" << kind << "', ";
    ss << "found " << numArgs;
    parseError(ss.str());
  }
}

void Input::enableChecks() {
  d_checksEnabled = true;
}

void Input::disableChecks() {
  d_checksEnabled = false;
}


}/* CVC4::parser namespace */
}/* CVC4 namespace */
