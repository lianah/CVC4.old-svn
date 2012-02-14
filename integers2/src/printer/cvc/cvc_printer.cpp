/*********************                                                        */
/*! \file cvc_printer.cpp
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: none
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010, 2011  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief The pretty-printer interface for the CVC output language
 **
 ** The pretty-printer interface for the CVC output language.
 **/

#include "printer/cvc/cvc_printer.h"
#include "expr/expr.h" // for ExprSetDepth etc..
#include "util/language.h" // for LANG_AST
#include "expr/node_manager.h" // for VarNameAttr
#include "expr/command.h"

#include <iostream>
#include <vector>
#include <string>
#include <typeinfo>
#include <algorithm>
#include <iterator>
#include <stack>

using namespace std;

namespace CVC4 {
namespace printer {
namespace cvc {

void CvcPrinter::toStream(std::ostream& out, TNode n,
                          int toDepth, bool types) const throw() {
  // null
  if(n.getKind() == kind::NULL_EXPR) {
    out << "NULL";
    return;
  }

  // variable
  if(n.getMetaKind() == kind::metakind::VARIABLE) {
    string s;
    if(n.getAttribute(expr::VarNameAttr(), s)) {
      out << s;
    } else {
      if(n.getKind() == kind::VARIABLE) {
        out << "var_";
      } else {
        out << n.getKind() << '_';
      }
      out << n.getId();
    }
    if(types) {
      // print the whole type, but not *its* type
      out << ":";
      n.getType().toStream(out, -1, false, language::output::LANG_CVC4);
    }

    return;
  }

  // constant
  if(n.getMetaKind() == kind::metakind::CONSTANT) {
    switch(n.getKind()) {
    case kind::BITVECTOR_TYPE:
      out << "BITVECTOR(" << n.getConst<BitVectorSize>().size << ")";
      break;
    case kind::CONST_BITVECTOR: {
      const BitVector& bv = n.getConst<BitVector>();
      const Integer& x = bv.getValue();
      out << "0bin";
      unsigned n = bv.getSize();
      while(n-- > 0) {
        out << (x.testBit(n) ? '1' : '0');
      }
      break;
    }
    case kind::CONST_BOOLEAN:
      // the default would print "1" or "0" for bool, that's not correct
      // for our purposes
      out << (n.getConst<bool>() ? "TRUE" : "FALSE");
      break;
    case kind::CONST_RATIONAL: {
      const Rational& rat = n.getConst<Rational>();
      out << '(' << rat.getNumerator() << '/' << rat.getDenominator() << ')';
      break;
    }
    case kind::TYPE_CONSTANT:
      switch(TypeConstant tc = n.getConst<TypeConstant>()) {
      case REAL_TYPE:
        out << "REAL";
        break;
      case INTEGER_TYPE:
        out << "INT";
        break;
      case BOOLEAN_TYPE:
        out << "BOOLEAN";
        break;
      case PSEUDOBOOLEAN_TYPE:
        out << "PSEUDOBOOLEAN";
        break;
      case KIND_TYPE:
        out << "TYPE";
        break;
      default:
        out << tc;
      }
      break;
    case kind::BUILTIN:
      switch(Kind k = n.getConst<Kind>()) {
      case kind::EQUAL: out << '='; break;
      case kind::PLUS: out << '+'; break;
      case kind::MULT: out << '*'; break;
      case kind::MINUS:
      case kind::UMINUS: out << '-'; break;
      case kind::DIVISION: out << '/'; break;
      case kind::LT: out << '<'; break;
      case kind::LEQ: out << "<="; break;
      case kind::GT: out << '>'; break;
      case kind::GEQ: out << ">="; break;
      case kind::IMPLIES: out << "=>"; break;
      case kind::IFF: out << "<=>"; break;

        // names are slightly different than the kind
      case kind::BITVECTOR_PLUS: out << "BVPLUS"; break;
      case kind::BITVECTOR_SUB: out << "BVSUB"; break;
      case kind::BITVECTOR_XOR: out << "BVXOR"; break;
      case kind::BITVECTOR_NAND: out << "BVNAND"; break;
      case kind::BITVECTOR_NOR: out << "BVNOR"; break;
      case kind::BITVECTOR_XNOR: out << "BVXNOR"; break;
      case kind::BITVECTOR_COMP: out << "BVCOMP"; break;
      case kind::BITVECTOR_MULT: out << "BVMULT"; break;
      case kind::BITVECTOR_UDIV: out << "BVUDIV"; break;
      case kind::BITVECTOR_UREM: out << "BVUREM"; break;
      case kind::BITVECTOR_SDIV: out << "BVSDIV"; break;
      case kind::BITVECTOR_SREM: out << "BVSREM"; break;
      case kind::BITVECTOR_SMOD: out << "BVSMOD"; break;
      case kind::BITVECTOR_SHL: out << "BVSHL"; break;
      case kind::BITVECTOR_LSHR: out << "BVLSHR"; break;
      case kind::BITVECTOR_ASHR: out << "BVASHR"; break;
      case kind::BITVECTOR_ULT: out << "BVLT"; break;
      case kind::BITVECTOR_ULE: out << "BVLE"; break;
      case kind::BITVECTOR_UGT: out << "BVGT"; break;
      case kind::BITVECTOR_UGE: out << "BVGE"; break;
      case kind::BITVECTOR_SLT: out << "BVSLT"; break;
      case kind::BITVECTOR_SLE: out << "BVSLE"; break;
      case kind::BITVECTOR_SGT: out << "BVSGT"; break;
      case kind::BITVECTOR_SGE: out << "BVSGE"; break;
      case kind::BITVECTOR_NEG: out << "BVUMINUS"; break;

      case kind::BITVECTOR_NOT: out << "~"; break;
      case kind::BITVECTOR_AND: out << "&"; break;
      case kind::BITVECTOR_OR: out << "|"; break;
      case kind::BITVECTOR_CONCAT: out << "@"; break;

      default:
        out << k;
      }
      break;
    default:
      // fall back on whatever operator<< does on underlying type; we
      // might luck out and be SMT-LIB v2 compliant
      kind::metakind::NodeValueConstPrinter::toStream(out, n);
    }

    return;
  } else if(n.getMetaKind() == kind::metakind::PARAMETERIZED) {
    switch(n.getKind()) {
    case kind::SORT_TYPE: {
      std::string name;
      if(n.getAttribute(expr::VarNameAttr(), name)) {
        out << name;
      } else {
        goto default_case;
      }
      break;
    }
    case kind::BITVECTOR_EXTRACT:
      out << n[0] << n.getOperator().getConst<BitVectorExtract>();
      break;
    case kind::BITVECTOR_REPEAT:
      out << "BVREPEAT(" << n[0] << "," << n.getOperator() << ')';
      break;
    case kind::BITVECTOR_ZERO_EXTEND:
      out << "BVZEROEXTEND(" << n[0] << "," << n.getOperator() << ')';
      break;
    case kind::BITVECTOR_SIGN_EXTEND:
      out << "SX(" << n[0] << "," << n.getOperator() << ')';
      break;
    case kind::BITVECTOR_ROTATE_LEFT:
      out << "BVROTL(" << n[0] << "," << n.getOperator() << ')';
      break;
    case kind::BITVECTOR_ROTATE_RIGHT:
      out << "BVROTR(" << n[0] << "," << n.getOperator() << ')';
      break;

    default:
    default_case:
      out << n.getOperator();
      if(n.getNumChildren() > 0) {
        out << '(';
        if(n.getNumChildren() > 1) {
          copy(n.begin(), n.end() - 1, ostream_iterator<TNode>(out, ", "));
        }
        out << n[n.getNumChildren() - 1] << ')';
      }
    }
    return;
  } else if(n.getMetaKind() == kind::metakind::OPERATOR) {
    switch(Kind k = n.getKind()) {
    case kind::FUNCTION_TYPE:
    case kind::CONSTRUCTOR_TYPE:
    case kind::SELECTOR_TYPE:
      if(n.getNumChildren() > 2) {
        out << '(';
        for(unsigned i = 0; i < n.getNumChildren() - 2; ++i) {
          out << n[i] << ", ";
        }
        out << n[n.getNumChildren() - 2]
            << ") -> " << n[n.getNumChildren() - 1];
      } else if(n.getNumChildren() == 2) {
        out << n[0] << " -> " << n[1];
      } else {
        Assert(n.getNumChildren() == 1);
        out << n[0];
      }
      break;
    case kind::TESTER_TYPE:
      out << n[0] << " -> BOOLEAN";
      break;

    case kind::ARRAY_TYPE:
      out << "ARRAY " << n[0] << " OF " << n[1];
      break;
    case kind::SELECT:
      out << n[0] << '[' << n[1] << ']';
      break;
    case kind::STORE: {
      stack<TNode> stk;
      stk.push(n);
      while(stk.top()[0].getKind() == kind::STORE) {
        stk.push(stk.top()[0]);
      }
      out << '(';
      TNode x = stk.top();
      out << x[0] << " WITH [" << x[1] << "] := " << x[2];
      stk.pop();
      while(!stk.empty()) {
        x = stk.top();
        out << ", [" << x[1] << "] := " << x[2];
        stk.pop();
      }
      out << ')';
      break;
    }

    case kind::TUPLE_TYPE:
      out << '[';
      copy(n.begin(), n.end() - 1, ostream_iterator<TNode>(out, ","));
      out << n[n.getNumChildren() - 1];
      out << ']';
      break;
    case kind::TUPLE:
      out << "( ";
      copy(n.begin(), n.end() - 1, ostream_iterator<TNode>(out, ", "));
      out << n[n.getNumChildren() - 1];
      out << " )";
      break;

    case kind::ITE:
      out << "IF " << n[0] << " THEN " << n[1] << " ELSE " << n[2];
      break;

    // these are prefix and have an extra size 'k' as first argument
    case kind::BITVECTOR_PLUS:
    case kind::BITVECTOR_SUB:
    case kind::BITVECTOR_MULT:
      out << n.getOperator() << '(' << n.getType().getBitVectorSize() << ','
          << n[0] << ',' << n[1] << ')';
      break;

    // these are prefix
    case kind::BITVECTOR_XOR:
    case kind::BITVECTOR_NAND:
    case kind::BITVECTOR_NOR:
    case kind::BITVECTOR_XNOR:
    case kind::BITVECTOR_COMP:
    case kind::BITVECTOR_UDIV:
    case kind::BITVECTOR_UREM:
    case kind::BITVECTOR_SDIV:
    case kind::BITVECTOR_SREM:
    case kind::BITVECTOR_SMOD:
    case kind::BITVECTOR_SHL:
    case kind::BITVECTOR_LSHR:
    case kind::BITVECTOR_ASHR:
    case kind::BITVECTOR_ULT:
    case kind::BITVECTOR_ULE:
    case kind::BITVECTOR_UGT:
    case kind::BITVECTOR_UGE:
    case kind::BITVECTOR_SLT:
    case kind::BITVECTOR_SLE:
    case kind::BITVECTOR_SGT:
    case kind::BITVECTOR_SGE:
      out << n.getOperator() << '(' << n[0] << ',' << n[1] << ')';
      break;

    // N-ary infix
    case kind::BITVECTOR_CONCAT:
      out << '(';
      for(unsigned i = 0; i < n.getNumChildren() - 1; ++i) {
        out << n[i] << ' ' << n.getOperator();
      }
      out << n[n.getNumChildren() - 1] << ')';

    default:
      if(k == kind::NOT && n[0].getKind() == kind::EQUAL) {
        // collapse NOT-over-EQUAL
        out << n[0][0] << " /= " << n[0][1];
      } else if(n.getNumChildren() == 2) {
        // infix binary operator
        out << '(' << n[0] << ' ' << n.getOperator() << ' ' << n[1] << ')';
      } else if(n.getKind() == kind::AND ||
                n.getKind() == kind::OR ||
                n.getKind() == kind::PLUS ||
                n.getKind() == kind::MULT) {
        // infix N-ary operator
        TNode::iterator i = n.begin();
        out << '(' << *i++;
        while(i != n.end()) {
          out << ' ' << n.getOperator() << ' ' << *i++;
        }
        out << ')';
      } else if(k == kind::BITVECTOR_NOT) {
        // precedence on ~ is a little unexpected; add parens
        out << "(~(" << n[0] << "))";
      } else {
        // prefix N-ary operator for N != 2
        if(n.getNumChildren() == 0) {
          // no parens or spaces
          out << n.getOperator();
        } else {
          out << '(' << n.getOperator() << ' ';
          if(n.getNumChildren() > 1) {
            copy(n.begin(), n.end() - 1, ostream_iterator<TNode>(out, " "));
          }
          out << n[n.getNumChildren() - 1] << ')';
        }
      }
    }
    return;
  }

  // shouldn't ever happen (unless new metakinds are added)
  Unhandled();

}/* CvcPrinter::toStream(TNode) */

template <class T>
static bool tryToStream(std::ostream& out, const Command* c) throw();

void CvcPrinter::toStream(std::ostream& out, const Command* c,
                           int toDepth, bool types) const throw() {
  expr::ExprSetDepth::Scope sdScope(out, toDepth);
  expr::ExprPrintTypes::Scope ptScope(out, types);

  if(tryToStream<AssertCommand>(out, c) ||
     tryToStream<PushCommand>(out, c) ||
     tryToStream<PopCommand>(out, c) ||
     tryToStream<CheckSatCommand>(out, c) ||
     tryToStream<QueryCommand>(out, c) ||
     tryToStream<QuitCommand>(out, c) ||
     tryToStream<DeclarationSequence>(out, c) ||
     tryToStream<CommandSequence>(out, c) ||
     tryToStream<DeclareFunctionCommand>(out, c) ||
     tryToStream<DefineFunctionCommand>(out, c) ||
     tryToStream<DeclareTypeCommand>(out, c) ||
     tryToStream<DefineTypeCommand>(out, c) ||
     tryToStream<DefineNamedFunctionCommand>(out, c) ||
     tryToStream<SimplifyCommand>(out, c) ||
     tryToStream<GetValueCommand>(out, c) ||
     tryToStream<GetAssignmentCommand>(out, c) ||
     tryToStream<GetAssertionsCommand>(out, c) ||
     tryToStream<SetBenchmarkStatusCommand>(out, c) ||
     tryToStream<SetBenchmarkLogicCommand>(out, c) ||
     tryToStream<SetInfoCommand>(out, c) ||
     tryToStream<GetInfoCommand>(out, c) ||
     tryToStream<SetOptionCommand>(out, c) ||
     tryToStream<GetOptionCommand>(out, c) ||
     tryToStream<DatatypeDeclarationCommand>(out, c) ||
     tryToStream<CommentCommand>(out, c) ||
     tryToStream<EmptyCommand>(out, c)) {
    return;
  }

  out << "ERROR: don't know how to print a Command of class: "
      << typeid(*c).name() << endl;

}/* CvcPrinter::toStream(Command*) */

template <class T>
static bool tryToStream(std::ostream& out, const CommandStatus* s) throw();

void CvcPrinter::toStream(std::ostream& out, const CommandStatus* s) const throw() {

  if(tryToStream<CommandSuccess>(out, s) ||
     tryToStream<CommandFailure>(out, s) ||
     tryToStream<CommandUnsupported>(out, s)) {
    return;
  }

  out << "ERROR: don't know how to print a CommandStatus of class: "
      << typeid(*s).name() << endl;

}/* CvcPrinter::toStream(CommandStatus*) */

static void toStream(std::ostream& out, const AssertCommand* c) throw() {
  out << "ASSERT " << c->getExpr() << ";";
}

static void toStream(std::ostream& out, const PushCommand* c) throw() {
  out << "PUSH;";
}

static void toStream(std::ostream& out, const PopCommand* c) throw() {
  out << "POP;";
}

static void toStream(std::ostream& out, const CheckSatCommand* c) throw() {
  BoolExpr e = c->getExpr();
  if(!e.isNull()) {
    out << "CHECKSAT " << e << ";";
  } else {
    out << "CHECKSAT;";
  }
}

static void toStream(std::ostream& out, const QueryCommand* c) throw() {
  BoolExpr e = c->getExpr();
  if(!e.isNull()) {
    out << "QUERY " << e << ";";
  } else {
    out << "QUERY TRUE;";
  }
}

static void toStream(std::ostream& out, const QuitCommand* c) throw() {
  //out << "EXIT;";
}

static void toStream(std::ostream& out, const CommandSequence* c) throw() {
  for(CommandSequence::const_iterator i = c->begin();
      i != c->end();
      ++i) {
    out << *i << endl;
  }
}

static void toStream(std::ostream& out, const DeclarationSequence* c) throw() {
  DeclarationSequence::const_iterator i = c->begin();
  for(;;) {
    DeclarationDefinitionCommand* dd =
      static_cast<DeclarationDefinitionCommand*>(*i++);
    if(i != c->end()) {
      out << dd->getSymbol() << ", ";
    } else {
      out << *dd;
      break;
    }
  }
}

static void toStream(std::ostream& out, const DeclareFunctionCommand* c) throw() {
  out << c->getSymbol() << " : " << c->getType() << ";";
}

static void toStream(std::ostream& out, const DefineFunctionCommand* c) throw() {
  Expr func = c->getFunction();
  const vector<Expr>& formals = c->getFormals();
  Expr formula = c->getFormula();
  out << func << " : " << func.getType() << " = ";
  if(formals.size() > 0) {
    out << "LAMBDA(";
    vector<Expr>::const_iterator i = formals.begin();
    while(i != formals.end()) {
      out << (*i) << ":" << (*i).getType();
      if(++i != formals.end()) {
        out << ", ";
      }
    }
    out << "): ";
  }
  out << formula << ";";
}

static void toStream(std::ostream& out, const DeclareTypeCommand* c) throw() {
  if(c->getArity() > 0) {
    out << "ERROR: Don't know how to print parameterized type declaration "
           "in CVC language:" << endl << c->toString() << endl;
  } else {
    out << c->getSymbol() << " : TYPE;";
  }
}

static void toStream(std::ostream& out, const DefineTypeCommand* c) throw() {
  if(c->getParameters().size() > 0) {
    out << "ERROR: Don't know how to print parameterized type definition "
           "in CVC language:" << endl << c->toString() << endl;
  } else {
    out << c->getSymbol() << " : TYPE = " << c->getType() << ";";
  }
}

static void toStream(std::ostream& out, const DefineNamedFunctionCommand* c) throw() {
  toStream(out, static_cast<const DefineFunctionCommand*>(c));
}

static void toStream(std::ostream& out, const SimplifyCommand* c) throw() {
  out << "TRANSFORM " << c->getTerm() << ";";
}

static void toStream(std::ostream& out, const GetValueCommand* c) throw() {
  out << "% (get-value " << c->getTerm() << ")";
}

static void toStream(std::ostream& out, const GetAssignmentCommand* c) throw() {
  out << "% (get-assignment)";
}

static void toStream(std::ostream& out, const GetAssertionsCommand* c) throw() {
  out << "% (get-assertions)";
}

static void toStream(std::ostream& out, const SetBenchmarkStatusCommand* c) throw() {
  out << "% (set-info :status " << c->getStatus() << ")";
}

static void toStream(std::ostream& out, const SetBenchmarkLogicCommand* c) throw() {
  out << "% (set-logic " << c->getLogic() << ")";
}

static void toStream(std::ostream& out, const SetInfoCommand* c) throw() {
  out << "% (set-info " << c->getFlag() << " " << c->getSExpr() << ")";
}

static void toStream(std::ostream& out, const GetInfoCommand* c) throw() {
  out << "% (get-info " << c->getFlag() << ")";
}

static void toStream(std::ostream& out, const SetOptionCommand* c) throw() {
  out << "% (set-option " << c->getFlag() << " " << c->getSExpr() << ")";
}

static void toStream(std::ostream& out, const GetOptionCommand* c) throw() {
  out << "% (get-option " << c->getFlag() << ")";
}

static void toStream(std::ostream& out, const DatatypeDeclarationCommand* c) throw() {
  const vector<DatatypeType>& datatypes = c->getDatatypes();
  for(vector<DatatypeType>::const_iterator i = datatypes.begin(),
        i_end = datatypes.end();
      i != i_end;
      ++i) {
    out << *i;
  }
}

static void toStream(std::ostream& out, const CommentCommand* c) throw() {
  out << "% " << c->getComment();
}

static void toStream(std::ostream& out, const EmptyCommand* c) throw() {
}

template <class T>
static bool tryToStream(std::ostream& out, const Command* c) throw() {
  if(typeid(*c) == typeid(T)) {
    toStream(out, dynamic_cast<const T*>(c));
    return true;
  }
  return false;
}

static void toStream(std::ostream& out, const CommandSuccess* s) throw() {
  if(Command::printsuccess::getPrintSuccess(out)) {
    out << "OK" << endl;
  }
}

static void toStream(std::ostream& out, const CommandUnsupported* s) throw() {
  out << "UNSUPPORTED" << endl;
}

static void toStream(std::ostream& out, const CommandFailure* s) throw() {
  out << s->getMessage() << endl;
}

template <class T>
static bool tryToStream(std::ostream& out, const CommandStatus* s) throw() {
  if(typeid(*s) == typeid(T)) {
    toStream(out, dynamic_cast<const T*>(s));
    return true;
  }
  return false;
}

}/* CVC4::printer::cvc namespace */
}/* CVC4::printer namespace */
}/* CVC4 namespace */