(benchmark fuzzsmt
:logic QF_BV
:extrafuns ((v13 BitVec[16]))
:extrafuns ((v9 BitVec[14]))
:extrafuns ((v11 BitVec[13]))
:extrafuns ((v3 BitVec[11]))
:extrafuns ((v8 BitVec[9]))
:extrafuns ((v4 BitVec[14]))
:status unknown
:formula
(let (?n1 bv0[1])
(let (?n2 bv1[16])
(let (?n3 (sign_extend[2] v4))
(let (?n4 (bvshl ?n2 ?n3))
(let (?n5 (extract[5:2] v8))
(let (?n6 (sign_extend[9] ?n5))
(let (?n7 (bvxnor ?n6 v11))
(let (?n8 (sign_extend[3] ?n7))
(let (?n9 (bvnand ?n4 ?n8))
(let (?n10 (bvneg ?n9))
(let (?n11 bv0[16])
(flet ($n12 (bvugt ?n4 ?n11))
(let (?n13 bv1[1])
(let (?n14 (ite $n12 ?n13 ?n1))
(let (?n15 (zero_extend[8] ?n14))
(let (?n16 (extract[13:5] v9))
(let (?n17 (bvashr ?n15 ?n16))
(let (?n18 (zero_extend[7] ?n17))
(let (?n19 (bvsub ?n10 ?n18))
(flet ($n20 (distinct ?n2 ?n19))
(let (?n21 (ite $n20 ?n13 ?n1))
(flet ($n22 (= ?n1 ?n21))
(flet ($n23 (not $n22))
(let (?n24 (sign_extend[1] v11))
(flet ($n25 (bvugt ?n24 v4))
(let (?n26 (ite $n25 ?n13 ?n1))
(let (?n27 bv21[8])
(let (?n28 (zero_extend[1] ?n27))
(flet ($n29 (bvuge ?n28 v8))
(let (?n30 (ite $n29 ?n13 ?n1))
(flet ($n31 (bvugt ?n26 ?n30))
(let (?n32 (ite $n31 ?n13 ?n1))
(let (?n33 (sign_extend[14] ?n32))
(let (?n34 (sign_extend[2] v11))
(let (?n35 (bvand ?n33 ?n34))
(let (?n36 bv0[15])
(flet ($n37 (bvslt ?n35 ?n36))
(let (?n38 (ite $n37 ?n13 ?n1))
(let (?n39 (sign_extend[3] ?n38))
(flet ($n40 (bvsle ?n5 ?n39))
(flet ($n41 false)
(let (?n42 bv0[14])
(flet ($n43 (bvslt v4 ?n42))
(let (?n44 (ite $n43 ?n13 ?n1))
(let (?n45 (zero_extend[7] v8))
(let (?n46 (bvand ?n45 v13))
(let (?n47 (bvsub ?n2 ?n46))
(let (?n48 (sign_extend[7] v8))
(flet ($n49 (= ?n47 ?n48))
(let (?n50 (ite $n49 ?n13 ?n1))
(let (?n51 (zero_extend[8] ?n50))
(let (?n52 bv1[9])
(let (?n53 (bvnor ?n52 ?n52))
(let (?n54 (bvsub ?n51 ?n53))
(let (?n55 (zero_extend[6] ?n54))
(let (?n56 (bvshl ?n35 ?n55))
(let (?n57 (zero_extend[1] ?n56))
(flet ($n58 (distinct ?n11 ?n57))
(let (?n59 (ite $n58 ?n13 ?n1))
(let (?n60 (bvcomp ?n44 ?n59))
(let (?n61 (zero_extend[13] ?n60))
(flet ($n62 (bvult ?n42 ?n61))
(flet ($n63 (bvsgt ?n46 ?n45))
(let (?n64 (ite $n63 ?n13 ?n1))
(let (?n65 (sign_extend[8] ?n64))
(let (?n66 (sign_extend[8] ?n13))
(let (?n67 (bvadd ?n65 ?n66))
(let (?n68 bv0[9])
(flet ($n69 (= ?n67 ?n68))
(flet ($n70 (or $n41 $n62 $n69))
(let (?n71 (zero_extend[2] v9))
(flet ($n72 (bvsle ?n71 ?n46))
(let (?n73 (ite $n72 ?n13 ?n1))
(flet ($n74 (= ?n1 ?n73))
(let (?n75 bv1[13])
(flet ($n76 (= ?n7 ?n75))
(let (?n77 (ite $n76 ?n13 ?n1))
(let (?n78 (sign_extend[10] ?n77))
(flet ($n79 (bvsge ?n78 v3))
(let (?n80 (ite $n79 ?n13 ?n1))
(let (?n81 (zero_extend[15] ?n80))
(flet ($n82 (bvsge ?n81 ?n11))
(let (?n83 (bvxnor v11 ?n75))
(let (?n84 (zero_extend[3] ?n83))
(let (?n85 bv1[14])
(flet ($n86 (bvsgt ?n85 v9))
(let (?n87 (ite $n86 ?n13 ?n1))
(flet ($n88 (= ?n13 ?n87))
(let (?n89 (ite $n88 v13 ?n11))
(let (?n90 (bvxnor ?n84 ?n89))
(flet ($n91 (distinct ?n2 ?n90))
(flet ($n92 (not $n91))
(flet ($n93 (and $n23 $n40 $n70 $n74 $n82 $n92))
$n93
))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))
