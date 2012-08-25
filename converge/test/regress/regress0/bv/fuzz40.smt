(benchmark fuzzsmt
:logic QF_BV
:status unsat
:extrafuns ((v0 BitVec[4]))
:extrafuns ((v1 BitVec[10]))
:extrafuns ((v2 BitVec[15]))
:formula
(let (?e3 bv1[3])
(let (?e4 (ite (bvsgt (sign_extend[7] ?e3) v1) bv1[1] bv0[1]))
(let (?e5 (bvmul (zero_extend[1] ?e3) v0))
(let (?e6 (extract[3:0] ?e5))
(let (?e7 (bvneg ?e3))
(let (?e8 (bvudiv (zero_extend[6] v0) v1))
(let (?e9 (extract[9:4] v1))
(let (?e10 (bvor ?e8 (zero_extend[9] ?e4)))
(let (?e11 (rotate_right[0] ?e4))
(let (?e12 (bvxor ?e10 (zero_extend[7] ?e7)))
(let (?e13 (bvneg ?e11))
(let (?e14 (ite (bvsle (sign_extend[7] ?e7) v1) bv1[1] bv0[1]))
(let (?e15 (bvand (zero_extend[11] ?e6) v2))
(flet ($e16 (bvsgt ?e8 (zero_extend[6] v0)))
(flet ($e17 (bvuge (zero_extend[12] ?e7) v2))
(flet ($e18 (= v2 (sign_extend[14] ?e13)))
(flet ($e19 (bvule (sign_extend[14] ?e13) ?e15))
(flet ($e20 (bvule (sign_extend[6] ?e6) ?e12))
(flet ($e21 (bvugt (sign_extend[9] ?e9) v2))
(flet ($e22 (bvslt (sign_extend[6] ?e6) ?e8))
(flet ($e23 (bvult v2 (zero_extend[11] v0)))
(flet ($e24 (bvsgt ?e8 (sign_extend[9] ?e13)))
(flet ($e25 (bvsgt (zero_extend[4] ?e9) ?e12))
(flet ($e26 (bvugt (zero_extend[12] ?e7) ?e15))
(flet ($e27 (bvslt v2 (zero_extend[14] ?e11)))
(flet ($e28 (bvult (sign_extend[5] ?e13) ?e9))
(flet ($e29 (= ?e8 (sign_extend[9] ?e11)))
(flet ($e30 (bvult ?e15 ?e15))
(flet ($e31 (bvult ?e15 (zero_extend[14] ?e4)))
(flet ($e32 (bvsge (zero_extend[7] ?e7) v1))
(flet ($e33 (bvuge (sign_extend[2] ?e6) ?e9))
(flet ($e34 (bvslt (zero_extend[2] ?e14) ?e7))
(flet ($e35 (bvsge ?e6 (zero_extend[3] ?e4)))
(flet ($e36 (bvsgt ?e10 v1))
(flet ($e37 (bvult ?e10 ?e10))
(flet ($e38 (bvslt v2 (sign_extend[14] ?e11)))
(flet ($e39 (bvule v0 (zero_extend[3] ?e14)))
(flet ($e40 (bvult (sign_extend[9] ?e13) ?e10))
(flet ($e41 (bvsgt v1 (sign_extend[7] ?e3)))
(flet ($e42 (bvule ?e9 (sign_extend[2] ?e5)))
(flet ($e43 (and $e17 $e39))
(flet ($e44 (not $e43))
(flet ($e45 (or $e23 $e44))
(flet ($e46 (xor $e16 $e25))
(flet ($e47 (if_then_else $e29 $e22 $e45))
(flet ($e48 (if_then_else $e19 $e37 $e18))
(flet ($e49 (implies $e46 $e35))
(flet ($e50 (iff $e48 $e48))
(flet ($e51 (iff $e28 $e24))
(flet ($e52 (xor $e20 $e51))
(flet ($e53 (xor $e47 $e42))
(flet ($e54 (and $e32 $e41))
(flet ($e55 (iff $e31 $e21))
(flet ($e56 (and $e54 $e36))
(flet ($e57 (and $e56 $e40))
(flet ($e58 (xor $e57 $e34))
(flet ($e59 (not $e58))
(flet ($e60 (xor $e55 $e53))
(flet ($e61 (not $e52))
(flet ($e62 (and $e38 $e33))
(flet ($e63 (implies $e50 $e49))
(flet ($e64 (and $e59 $e61))
(flet ($e65 (or $e26 $e60))
(flet ($e66 (if_then_else $e62 $e65 $e64))
(flet ($e67 (not $e30))
(flet ($e68 (implies $e63 $e66))
(flet ($e69 (xor $e27 $e68))
(flet ($e70 (not $e67))
(flet ($e71 (iff $e70 $e70))
(flet ($e72 (xor $e71 $e71))
(flet ($e73 (or $e72 $e72))
(flet ($e74 (and $e69 $e69))
(flet ($e75 (xor $e73 $e73))
(flet ($e76 (and $e74 $e75))
(flet ($e77 (and $e76 (not (= v1 bv0[10]))))
$e77
))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))

