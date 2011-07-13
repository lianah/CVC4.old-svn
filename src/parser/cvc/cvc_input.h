/*********************                                                        */
/*! \file cvc_input.h
 ** \verbatim
 ** Original author: cconway
 ** Major contributors: mdeters
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010, 2011  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief [[ Add file-specific comments here ]].
 **
 ** [[ Add file-specific comments here ]]
 **/

#include "cvc4parser_private.h"

#ifndef __CVC4__PARSER__CVC_INPUT_H
#define __CVC4__PARSER__CVC_INPUT_H

#include "parser/antlr_input.h"
#include "parser/cvc/generated/CvcLexer.h"
#include "parser/cvc/generated/CvcParser.h"

// extern void CvcParserSetAntlrParser(CVC4::parser::AntlrParser* newAntlrParser);

namespace CVC4 {

class Command;
class Expr;
class ExprManager;

namespace parser {

class CvcInput : public AntlrInput {
  /** The ANTLR3 CVC lexer for the input. */
  pCvcLexer d_pCvcLexer;

  /** The ANTLR3 CVC parser for the input. */
  pCvcParser d_pCvcParser;

public:

  /** Create an input.
   *
   * @param inputStream the input to parse
   */
  CvcInput(AntlrInputStream& inputStream);

  /** Destructor. Frees the lexer and the parser. */
  ~CvcInput();

  /** Get the language that this Input is reading. */
  InputLanguage getLanguage() const throw() {
    return language::input::LANG_CVC4;
  }

protected:

  /** Parse a command from the input. Returns <code>NULL</code> if there is
   * no command there to parse.
   *
   * @throws ParserException if an error is encountered during parsing.
   */
  Command* parseCommand() 
    throw(ParserException, TypeCheckingException, AssertionException);

  /** Parse an expression from the input. Returns a null <code>Expr</code>
   * if there is no expression there to parse.
   *
   * @throws ParserException if an error is encountered during parsing.
   */
  Expr parseExpr() 
    throw(ParserException, TypeCheckingException, AssertionException);

private:

  /** Initialize the class. Called from the constructors once the input stream
   * is initialized. */
  void init();

}; // class CvcInput

}/* CVC4::parser namespace */
}/* CVC4 namespace */

#endif /* __CVC4__PARSER__CVC_INPUT_H */
