(benchmark fuzzsmt
:logic QF_BV
:status unknown
:extrafuns ((v0 BitVec[4]))
:extrafuns ((v1 BitVec[4]))
:extrafuns ((v2 BitVec[4]))
:formula
(let (?e3 bv1[4])
(let (?e4 bv11[4])
(let (?e5 bv2[4])
(let (?e6 (ite (= bv1[1] (extract[3:3] v2)) v1 ?e3))
(let (?e7 (bvshl v1 ?e3))
(let (?e8 (repeat[1] ?e5))
(let (?e9 (ite (bvslt v2 v2) bv1[1] bv0[1]))
(let (?e10 (repeat[1] ?e6))
(let (?e11 (ite (bvuge ?e8 ?e8) bv1[1] bv0[1]))
(let (?e12 (bvadd (zero_extend[3] ?e11) v2))
(let (?e13 (bvshl ?e3 v1))
(let (?e14 (bvnot ?e13))
(let (?e15 (ite (bvuge ?e12 ?e4) bv1[1] bv0[1]))
(let (?e16 (ite (bvslt ?e10 ?e10) bv1[1] bv0[1]))
(let (?e17 (ite (bvsge ?e8 (sign_extend[3] ?e16)) bv1[1] bv0[1]))
(let (?e18 (sign_extend[0] ?e12))
(let (?e19 (bvlshr v1 ?e18))
(let (?e20 (ite (bvuge ?e17 ?e9) bv1[1] bv0[1]))
(let (?e21 (bvsub ?e14 (sign_extend[3] ?e11)))
(let (?e22 (repeat[1] ?e13))
(let (?e23 (bvcomp v2 ?e22))
(let (?e24 (ite (bvslt ?e7 ?e13) bv1[1] bv0[1]))
(let (?e25 (bvor (sign_extend[3] ?e9) v1))
(let (?e26 (bvashr v2 (sign_extend[3] ?e15)))
(let (?e27 (zero_extend[0] v1))
(let (?e28 (bvcomp ?e10 ?e12))
(let (?e29 (ite (bvsle (sign_extend[3] ?e28) ?e5) bv1[1] bv0[1]))
(let (?e30 (ite (bvsge (zero_extend[3] ?e24) ?e12) bv1[1] bv0[1]))
(let (?e31 (bvnor ?e25 ?e18))
(let (?e32 (bvneg ?e11))
(let (?e33 (bvmul ?e12 v2))
(let (?e34 (sign_extend[0] ?e4))
(let (?e35 (ite (bvuge (zero_extend[3] ?e15) ?e26) bv1[1] bv0[1]))
(let (?e36 (ite (bvslt ?e8 (zero_extend[3] ?e32)) bv1[1] bv0[1]))
(let (?e37 (ite (distinct ?e27 (zero_extend[3] ?e17)) bv1[1] bv0[1]))
(let (?e38 (bvxor v2 (sign_extend[3] ?e28)))
(let (?e39 (ite (bvsle (zero_extend[3] ?e23) ?e25) bv1[1] bv0[1]))
(let (?e40 (bvor (sign_extend[3] ?e36) ?e8))
(let (?e41 (repeat[1] ?e18))
(let (?e42 (bvneg ?e28))
(let (?e43 (extract[0:0] ?e29))
(let (?e44 (rotate_right[2] ?e41))
(let (?e45 (ite (= bv1[1] (extract[0:0] ?e21)) ?e30 ?e36))
(let (?e46 (bvnand ?e26 ?e18))
(let (?e47 (bvmul (zero_extend[3] ?e36) ?e44))
(let (?e48 (bvnor ?e38 v0))
(flet ($e49 (distinct ?e38 ?e8))
(flet ($e50 (bvule ?e10 ?e40))
(flet ($e51 (bvult (zero_extend[3] ?e17) ?e18))
(flet ($e52 (bvslt (zero_extend[3] ?e45) ?e19))
(flet ($e53 (bvsge ?e8 ?e31))
(flet ($e54 (distinct ?e21 ?e5))
(flet ($e55 (bvsgt ?e11 ?e45))
(flet ($e56 (bvult ?e33 ?e8))
(flet ($e57 (bvuge ?e17 ?e43))
(flet ($e58 (bvugt ?e12 (zero_extend[3] ?e16)))
(flet ($e59 (= ?e27 (zero_extend[3] ?e20)))
(flet ($e60 (= ?e7 (sign_extend[3] ?e32)))
(flet ($e61 (= ?e12 ?e14))
(flet ($e62 (distinct ?e20 ?e45))
(flet ($e63 (distinct ?e25 ?e38))
(flet ($e64 (bvsgt ?e48 ?e6))
(flet ($e65 (bvsgt ?e32 ?e9))
(flet ($e66 (bvsge ?e10 ?e10))
(flet ($e67 (bvuge ?e31 (sign_extend[3] ?e30)))
(flet ($e68 (bvugt ?e3 v0))
(flet ($e69 (bvsle ?e8 (sign_extend[3] ?e42)))
(flet ($e70 (bvule ?e44 ?e46))
(flet ($e71 (bvsgt (zero_extend[3] ?e24) ?e14))
(flet ($e72 (bvsgt ?e22 (sign_extend[3] ?e20)))
(flet ($e73 (distinct ?e11 ?e43))
(flet ($e74 (bvsgt ?e31 (zero_extend[3] ?e42)))
(flet ($e75 (bvsgt ?e14 (zero_extend[3] ?e29)))
(flet ($e76 (bvult (zero_extend[3] ?e20) ?e46))
(flet ($e77 (bvult (sign_extend[3] ?e37) ?e25))
(flet ($e78 (bvsle ?e44 ?e3))
(flet ($e79 (bvsge (sign_extend[3] ?e39) ?e38))
(flet ($e80 (bvsge ?e31 (zero_extend[3] ?e11)))
(flet ($e81 (bvsge ?e13 ?e6))
(flet ($e82 (bvsgt ?e26 (sign_extend[3] ?e24)))
(flet ($e83 (bvsgt ?e31 ?e19))
(flet ($e84 (distinct (zero_extend[3] ?e23) ?e13))
(flet ($e85 (bvsle ?e28 ?e29))
(flet ($e86 (bvuge ?e46 (sign_extend[3] ?e16)))
(flet ($e87 (bvugt ?e42 ?e30))
(flet ($e88 (bvsge ?e10 (zero_extend[3] ?e24)))
(flet ($e89 (bvsge ?e4 ?e46))
(flet ($e90 (bvslt ?e20 ?e39))
(flet ($e91 (bvsgt (zero_extend[3] ?e16) ?e7))
(flet ($e92 (= ?e19 (zero_extend[3] ?e23)))
(flet ($e93 (bvuge ?e47 ?e26))
(flet ($e94 (bvsgt ?e40 ?e7))
(flet ($e95 (bvsgt ?e11 ?e9))
(flet ($e96 (bvult ?e26 ?e18))
(flet ($e97 (= ?e40 ?e41))
(flet ($e98 (bvslt v0 ?e14))
(flet ($e99 (bvsgt v0 (zero_extend[3] ?e24)))
(flet ($e100 (bvule (zero_extend[3] ?e11) ?e33))
(flet ($e101 (= ?e22 ?e21))
(flet ($e102 (bvult ?e4 ?e44))
(flet ($e103 (bvsgt ?e31 ?e34))
(flet ($e104 (bvuge ?e12 v2))
(flet ($e105 (bvult ?e24 ?e29))
(flet ($e106 (bvslt ?e48 ?e33))
(flet ($e107 (bvult (zero_extend[3] ?e30) ?e13))
(flet ($e108 (= ?e4 ?e25))
(flet ($e109 (bvsle ?e15 ?e28))
(flet ($e110 (bvslt ?e32 ?e32))
(flet ($e111 (bvule (zero_extend[3] ?e39) ?e38))
(flet ($e112 (bvugt ?e40 ?e12))
(flet ($e113 (bvsge (sign_extend[3] ?e30) v1))
(flet ($e114 (bvugt (sign_extend[3] ?e39) ?e4))
(flet ($e115 (bvsle ?e26 (zero_extend[3] ?e37)))
(flet ($e116 (bvult ?e18 ?e33))
(flet ($e117 (bvsge ?e11 ?e37))
(flet ($e118 (distinct ?e48 (sign_extend[3] ?e29)))
(flet ($e119 (bvule ?e42 ?e15))
(flet ($e120 (distinct (zero_extend[3] ?e20) v1))
(flet ($e121 (bvsge (zero_extend[3] ?e23) ?e44))
(flet ($e122 (bvsle ?e44 ?e21))
(flet ($e123 (bvugt ?e13 ?e14))
(flet ($e124 (bvslt ?e7 (sign_extend[3] ?e35)))
(flet ($e125 (bvule ?e13 (zero_extend[3] ?e20)))
(flet ($e126 (bvugt ?e7 (sign_extend[3] ?e28)))
(flet ($e127 (bvule ?e12 ?e38))
(flet ($e128 (bvuge (zero_extend[3] ?e42) ?e22))
(flet ($e129 (bvult ?e6 ?e44))
(flet ($e130 (bvsge (zero_extend[3] ?e45) ?e40))
(flet ($e131 (bvslt ?e25 ?e6))
(flet ($e132 (distinct ?e19 (zero_extend[3] ?e30)))
(flet ($e133 (bvugt (zero_extend[3] ?e37) ?e34))
(flet ($e134 (bvsgt ?e33 ?e22))
(flet ($e135 (distinct (sign_extend[3] ?e16) ?e25))
(flet ($e136 (distinct (zero_extend[3] ?e42) ?e34))
(flet ($e137 (bvslt ?e23 ?e9))
(flet ($e138 (bvule v0 (zero_extend[3] ?e16)))
(flet ($e139 (bvsgt ?e5 ?e27))
(flet ($e140 (bvult ?e35 ?e43))
(flet ($e141 (bvsle (sign_extend[3] ?e29) ?e18))
(flet ($e142 (distinct (zero_extend[3] ?e39) ?e33))
(flet ($e143 (distinct ?e5 ?e8))
(flet ($e144 (distinct ?e28 ?e17))
(flet ($e145 (distinct (zero_extend[3] ?e17) ?e14))
(flet ($e146 (bvuge (sign_extend[3] ?e37) ?e14))
(flet ($e147 (bvult ?e44 (sign_extend[3] ?e11)))
(flet ($e148 (bvule (zero_extend[3] ?e35) ?e44))
(flet ($e149 (= ?e4 (zero_extend[3] ?e28)))
(flet ($e150 (bvule ?e8 v2))
(flet ($e151 (bvsle (zero_extend[3] ?e37) ?e6))
(flet ($e152 (bvuge ?e36 ?e43))
(flet ($e153 
(and
 (or (not $e74) $e58 $e57)
 (or $e119 (not $e138) $e95)
 (or (not $e106) $e63 (not $e56))
 (or $e126 $e64 (not $e137))
 (or (not $e101) (not $e109) $e59)
 (or (not $e132) (not $e57) (not $e54))
 (or $e61 (not $e104) (not $e66))
 (or (not $e50) $e133 $e83)
 (or $e53 $e151 (not $e113))
 (or $e104 (not $e75) (not $e82))
 (or (not $e74) $e56 $e101)
 (or $e149 (not $e83) (not $e61))
 (or (not $e88) (not $e122) (not $e122))
 (or $e98 $e114 $e109)
 (or $e85 $e60 $e130)
 (or (not $e61) (not $e118) $e75)
 (or (not $e107) $e138 $e55)
 (or $e81 $e129 (not $e88))
 (or $e102 $e129 $e97)
 (or (not $e99) (not $e130) (not $e89))
 (or (not $e53) (not $e49) (not $e107))
 (or $e152 (not $e134) (not $e58))
 (or (not $e141) $e120 (not $e104))
 (or (not $e138) (not $e101) $e148)
 (or (not $e64) $e78 $e50)
 (or $e98 (not $e152) $e74)
 (or (not $e53) (not $e146) (not $e114))
 (or $e113 (not $e105) (not $e133))
 (or $e89 $e92 (not $e115))
 (or $e119 (not $e91) (not $e129))
 (or $e138 (not $e101) (not $e88))
 (or $e125 (not $e111) $e135)
 (or $e131 (not $e114) $e134)
 (or $e126 $e136 (not $e122))
 (or (not $e131) (not $e143) (not $e68))
 (or $e113 (not $e141) $e101)
 (or $e80 (not $e110) $e102)
 (or $e138 (not $e102) (not $e145))
 (or $e81 $e78 (not $e65))
 (or (not $e94) $e74 (not $e54))
 (or (not $e145) $e49 (not $e49))
 (or $e126 $e52 $e80)
 (or (not $e127) $e71 (not $e86))
 (or (not $e140) $e70 $e129)
 (or $e137 $e55 (not $e108))
 (or (not $e57) $e93 (not $e142))
 (or $e56 (not $e116) $e65)
 (or (not $e151) $e51 $e50)
 (or $e55 (not $e132) (not $e145))
 (or $e83 $e148 $e79)
 (or (not $e141) (not $e68) (not $e76))
 (or (not $e107) (not $e63) (not $e99))
))
$e153
))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))

