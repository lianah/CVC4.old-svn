(benchmark fuzzsmt
:logic QF_BV
:status sat
:extrafuns ((v0 BitVec[12]))
:extrafuns ((v1 BitVec[2]))
:extrafuns ((v2 BitVec[13]))
:extrafuns ((v3 BitVec[10]))
:formula
(let (?e4 bv47[10])
(let (?e5 (bvshl (zero_extend[8] v1) v3))
(let (?e6 (bvcomp v2 (zero_extend[11] v1)))
(let (?e7 (ite (bvslt ?e5 ?e4) bv1[1] bv0[1]))
(let (?e8 (ite (bvsgt ?e4 (zero_extend[9] ?e6)) bv1[1] bv0[1]))
(let (?e9 (bvadd v1 (zero_extend[1] ?e6)))
(let (?e10 (ite (bvsle (zero_extend[9] ?e7) ?e4) bv1[1] bv0[1]))
(let (?e11 (rotate_left[0] ?e8))
(let (?e12 (ite (= bv1[1] (extract[0:0] ?e7)) (zero_extend[1] ?e8) ?e9))
(let (?e13 (bvadd ?e9 ?e9))
(let (?e14 (rotate_right[0] ?e7))
(let (?e15 (ite (bvslt (sign_extend[9] ?e6) ?e5) bv1[1] bv0[1]))
(let (?e16 (ite (distinct ?e7 ?e6) bv1[1] bv0[1]))
(let (?e17 (bvor ?e7 ?e8))
(let (?e18 (bvand (zero_extend[1] ?e6) ?e9))
(let (?e19 (ite (bvuge ?e18 (sign_extend[1] ?e16)) bv1[1] bv0[1]))
(let (?e20 (ite (bvsgt ?e18 (sign_extend[1] ?e17)) bv1[1] bv0[1]))
(let (?e21 (ite (= (sign_extend[3] v3) v2) bv1[1] bv0[1]))
(let (?e22 (bvxor ?e10 ?e19))
(let (?e23 (bvxor (zero_extend[1] ?e19) v1))
(let (?e24 (ite (bvule v1 (sign_extend[1] ?e19)) bv1[1] bv0[1]))
(let (?e25 (ite (bvuge ?e8 ?e17) bv1[1] bv0[1]))
(let (?e26 (ite (bvsge ?e20 ?e22) bv1[1] bv0[1]))
(let (?e27 (zero_extend[1] ?e22))
(let (?e28 (bvsub (sign_extend[1] ?e10) v1))
(let (?e29 (bvlshr ?e27 (sign_extend[1] ?e17)))
(let (?e30 (concat ?e17 ?e4))
(let (?e31 (bvnand (zero_extend[11] ?e23) v2))
(let (?e32 (bvashr (zero_extend[9] ?e19) ?e5))
(let (?e33 (bvmul ?e23 (zero_extend[1] ?e25)))
(let (?e34 (repeat[1] ?e27))
(let (?e35 (ite (bvuge ?e27 (zero_extend[1] ?e11)) bv1[1] bv0[1]))
(let (?e36 (ite (= (sign_extend[1] ?e11) v1) bv1[1] bv0[1]))
(let (?e37 (bvneg ?e31))
(let (?e38 (ite (bvsgt ?e22 ?e14) bv1[1] bv0[1]))
(let (?e39 (ite (bvsge ?e36 ?e8) bv1[1] bv0[1]))
(let (?e40 (bvand ?e8 ?e14))
(let (?e41 (bvand (zero_extend[12] ?e17) v2))
(let (?e42 (ite (bvsle ?e35 ?e26) bv1[1] bv0[1]))
(let (?e43 (ite (bvsle v2 (sign_extend[12] ?e21)) bv1[1] bv0[1]))
(let (?e44 (bvshl ?e16 ?e42))
(let (?e45 (ite (= ?e23 ?e29) bv1[1] bv0[1]))
(let (?e46 (repeat[1] ?e41))
(let (?e47 (bvcomp ?e10 ?e21))
(let (?e48 (ite (= (zero_extend[9] ?e47) ?e4) bv1[1] bv0[1]))
(let (?e49 (bvnand (zero_extend[11] ?e33) ?e41))
(let (?e50 (ite (bvslt ?e44 ?e20) bv1[1] bv0[1]))
(let (?e51 (ite (bvsgt ?e31 (sign_extend[12] ?e21)) bv1[1] bv0[1]))
(let (?e52 (ite (bvslt v0 (zero_extend[11] ?e38)) bv1[1] bv0[1]))
(flet ($e53 (= v1 ?e29))
(flet ($e54 (= (zero_extend[1] ?e44) ?e13))
(flet ($e55 (= ?e17 ?e44))
(flet ($e56 (= ?e4 (zero_extend[9] ?e39)))
(flet ($e57 (= (sign_extend[11] ?e14) v0))
(flet ($e58 (= (zero_extend[12] ?e42) ?e37))
(flet ($e59 (= ?e40 ?e36))
(flet ($e60 (= ?e23 ?e33))
(flet ($e61 (= (zero_extend[1] ?e24) ?e29))
(flet ($e62 (= ?e28 (zero_extend[1] ?e50)))
(flet ($e63 (= ?e6 ?e25))
(flet ($e64 (= ?e49 (sign_extend[12] ?e21)))
(flet ($e65 (= ?e20 ?e38))
(flet ($e66 (= (zero_extend[1] ?e16) ?e18))
(flet ($e67 (= ?e50 ?e16))
(flet ($e68 (= ?e20 ?e48))
(flet ($e69 (= ?e17 ?e21))
(flet ($e70 (= (sign_extend[12] ?e44) ?e46))
(flet ($e71 (= ?e51 ?e47))
(flet ($e72 (= ?e5 (sign_extend[9] ?e16)))
(flet ($e73 (= ?e16 ?e21))
(flet ($e74 (= ?e5 (sign_extend[8] ?e27)))
(flet ($e75 (= (zero_extend[9] ?e21) ?e5))
(flet ($e76 (= (sign_extend[8] ?e28) ?e5))
(flet ($e77 (= (sign_extend[9] ?e10) ?e32))
(flet ($e78 (= ?e28 (sign_extend[1] ?e11)))
(flet ($e79 (= ?e29 (sign_extend[1] ?e17)))
(flet ($e80 (= ?e36 ?e15))
(flet ($e81 (= (sign_extend[11] ?e45) v0))
(flet ($e82 (= ?e27 (sign_extend[1] ?e40)))
(flet ($e83 (= ?e28 (zero_extend[1] ?e44)))
(flet ($e84 (= v2 (zero_extend[1] v0)))
(flet ($e85 (= ?e32 (sign_extend[9] ?e47)))
(flet ($e86 (= v3 (sign_extend[9] ?e24)))
(flet ($e87 (= ?e46 (sign_extend[12] ?e52)))
(flet ($e88 (= ?e46 ?e46))
(flet ($e89 (= v2 (sign_extend[12] ?e20)))
(flet ($e90 (= v0 (sign_extend[10] ?e23)))
(flet ($e91 (= (zero_extend[9] ?e11) ?e4))
(flet ($e92 (= ?e52 ?e17))
(flet ($e93 (= v2 (zero_extend[12] ?e40)))
(flet ($e94 (= ?e35 ?e51))
(flet ($e95 (= ?e42 ?e10))
(flet ($e96 (= ?e47 ?e43))
(flet ($e97 (= (zero_extend[1] ?e11) v1))
(flet ($e98 (= (zero_extend[1] ?e32) ?e30))
(flet ($e99 (= ?e23 (zero_extend[1] ?e11)))
(flet ($e100 (= ?e44 ?e22))
(flet ($e101 (= ?e31 (sign_extend[12] ?e16)))
(flet ($e102 (= ?e32 (zero_extend[8] ?e27)))
(flet ($e103 (= (zero_extend[9] ?e45) ?e5))
(flet ($e104 (= ?e27 ?e13))
(flet ($e105 (= (zero_extend[1] ?e7) ?e13))
(flet ($e106 (= ?e33 (zero_extend[1] ?e26)))
(flet ($e107 (= ?e13 (zero_extend[1] ?e51)))
(flet ($e108 (= ?e32 (zero_extend[9] ?e40)))
(flet ($e109 (= ?e29 v1))
(flet ($e110 (= ?e35 ?e20))
(flet ($e111 (= ?e34 (sign_extend[1] ?e19)))
(flet ($e112 (= ?e52 ?e52))
(flet ($e113 (= (sign_extend[12] ?e40) ?e49))
(flet ($e114 (= ?e47 ?e47))
(flet ($e115 (= (zero_extend[12] ?e35) ?e46))
(flet ($e116 (= (sign_extend[11] ?e35) v0))
(flet ($e117 (= ?e4 (zero_extend[9] ?e38)))
(flet ($e118 (= v2 (sign_extend[12] ?e7)))
(flet ($e119 (= ?e47 ?e20))
(flet ($e120 (= (zero_extend[1] ?e11) ?e9))
(flet ($e121 (= ?e29 (zero_extend[1] ?e16)))
(flet ($e122 (= v0 (zero_extend[11] ?e24)))
(flet ($e123 (= ?e51 ?e36))
(flet ($e124 (= (sign_extend[8] ?e13) v3))
(flet ($e125 (= v1 (zero_extend[1] ?e45)))
(flet ($e126 (= (sign_extend[1] ?e48) ?e33))
(flet ($e127 (= ?e45 ?e7))
(flet ($e128 (= ?e26 ?e47))
(flet ($e129 (= (zero_extend[11] v1) ?e41))
(flet ($e130 (= (sign_extend[1] v0) v2))
(flet ($e131 (= ?e5 (sign_extend[9] ?e26)))
(flet ($e132 (= (sign_extend[11] ?e48) v0))
(flet ($e133 (= ?e38 ?e50))
(flet ($e134 (= ?e13 v1))
(flet ($e135 (= (sign_extend[1] ?e42) ?e23))
(flet ($e136 (= ?e20 ?e7))
(flet ($e137 (= ?e39 ?e19))
(flet ($e138 (= ?e52 ?e38))
(flet ($e139 (= ?e5 (zero_extend[9] ?e11)))
(flet ($e140 (= (sign_extend[11] ?e34) ?e49))
(flet ($e141 (= ?e18 ?e28))
(flet ($e142 (= ?e43 ?e26))
(flet ($e143 (= ?e14 ?e16))
(flet ($e144 (= ?e10 ?e40))
(flet ($e145 (= ?e6 ?e45))
(flet ($e146 (= ?e29 ?e18))
(flet ($e147 (= ?e5 (sign_extend[9] ?e48)))
(flet ($e148 (= (zero_extend[9] ?e19) v3))
(flet ($e149 (= ?e33 (sign_extend[1] ?e22)))
(flet ($e150 (= ?e50 ?e10))
(flet ($e151 (= (sign_extend[9] ?e35) v3))
(flet ($e152 (= ?e23 (zero_extend[1] ?e21)))
(flet ($e153 (= v2 (zero_extend[12] ?e15)))
(flet ($e154 (= v1 (sign_extend[1] ?e38)))
(flet ($e155 (= ?e23 (zero_extend[1] ?e22)))
(flet ($e156 (= ?e34 (zero_extend[1] ?e42)))
(flet ($e157 (= (zero_extend[9] ?e15) ?e4))
(flet ($e158 (= v3 (sign_extend[9] ?e8)))
(flet ($e159 (= (sign_extend[9] ?e14) ?e32))
(flet ($e160 (= (sign_extend[10] ?e13) v0))
(flet ($e161 (= (zero_extend[1] ?e35) ?e12))
(flet ($e162 (iff $e69 $e132))
(flet ($e163 (and $e62 $e124))
(flet ($e164 (iff $e152 $e133))
(flet ($e165 (not $e65))
(flet ($e166 (and $e159 $e57))
(flet ($e167 (and $e163 $e56))
(flet ($e168 (and $e129 $e90))
(flet ($e169 (if_then_else $e54 $e102 $e91))
(flet ($e170 (xor $e127 $e61))
(flet ($e171 (iff $e137 $e59))
(flet ($e172 (implies $e87 $e78))
(flet ($e173 (iff $e77 $e81))
(flet ($e174 (if_then_else $e170 $e161 $e171))
(flet ($e175 (if_then_else $e174 $e94 $e92))
(flet ($e176 (or $e168 $e55))
(flet ($e177 (and $e121 $e88))
(flet ($e178 (or $e167 $e131))
(flet ($e179 (or $e126 $e125))
(flet ($e180 (or $e151 $e79))
(flet ($e181 (not $e122))
(flet ($e182 (or $e112 $e83))
(flet ($e183 (or $e130 $e136))
(flet ($e184 (xor $e105 $e153))
(flet ($e185 (if_then_else $e148 $e128 $e66))
(flet ($e186 (iff $e109 $e154))
(flet ($e187 (and $e119 $e96))
(flet ($e188 (if_then_else $e179 $e64 $e140))
(flet ($e189 (xor $e134 $e123))
(flet ($e190 (implies $e97 $e165))
(flet ($e191 (or $e188 $e71))
(flet ($e192 (and $e145 $e185))
(flet ($e193 (or $e191 $e99))
(flet ($e194 (implies $e53 $e89))
(flet ($e195 (iff $e180 $e116))
(flet ($e196 (and $e84 $e117))
(flet ($e197 (or $e135 $e75))
(flet ($e198 (xor $e82 $e197))
(flet ($e199 (if_then_else $e142 $e195 $e177))
(flet ($e200 (implies $e181 $e73))
(flet ($e201 (or $e169 $e70))
(flet ($e202 (or $e150 $e106))
(flet ($e203 (xor $e110 $e115))
(flet ($e204 (or $e60 $e60))
(flet ($e205 (implies $e138 $e187))
(flet ($e206 (and $e194 $e156))
(flet ($e207 (not $e114))
(flet ($e208 (if_then_else $e147 $e63 $e118))
(flet ($e209 (not $e86))
(flet ($e210 (xor $e202 $e178))
(flet ($e211 (if_then_else $e210 $e58 $e100))
(flet ($e212 (implies $e113 $e166))
(flet ($e213 (iff $e120 $e141))
(flet ($e214 (and $e157 $e196))
(flet ($e215 (if_then_else $e72 $e199 $e68))
(flet ($e216 (not $e146))
(flet ($e217 (and $e215 $e95))
(flet ($e218 (not $e139))
(flet ($e219 (xor $e214 $e176))
(flet ($e220 (or $e175 $e211))
(flet ($e221 (if_then_else $e162 $e198 $e172))
(flet ($e222 (xor $e221 $e189))
(flet ($e223 (xor $e213 $e104))
(flet ($e224 (not $e206))
(flet ($e225 (and $e203 $e108))
(flet ($e226 (iff $e200 $e155))
(flet ($e227 (if_then_else $e111 $e208 $e144))
(flet ($e228 (implies $e227 $e85))
(flet ($e229 (not $e223))
(flet ($e230 (implies $e217 $e205))
(flet ($e231 (iff $e204 $e80))
(flet ($e232 (implies $e107 $e143))
(flet ($e233 (if_then_else $e209 $e220 $e209))
(flet ($e234 (or $e74 $e225))
(flet ($e235 (or $e103 $e103))
(flet ($e236 (implies $e67 $e234))
(flet ($e237 (xor $e232 $e182))
(flet ($e238 (xor $e190 $e235))
(flet ($e239 (or $e231 $e233))
(flet ($e240 (implies $e212 $e186))
(flet ($e241 (iff $e218 $e237))
(flet ($e242 (not $e238))
(flet ($e243 (xor $e193 $e240))
(flet ($e244 (implies $e228 $e222))
(flet ($e245 (not $e230))
(flet ($e246 (iff $e224 $e93))
(flet ($e247 (and $e158 $e207))
(flet ($e248 (if_then_else $e229 $e245 $e183))
(flet ($e249 (iff $e98 $e184))
(flet ($e250 (iff $e244 $e242))
(flet ($e251 (if_then_else $e149 $e101 $e250))
(flet ($e252 (xor $e236 $e173))
(flet ($e253 (xor $e248 $e226))
(flet ($e254 (and $e249 $e192))
(flet ($e255 (if_then_else $e246 $e254 $e247))
(flet ($e256 (if_then_else $e76 $e255 $e239))
(flet ($e257 (not $e256))
(flet ($e258 (or $e257 $e164))
(flet ($e259 (or $e243 $e258))
(flet ($e260 (iff $e219 $e216))
(flet ($e261 (xor $e160 $e201))
(flet ($e262 (not $e241))
(flet ($e263 (not $e252))
(flet ($e264 (not $e259))
(flet ($e265 (and $e261 $e251))
(flet ($e266 (if_then_else $e253 $e260 $e253))
(flet ($e267 (implies $e264 $e264))
(flet ($e268 (if_then_else $e266 $e266 $e265))
(flet ($e269 (iff $e263 $e267))
(flet ($e270 (if_then_else $e262 $e269 $e269))
(flet ($e271 (and $e270 $e270))
(flet ($e272 (not $e271))
(flet ($e273 (xor $e272 $e272))
(flet ($e274 (iff $e273 $e268))
$e274
))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))
