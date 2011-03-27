/*********************                                                        */
/*! \file pickle.cpp
 ** \verbatim
 ** Original author: kshitij
 ** Major contributors: none
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010, 2011  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief [[ Add one-line brief description here ]]
 **
 ** [[ Add lengthier description here ]]
 ** \todo document this file
 **/

#include "expr/expr.h"
#include "expr/node.h"
#include "expr/expr_manager_scope.h"
#include "expr/variable_type_map.h"
#include "util/Assert.h"
#include "expr/kind.h"
#include "expr/metakind.h"

/* Steps
 *
 * Today's goals
 *
 * 1. Decide upon the format exactly
 *    > so-many-bytes-for-so-and-so
 *
 * 2. Pickler:
 *    > some sort of stream to which we can easily
 *      append stuff
 *    > etc.
 */

/* Format'
 *
 * Block size = 8 bits.
 *
 * First block: __CVC4__EXPR__NODE_VALUE__NBITS__KIND = 8 bits
 * 
 * If metakind of kind given by first block is
 * 
 * > Constants
 *   - Next block(s) tell the number of data blocks that hold
 *     the value of the constant
 *   - Required number of data blocks follow.
 * > Variables
 *   - Have the address of this node stored in the next 8 blocks
 * > Operators and Parameterized Operators
 *   - __CVC4__EXPR__NODE_VALUE__NBITS__NCHILDREN = 16 bits, 
 *     the number of children that this node has.
 *   - If parameterized a node follows representing the operator
 */

/* Format
 *
 * Block size = 64 bits. May be a block is made up of 8-bit
 * chunks?
 *
 * First 8 bits: __CVC4__EXPR__NODE_VALUE__NBITS__KIND = 8
 * 
 * If metakind of kind given by first section is
 * 
 * > Constants
 *   - Remaining bits of the block tell number of data blocks
 *     required. (i.e. 64-8=56 bits)
 *   - Required number of data blocks follow.
 * > Variables
 *   - In this block, nothing more to store really, maybe store
 *     the ID
 *   - Have the address of this node stored in the next block
 * > Operators and Parameterized Operators
 *   - Add child nodes (in which order?)
 *   - __CVC4__EXPR__NODE_VALUE__NBITS__NCHILDREN = 16 bits, 
 *     the number of blocks to follow.
 *   - Remaning 48 bits are empty
 *   - If parameterized a node follows representing the operator
 */


namespace CVC4{

namespace expr {
namespace pickle {

const unsigned NBITS_BLOCK = 64;
const unsigned NBITS_KIND = __CVC4__EXPR__NODE_VALUE__NBITS__KIND;
const unsigned NBITS_NCHILDREN = __CVC4__EXPR__NODE_VALUE__NBITS__NCHILDREN;
const unsigned NBITS_CONSTBLOCKS = 32;

struct BlockHeaderOperator {
  unsigned d_kind          : NBITS_KIND;
  unsigned d_nchildren     : NBITS_NCHILDREN;
  unsigned d_parameterized : 1;
  unsigned                 : NBITS_BLOCK - (NBITS_KIND + NBITS_NCHILDREN + 1);
  // WARNING the d_parameterized : 1 may actually take more bits, so the size
  // of an object may be more that NBITS_BLOCK.
};

struct BlockHeaderConstant {
  unsigned d_kind          : NBITS_KIND;
  unsigned d_constblocks   : NBITS_CONSTBLOCKS;
  unsigned                 : NBITS_BLOCK - (NBITS_KIND + NBITS_CONSTBLOCKS);
};

struct BlockHeaderVariable {
  unsigned d_kind          : NBITS_KIND;
  unsigned                 : NBITS_BLOCK - NBITS_KIND;
};

class Pickler {
private:
  
public:
  void operator << (const BlockHeaderConstant &bla) { }
  void operator << (const BlockHeaderOperator &bla) { }
  void operator << (const BlockHeaderVariable &bla) { }
};

void pickleNode(Pickler &p, TNode n)
{
  Kind k = n.getKind();
  kind::MetaKind m = metaKindOf(k);
  if(m == kind::metakind::CONSTANT) {
    BlockHeaderConstant blk;
    blk.d_kind = k;
    blk.d_constblocks = 0; // TODO: set this before appending
    p << blk;
    // TODO: Append constant
  } else if(m == kind::metakind::VARIABLE) {
    BlockHeaderVariable blk;
    blk.d_kind = k;
    p << blk;
  } else {
    for(TNode::iterator i = n.begin(), i_end = n.end(); i != i_end; ++i) {
      pickleNode(p, *i);
    }
    BlockHeaderOperator blk;
    blk.d_kind = k;
    blk.d_nchildren = n.getNumChildren();
    blk.d_parameterized = (m == kind::metakind::PARAMETERIZED ? 1 : 0);
    p << blk;
    if(m == kind::metakind::PARAMETERIZED) {
      pickleNode(p, n.getOperator());
    }
  }
}

} /* namespace pickle */
} /* namespace expr */

} /* namespace CVC4 */
