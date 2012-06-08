(benchmark fuzzsmt
:logic QF_BV
:extrafuns ((v1 BitVec[9]))
:extrafuns ((v2 BitVec[10]))
:extrafuns ((v0 BitVec[3]))
:status sat
:formula
(let (?n1 bv1[3])
(flet ($n2 (= ?n1 v0))
(let (?n3 bv0[9])
(let (?n4 bv1[1])
(let (?n5 (sign_extend[2] v2))
(let (?n6 (extract[9:9] ?n5))
(flet ($n7 (= ?n4 ?n6))
(let (?n8 (bvneg v1))
(let (?n9 bv1[11])
(let (?n10 (zero_extend[8] v0))
(flet ($n11 (bvsgt ?n9 ?n10))
(let (?n12 bv0[1])
(let (?n13 (ite $n11 ?n4 ?n12))
(let (?n14 (zero_extend[8] ?n13))
(let (?n15 (ite $n7 ?n8 ?n14))
(flet ($n16 (= ?n3 ?n15))
(let (?n17 bv1[12])
(let (?n18 (zero_extend[3] v1))
(flet ($n19 (bvult ?n17 ?n18))
(let (?n20 (ite $n19 ?n4 ?n12))
(let (?n21 (zero_extend[1] v1))
(let (?n22 (bvlshr v2 ?n21))
(let (?n23 (zero_extend[2] ?n22))
(let (?n24 bv0[12])
(flet ($n25 (= ?n23 ?n24))
(let (?n26 (ite $n25 ?n4 ?n12))
(flet ($n27 (= ?n20 ?n26))
(flet ($n28 (or $n16 $n27))
(let (?n29 (sign_extend[9] v0))
(flet ($n30 (= ?n24 ?n29))
(let (?n31 bv0[10])
(let (?n32 (rotate_left[3] ?n8))
(let (?n33 (zero_extend[1] ?n32))
(let (?n34 (bvmul ?n22 ?n33))
(let (?n35 (bvcomp ?n31 ?n34))
(flet ($n36 (= ?n4 ?n35))
(let (?n37 bv1[9])
(let (?n38 (bvadd v1 ?n37))
(let (?n39 (zero_extend[6] v0))
(flet ($n40 (bvsge ?n38 ?n39))
(let (?n41 (ite $n40 ?n4 ?n12))
(let (?n42 (bvnor ?n41 ?n41))
(flet ($n43 (= ?n4 ?n42))
(let (?n44 (ite $n43 ?n31 ?n22))
(flet ($n45 (= ?n31 ?n44))
(flet ($n46 (if_then_else $n30 $n36 $n45))
(flet ($n47 (xor $n28 $n46))
(flet ($n48 (implies $n2 $n47))
$n48
)))))))))))))))))))))))))))))))))))))))))))))))))