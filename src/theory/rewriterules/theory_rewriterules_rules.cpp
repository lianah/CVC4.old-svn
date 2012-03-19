/*********************                                                       */
/*! \file rewrite_engine.cpp
 ** \verbatim
 ** Original author: ajreynolds
 ** Major contributors: bobot
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010, 2011
 ** The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief [[ Deals with rewrite rules ]]
 **
 ** [[ Add lengthier description here ]]
 ** \todo document this file
 **/

#include "theory/rewriterules/theory_rewriterules_rules.h"
#include "theory/rewriterules/theory_rewriterules_params.h"


using namespace std;
using namespace CVC4;
using namespace CVC4::kind;
using namespace CVC4::context;
using namespace CVC4::theory;
using namespace CVC4::theory::rewriterules;


namespace CVC4 {
namespace theory {
namespace rewriterules {

void TheoryRewriteRules::computeMatchBody ( const RewriteRule & rule,
                                            size_t start){
  std::vector<TNode> stack(1,rule.new_terms);

  while(!stack.empty()){
    Node t = stack.back(); stack.pop_back();

    // We don't want to consider variable in t
    if( std::find(rule.free_vars.begin(), rule.free_vars.end(), t)
        != rule.free_vars.end()) continue;
    // t we want to consider only UF function
    if( t.getKind() == APPLY_UF ){
      for(size_t rid = start, end = d_rules.size(); rid < end; ++rid) {
        const RewriteRule & r = d_rules[rid];
        Trigger & tr = const_cast<Trigger &> (r.trigger_for_body_match);
        if(!tr.nonunifiable(t, rule.free_vars)){
          /** modify a context depend structure so const_cast ok*/
          const_cast<RewriteRule &>(rule).body_match
            .push_back(std::make_pair(t,rid));
        }
      }
    }

    //put the children on the stack
    for( size_t i=0; i < t.getNumChildren(); i++ ){
      stack.push_back(t[i]);
    };

  }
}

inline void addPattern(TheoryRewriteRules & re,
                       TNode tri,
                       std::vector<Node> & pattern,
                       std::vector<Node> & vars,
                       std::vector<Node> & inst_constants,
                       TNode r){
  if (tri.getKind() == kind::NOT) tri = tri[0];
  pattern.push_back(re.getQuantifiersEngine()->
                    convertNodeToPattern(tri,r,vars,inst_constants));
}

void checkPatternVarsAux(TNode pat,const std::vector<Node> & vars,
                         std::vector<bool> & seen){
  for(size_t id=0;id < vars.size(); ++id){
    if(pat == vars[id]){
      seen[id]=true;
      break;
    };
  };
  for(Node::iterator i = pat.begin(); i != pat.end(); ++i) {
    checkPatternVarsAux(*i,vars,seen);
  };
}

bool checkPatternVars(const std::vector<Node> & pattern,
                      const std::vector<Node> & vars){
  std::vector<bool> seen(vars.size(),false);
  for(std::vector<Node>::const_iterator i = pattern.begin();
      i != pattern.end(); ++i) {
    checkPatternVarsAux(*i,vars,seen);
  };
  return (find(seen.begin(),seen.end(),false) == seen.end());
}

void TheoryRewriteRules::addRewriteRule(const Node r)
{
  Assert(r.getKind() == kind::REWRITE_RULE);

  Debug("rewriterules") << "create rewriterule:" << r << std::endl;
  /*   Replace variables by Inst_* variable and tag the terms that
       contain them */
  std::vector<Node> vars;
  vars.reserve(r[0].getNumChildren());
  for( Node::const_iterator v = r[0].begin(); v != r[0].end(); ++v ){
    vars.push_back(*v);
  };
  /* Instantiation version */
  std::vector<Node> inst_constants =
    getQuantifiersEngine()->createInstVariable(vars);
  /* Body/Remove_term/Guards/Triggers */
  Node body = r[2][1];
  TNode new_terms = r[2][1];
  std::vector<Node> guards;
  std::vector<Node> pattern;
  std::vector<Node> to_remove;  /* remove the terms from the database
                                   when fired */
  /* shortcut */
  TNode head = r[2][0];
  switch(r[2].getKind()){
  case kind::RR_REWRITE:
    /* Equality */
    to_remove.push_back(head);
    addPattern(*this,head,pattern,vars,inst_constants,r);
    body = head.eqNode(body);
    break;
  case kind::RR_REDUCTION:
    /** Add head to remove */
    to_remove.push_back(head);
  case kind::RR_DEDUCTION:
    /** Add head to guards and pattern */
    switch(head.getKind()){
    case kind::AND:
      guards.reserve(head.getNumChildren());
      for(Node::iterator i = head.begin(); i != head.end(); ++i) {
        guards.push_back(*i);
        addPattern(*this,*i,pattern,vars,inst_constants,r);
      };
      break;
    default:
      if (head != d_true){
        guards.push_back(head);
        addPattern(*this,head,pattern,vars,inst_constants,r);
      };
      /** otherwise guards is empty */
    };
    break;
  default:
    Unreachable("RewriteRules can be of only threee kinds");
  };
  /* Add the other guards */
  TNode g = r[1];
  switch(g.getKind()){
  case kind::AND:
    guards.reserve(g.getNumChildren());
    for(Node::iterator i = g.begin(); i != g.end(); ++i) {
      guards.push_back(*i);
    };
    break;
  default:
    if (g != d_true) guards.push_back(g);
    /** otherwise guards is empty */
  };
  /* Add the other triggers */
  if( r[2].getNumChildren() >= 3 )
  if (!disableAdditionnalTrigger ||
      (pattern.size() == 0 && r[2][2].getNumChildren() == 1) )
    for(Node::iterator i = r[2][2].begin(); i != r[2][2].end(); ++i) {
      // todo test during typing that its a good term (no not, atom, or term...)
      addPattern(*this,(*i)[0],pattern,vars,inst_constants,r);
    };
  Assert(pattern.size() == 1, "currently only single pattern are supported");
  //Every variable must be seen in the pattern
  if (!checkPatternVars(pattern,inst_constants)){
    Warning() << "The rule" << r <<
      " has been removed since it doesn't contain every variables."
              << std::endl;
  }else{
  // final construction
  Trigger trigger = createTrigger(r,pattern);
  Trigger trigger2 = createTrigger(r,pattern); //Hack
  RewriteRule rr = RewriteRule(*this, trigger, trigger2,
                               guards, body, new_terms,
                               vars, inst_constants, to_remove);
  /** other -> rr */
  if(compute_opt) computeMatchBody(rr);
  d_rules.push_back(rr);
  /** rr -> all (including himself) */
  if(compute_opt)
    for(size_t rid = 0, end = d_rules.size(); rid < end; ++rid)
      computeMatchBody(d_rules[rid],
                       d_rules.size() - 1);

  }
};



RewriteRule::RewriteRule(TheoryRewriteRules & re,
                         Trigger & tr, Trigger & tr2,
                         std::vector<Node> & g, Node b, TNode nt,
                         std::vector<Node> & fv,std::vector<Node> & iv,
                         std::vector<Node> & to_r) :
  trigger(tr), body(b), new_terms(nt), free_vars(), inst_vars(),
  body_match(),trigger_for_body_match(tr2),
  d_cache(re.getContext()){
  free_vars.swap(fv); inst_vars.swap(iv); guards.swap(g); to_remove.swap(to_r);
};


bool RewriteRule::inCache(std::vector<Node> & subst)const{
  /* INST_PATTERN because its 1: */
  NodeBuilder<> nodeb(kind::INST_PATTERN);
  for(std::vector<Node>::const_iterator p = subst.begin();
      p != subst.end(); ++p){
    if (rewrite_before_cache) nodeb << Rewriter::rewrite(*p);
    else nodeb << *p;
  };
  Node node = nodeb;
  CacheNode::const_iterator e = d_cache.find(node);
  /* Already in the cache */
  if( e != d_cache.end() ) return true;
  /* Because we add only context dependent data and the const is for that */
  RewriteRule & rule = const_cast<RewriteRule &>(*this);
  rule.d_cache.insert(node);
  return false;
};

/** A rewrite rule */
void RewriteRule::toStream(std::ostream& out) const{
  for(std::vector<Node>::const_iterator
        iter = guards.begin(); iter != guards.end(); ++iter){
    out << *iter;
  };
  out << "=>" << body << std::endl;
  out << "[";
  for(BodyMatch::const_iterator
        iter = body_match.begin(); iter != body_match.end(); ++iter){
    out << (*iter).first << "(" << (*iter).second << ")" << ",";
  };
  out << "]" << std::endl;
}


}/* CVC4::theory::rewriterules namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */
