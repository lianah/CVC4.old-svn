(benchmark fuzzsmt
:logic QF_AX
:status unsat
:extrafuns ((v0 Array))
:extrafuns ((v1 Array))
:extrafuns ((v2 Index))
:extrafuns ((v3 Element))
:extrafuns ((v4 Element))
:formula
(flet ($e5 (distinct v0 v0))
(flet ($e6 (distinct v0 v1))
(flet ($e7 (distinct v2 v2))
(flet ($e8 (distinct v3 v4))
(let (?e9 (ite $e5 v1 v1))
(let (?e10 (ite $e7 ?e9 v0))
(let (?e11 (ite $e6 ?e10 v0))
(let (?e12 (ite $e8 v1 v0))
(let (?e13 (ite $e8 v2 v2))
(let (?e14 (ite $e7 ?e13 ?e13))
(let (?e15 (ite $e5 ?e13 ?e14))
(let (?e16 (ite $e7 v2 ?e13))
(let (?e17 (ite $e6 v2 ?e13))
(let (?e18 (ite $e7 v3 v4))
(let (?e19 (ite $e8 ?e18 v3))
(let (?e20 (ite $e6 v3 v4))
(let (?e21 (ite $e8 ?e19 v4))
(let (?e22 (ite $e6 ?e20 ?e18))
(let (?e23 (ite $e5 ?e22 ?e21))
(let (?e24 (store ?e9 ?e17 ?e23))
(let (?e25 (select ?e11 ?e15))
(flet ($e26 (= ?e11 ?e12))
(flet ($e27 (= ?e11 ?e11))
(flet ($e28 (= ?e12 ?e11))
(flet ($e29 (= v0 ?e9))
(flet ($e30 (= ?e9 v1))
(flet ($e31 (distinct ?e10 v1))
(flet ($e32 (= ?e12 ?e24))
(flet ($e33 (distinct ?e17 ?e14))
(flet ($e34 (distinct v2 ?e17))
(flet ($e35 (= ?e13 v2))
(flet ($e36 (distinct ?e15 ?e16))
(flet ($e37 (= v4 ?e22))
(flet ($e38 (distinct v4 ?e18))
(flet ($e39 (= ?e22 ?e23))
(flet ($e40 (= ?e23 ?e20))
(flet ($e41 (distinct ?e21 ?e22))
(flet ($e42 (= ?e21 ?e23))
(flet ($e43 (distinct ?e23 v4))
(flet ($e44 (distinct v4 v3))
(flet ($e45 (= ?e21 ?e19))
(flet ($e46 (= ?e22 ?e22))
(flet ($e47 (distinct ?e20 ?e25))
(flet ($e48 (not $e7))
(flet ($e49 (not $e33))
(flet ($e50 (iff $e35 $e39))
(flet ($e51 (xor $e28 $e42))
(flet ($e52 (xor $e31 $e38))
(flet ($e53 (implies $e46 $e36))
(flet ($e54 (or $e50 $e37))
(flet ($e55 (or $e52 $e5))
(flet ($e56 (and $e34 $e55))
(flet ($e57 (iff $e47 $e8))
(flet ($e58 (implies $e48 $e44))
(flet ($e59 (iff $e45 $e57))
(flet ($e60 (iff $e41 $e29))
(flet ($e61 (if_then_else $e58 $e40 $e27))
(flet ($e62 (iff $e53 $e59))
(flet ($e63 (if_then_else $e32 $e26 $e54))
(flet ($e64 (xor $e51 $e60))
(flet ($e65 (iff $e62 $e6))
(flet ($e66 (implies $e64 $e49))
(flet ($e67 (or $e63 $e65))
(flet ($e68 (if_then_else $e30 $e66 $e61))
(flet ($e69 (or $e56 $e68))
(flet ($e70 (and $e69 $e43))
(flet ($e71 (iff $e67 $e67))
(flet ($e72 (and $e70 $e71))
$e72
)))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))
