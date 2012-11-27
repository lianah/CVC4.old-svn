; EXPECT: unsat
; EXIT: 20
(set-logic QF_UF)
(set-info :status sat)
(set-option :produce-models true)
(declare-fun f (Bool) Bool)
(declare-fun x () Bool)
(declare-fun y () Bool)
(declare-fun z () Bool)
(assert (or (f x) (f y) (f z)))
(assert (not (f false)))
(assert (not (f true)))
(check-sat)
;(get-value ((f true) (f false) (f x) (f y) (f z) x y z))
