(benchmark fuzzsmt
:logic QF_UF
:extrasorts (S1)
:extrasorts (S0)
:extrafuns ((v0 S0))
:extrafuns ((v1 S0))
:extrafuns ((f4 S1 S1))
:extrafuns ((v2 S1))
:extrafuns ((v3 S1))
:extrapreds ((p3 S0 S0 S1))
:extrafuns ((f0 S0 S0 S1 S0))
:extrapreds ((p1 S0 S1 S1))
:extrapreds ((p4 S1))
:status unknown
:formula
(flet ($n1 (p4 v3))
(let (?n2 (f4 v2))
(flet ($n3 (p4 ?n2))
(flet ($n4 (p1 v0 ?n2 ?n2))
(let (?n5 (f0 v1 v1 v2))
(let (?n6 (ite $n4 ?n5 v0))
(flet ($n7 (p3 ?n6 v0 ?n2))
(flet ($n8 (iff $n3 $n7))
(flet ($n9 (= v3 ?n2))
(let (?n10 (ite $n9 v1 v0))
(flet ($n11 (distinct ?n10 ?n10))
(flet ($n12 false)
(flet ($n13 (implies $n11 $n12))
(flet ($n14 (implies $n13 $n12))
(flet ($n15 (if_then_else $n1 $n8 $n14))
$n15
))))))))))))))))