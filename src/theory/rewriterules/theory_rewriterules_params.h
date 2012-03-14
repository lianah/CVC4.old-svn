/*********************                                                       */
/*! \file rewrite_engine.cpp
 ** \verbatim
 ** Original author: bobot
 ** Major contributors: none
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010, 2011
 ** The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Parameters for the rewrite rules theory
 **
 ** [[ Add lengthier description here ]]
 ** \todo document this file
 **/

static const bool propagate_as_lemma = true;
static const bool cache_match = true;
static const bool compute_opt = true;
static const bool rewrite_before_cache = true;
static const size_t checkSlowdown = 10;