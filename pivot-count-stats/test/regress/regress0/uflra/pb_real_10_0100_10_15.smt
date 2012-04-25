(benchmark mathsat
:source { MathSat group }
:logic QF_UFLRA
:status sat 
:category { random } 
:difficulty { 3 }
:extrafuns ((f0_1 Real Real))
:extrafuns ((f0_2 Real Real Real))
:extrafuns ((f0_3 Real Real Real Real))
:extrafuns ((f0_4 Real Real Real Real Real))
:extrafuns ((f1_1 Real Real))
:extrafuns ((f1_2 Real Real Real))
:extrafuns ((f1_3 Real Real Real Real))
:extrafuns ((f1_4 Real Real Real Real Real))
:extrafuns ((x0 Real))
:extrafuns ((x1 Real))
:extrafuns ((x2 Real))
:extrafuns ((x3 Real))
:extrafuns ((x4 Real))
:extrafuns ((x5 Real))
:extrafuns ((x6 Real))
:extrafuns ((x7 Real))
:extrafuns ((x8 Real))
:extrafuns ((x9 Real))
:extrapreds ((P0))
:extrapreds ((P1))
:extrapreds ((P2))
:extrapreds ((P3))
:extrapreds ((P4))
:extrapreds ((P5))
:extrapreds ((P6))
:extrapreds ((P7))
:extrapreds ((P8))
:extrapreds ((P9))
:formula
(let (?x10 (f1_1 x4))
(let (?x11 (f0_2 x0 x0))
(let (?x12 (- (- (* (- 0 22) x3) (* 6 x9)) (* 14 x6)))
(let (?x13 (+ (- (* (- 0 19) x2) (* 15 ?x10)) (* 12 x4)))
(let (?x14 (f0_1 x7))
(let (?x15 (f1_2 x5 x7))
(let (?x16 (f1_2 x8 x8))
(let (?x17 (f1_2 x0 x9))
(let (?x18 (f1_2 ?x10 x3))
(let (?x19 (f0_2 x7 x9))
(let (?x20 (f0_2 x7 x9))
(let (?x21 (- (- (* (- 0 6) x7) (* 5 x6)) (* 9 x2)))
(let (?x22 (f0_2 x5 x5))
(let (?x23 (f1_2 x9 x0))
(let (?x24 (f0_1 x7))
(let (?x25 (f0_1 x5))
(let (?x26 (f0_1 x7))
(let (?x27 (+ (- (* 13 x6) (* 27 x1)) (* 4 x7)))
(let (?x28 (f0_1 x8))
(let (?x29 (f1_2 x3 x0))
(let (?x30 (+ (+ (* (- 0 10) x7) (* 23 x1)) (* 4 x4)))
(let (?x31 (+ (+ (* (- 0 4) x7) (* 6 x7)) (* 2 x1)))
(let (?x32 (f1_2 x0 x7))
(let (?x33 (f1_1 x9))
(let (?x34 (f1_2 ?x23 ?x25))
(let (?x35 (f1_1 x5))
(let (?x36 (f1_2 x7 x4))
(let (?x37 (f0_2 x0 x7))
(let (?x38 (+ (+ (* 25 ?x36) (* 20 x2)) (* 23 ?x27)))
(let (?x39 (f1_2 x2 x1))
(let (?x40 (+ (- (* 29 ?x11) (* 29 ?x32)) (* 20 ?x15)))
(let (?x41 (f1_2 ?x33 ?x18))
(let (?x42 (+ (+ (* (- 0 13) ?x21) (* 22 ?x23)) (* 9 x6)))
(let (?x43 (f1_1 x9))
(let (?x44 (f0_1 x9))
(let (?x45 (+ (+ (* 20 ?x15) (* 4 ?x34)) (* 22 ?x22)))
(let (?x46 (- (- (* (- 0 14) x7) (* 21 x6)) (* 21 x2)))
(let (?x47 (f0_2 x1 x7))
(let (?x48 (f1_1 x3))
(let (?x49 (f1_1 x2))
(flet ($P10 (< ?x35 (- 0 29)))
(flet ($P11 (< ?x38 21))
(flet ($P12 (< ?x28 (- 0 6)))
(flet ($P13 (= ?x27 ?x12))
(flet ($P14 (= ?x41 ?x32))
(flet ($P15 (< x0 (- 0 9)))
(flet ($P16 (< x0 (- 0 3)))
(flet ($P17 (< ?x25 26))
(flet ($P18 (< ?x13 2))
(flet ($P19 (< ?x30 (- 0 29)))
(flet ($P20 (< ?x33 (- 0 26)))
(flet ($P21 (< ?x17 0))
(flet ($P22 (< x5 0))
(flet ($P23 (< ?x42 0))
(flet ($P24 (< ?x27 (- 0 13)))
(flet ($P25 (< ?x22 28))
(flet ($P26 (< ?x26 (- 0 6)))
(flet ($P27 (< ?x24 11))
(flet ($P28 (< ?x41 6))
(flet ($P29 (< ?x18 (- 0 23)))
(flet ($P30 (= ?x31 ?x26))
(flet ($P31 (< ?x19 (- 0 10)))
(flet ($P32 (< ?x47 19))
(flet ($P33 (= ?x22 ?x26))
(flet ($P34 (< ?x31 (- 0 1)))
(flet ($P35 (< ?x16 6))
(flet ($P36 (< x0 18))
(flet ($P37 (< x8 28))
(flet ($P38 (< ?x27 21))
(flet ($P39 (< ?x32 12))
(flet ($P40 (= ?x32 ?x27))
(flet ($P41 (< ?x31 16))
(flet ($P42 (< ?x47 15))
(flet ($P43 (< ?x36 8))
(flet ($P44 (< ?x16 13))
(flet ($P45 (< ?x23 19))
(flet ($P46 (< ?x10 13))
(flet ($P47 (= ?x48 ?x38))
(flet ($P48 (< ?x20 (- 0 7)))
(flet ($P49 (< ?x21 (- 0 10)))
(flet ($P50 (< ?x33 (- 0 7)))
(flet ($P51 (= ?x32 ?x41))
(flet ($P52 (< ?x45 18))
(flet ($P53 (< ?x38 (- 0 13)))
(flet ($P54 (< x1 21))
(flet ($P55 (= ?x31 ?x30))
(flet ($P56 (< ?x11 28))
(flet ($P57 (< ?x48 (- 0 12)))
(flet ($P58 (= x3 ?x28))
(flet ($P59 (< x6 (- 0 19)))
(and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (or (or (not P8) (not $P18)) $P23) (or (or $P24 $P37) $P41)) (or (or (not $P29) (not $P37)) (not P2))) (or (or $P41 (not P3)) P0)) (or (or $P15 (not $P36)) (not $P41))) (or (or (not $P21) $P58) P1)) (or (or (not $P58) $P31) $P19)) (or (or $P39 (not $P18)) (not $P53))) (or (or (not $P47) P4) (not P2))) (or (or (not P7) (not $P26)) $P37)) (or (or $P19 (not $P51)) (not $P33))) (or (or (not $P15) (not P3)) $P52)) (or (or $P41 $P12) (not P9))) (or (or $P53 (not $P32)) (not $P34))) (or (or (not $P13) (not $P31)) (not $P16))) (or (or $P25 $P34) $P28)) (or (or $P28 (not $P25)) (not $P14))) (or (or $P38 $P56) (not $P36))) (or (or (not $P23) $P42) (not $P50))) (or (or (not $P43) $P53) $P34)) (or (or (not $P46) $P28) (not $P30))) (or (or (not $P50) $P39) $P17)) (or (or $P51 (not P1)) $P15)) (or (or P8 (not $P47)) (not $P45))) (or (or P1 (not P7)) $P29)) (or (or (not $P37) (not $P28)) (not $P11))) (or (or (not $P22) $P39) $P21)) (or (or $P28 $P20) (not $P16))) (or (or (not $P49) $P29) $P41)) (or (or $P42 (not $P13)) (not $P41))) (or (or (not $P25) $P29) (not $P56))) (or (or $P37 $P38) $P15)) (or (or (not P3) (not $P47)) $P57)) (or (or $P50 $P27) (not $P48))) (or (or (not P3) (not P2)) $P52)) (or (or $P18 (not P9)) (not $P41))) (or (or (not P3) $P44) (not $P56))) (or (or $P50 $P11) P4)) (or (or (not $P50) $P53) P1)) (or (or $P55 (not $P39)) (not $P58))) (or (or $P13 (not $P51)) $P50)) (or (or (not P5) $P31) $P24)) (or (or $P40 $P15) $P42)) (or (or $P35 $P23) (not $P30))) (or (or $P58 $P32) (not $P35))) (or (or $P40 (not $P33)) $P45)) (or (or (not $P28) P6) $P16)) (or (or P1 $P48) P6)) (or (or P4 P2) (not $P38))) (or (or (not $P31) P1) $P29)) (or (or (not $P49) $P56) $P31)) (or (or (not $P24) $P22) $P49)) (or (or $P38 (not $P45)) (not $P30))) (or (or P2 (not $P52)) (not $P55))) (or (or (not P6) (not P3)) P2)) (or (or $P37 $P28) $P47)) (or (or $P21 (not $P27)) (not $P24))) (or (or $P40 P6) (not $P56))) (or (or $P12 (not $P21)) $P51)) (or (or (not $P14) (not P4)) $P58)) (or (or $P20 $P23) (not $P14))) (or (or (not P7) $P21) $P35)) (or (or (not $P46) (not $P48)) $P16)) (or (or (not $P25) P7) $P54)) (or (or (not $P44) (not $P35)) (not $P51))) (or (or (not $P15) (not $P44)) $P46)) (or (or (not $P49) $P13) P5)) (or (or (not $P40) P2) (not $P21))) (or (or $P44 (not $P10)) (not $P18))) (or (or (not $P48) P6) $P21)) (or (or (not $P22) (not $P30)) (not $P57))) (or (or (not $P16) P1) $P28)) (or (or (not $P33) (not $P58)) (not $P55))) (or (or $P17 (not $P23)) $P34)) (or (or P8 (not $P57)) $P44)) (or (or $P26 $P31) (not $P48))) (or (or $P34 (not P4)) (not $P24))) (or (or (not $P39) $P48) (not $P37))) (or (or $P40 P4) $P38)) (or (or $P44 $P49) (not $P32))) (or (or $P26 (not $P25)) $P43)) (or (or (not P0) (not $P53)) (not $P42))) (or (or (not $P17) (not $P57)) (not $P26))) (or (or $P59 (not $P58)) (not $P50))) (or (or (not $P30) $P41) $P40)) (or (or (not $P13) (not $P43)) (not $P45))) (or (or (not $P27) (not $P17)) $P25)) (or (or P8 $P25) (not $P48))) (or (or $P10 (not $P23)) $P34)) (or (or $P36 (not $P55)) (not $P40))) (or (or $P52 (not P2)) $P18)) (or (or $P49 $P12) $P45)) (or (or P6 (not $P43)) (not P0))) (or (or $P56 (not $P38)) (not $P55))) (or (or (not $P41) (not P8)) (not $P21))) (or (or (not $P42) $P38) (not $P41))) (or (or (not $P46) (not $P12)) $P17)) (or (or $P58 (not $P13)) P2)) (or (or (not $P12) (not $P48)) (not P8))) (or (or $P31 $P32) $P57)))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))