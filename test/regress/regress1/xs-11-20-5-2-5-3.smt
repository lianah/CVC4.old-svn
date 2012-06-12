(benchmark mathsat
:source { MathSat group }
:logic QF_UFLIA
:status unsat 
:category { industrial } 
:difficulty { 2 }
:extrafuns ((fmt1 Int))
:extrafuns ((fmt0 Int))
:extrafuns ((arg1 Int))
:extrafuns ((arg0 Int))
:extrafuns ((fmt_length Int))
:extrafuns ((distance Int))
:extrafuns ((adr_hi Int))
:extrafuns ((adr_medhi Int))
:extrafuns ((adr_medlo Int))
:extrafuns ((adr_lo Int))
:extrafuns ((select_format Int Int))
:extrafuns ((percent Int))
:extrafuns ((s Int))
:extrafuns ((s_count Int Int))
:extrafuns ((x Int))
:extrafuns ((x_count Int Int))
:formula
(flet ($concval (and (and (and (and (and (and (and (and (= distance 20) (= fmt_length 11)) (= adr_lo 5)) (= adr_medlo 2)) (= adr_medhi 5)) (= adr_hi 3)) (= percent 37)) (= s 115)) (= x 120)))
(flet ($attack (and (and (and (and (and (and (and (= fmt0 0) (= arg0 (- fmt0 distance))) (>= arg1 fmt0)) (< fmt1 (- (+ fmt0 fmt_length) 1))) (> fmt1 (+ fmt0 1))) (>= arg1 (+ arg0 distance))) (< arg1 (- (+ (+ arg0 distance) fmt_length) 4))) (= arg1 (+ (+ arg0 (* 4 (s_count (- (- fmt1 2) fmt0)))) (* 4 (x_count (- (- fmt1 2) fmt0)))))))
(flet ($restrict (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (or (= (select_format 0) percent) (= (select_format 0) s)) (= (select_format 0) x)) (= (select_format 0) adr_lo)) (= (select_format 0) adr_medlo)) (= (select_format 0) adr_medhi)) (= (select_format 0) adr_hi)) (= (select_format 0) 255)) (= (select_format 1) percent)) (= (select_format 1) s)) (= (select_format 1) x)) (= (select_format 1) adr_lo)) (= (select_format 1) adr_medlo)) (= (select_format 1) adr_medhi)) (= (select_format 1) adr_hi)) (= (select_format 1) 255)) (= (select_format 2) percent)) (= (select_format 2) s)) (= (select_format 2) x)) (= (select_format 2) adr_lo)) (= (select_format 2) adr_medlo)) (= (select_format 2) adr_medhi)) (= (select_format 2) adr_hi)) (= (select_format 2) 255)) (= (select_format 3) percent)) (= (select_format 3) s)) (= (select_format 3) x)) (= (select_format 3) adr_lo)) (= (select_format 3) adr_medlo)) (= (select_format 3) adr_medhi)) (= (select_format 3) adr_hi)) (= (select_format 3) 255)) (= (select_format 4) percent)) (= (select_format 4) s)) (= (select_format 4) x)) (= (select_format 4) adr_lo)) (= (select_format 4) adr_medlo)) (= (select_format 4) adr_medhi)) (= (select_format 4) adr_hi)) (= (select_format 4) 255)) (= (select_format 5) percent)) (= (select_format 5) s)) (= (select_format 5) x)) (= (select_format 5) adr_lo)) (= (select_format 5) adr_medlo)) (= (select_format 5) adr_medhi)) (= (select_format 5) adr_hi)) (= (select_format 5) 255)) (= (select_format 6) percent)) (= (select_format 6) s)) (= (select_format 6) x)) (= (select_format 6) adr_lo)) (= (select_format 6) adr_medlo)) (= (select_format 6) adr_medhi)) (= (select_format 6) adr_hi)) (= (select_format 6) 255)) (= (select_format 7) percent)) (= (select_format 7) s)) (= (select_format 7) x)) (= (select_format 7) adr_lo)) (= (select_format 7) adr_medlo)) (= (select_format 7) adr_medhi)) (= (select_format 7) adr_hi)) (= (select_format 7) 255)) (= (select_format 8) percent)) (= (select_format 8) s)) (= (select_format 8) x)) (= (select_format 8) adr_lo)) (= (select_format 8) adr_medlo)) (= (select_format 8) adr_medhi)) (= (select_format 8) adr_hi)) (= (select_format 8) 255)) (= (select_format 9) percent)) (= (select_format 9) s)) (= (select_format 9) x)) (= (select_format 9) adr_lo)) (= (select_format 9) adr_medlo)) (= (select_format 9) adr_medhi)) (= (select_format 9) adr_hi)) (= (select_format 9) 255)) (= (select_format 10) percent)) (= (select_format 10) s)) (= (select_format 10) x)) (= (select_format 10) adr_lo)) (= (select_format 10) adr_medlo)) (= (select_format 10) adr_medhi)) (= (select_format 10) adr_hi)) (= (select_format 10) 255)))
(flet ($counterdef (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (and (if_then_else (and (= (select_format 0) percent) (= (select_format 1) s)) (= (s_count 0) 1) (= (s_count 0) 0)) (if_then_else (and (= (select_format 1) percent) (= (select_format 2) s)) (= (s_count 1) (+ (s_count 0) 1)) (= (s_count 1) (s_count 0)))) (if_then_else (and (= (select_format 2) percent) (= (select_format 3) s)) (= (s_count 2) (+ (s_count 1) 1)) (= (s_count 2) (s_count 1)))) (if_then_else (and (= (select_format 3) percent) (= (select_format 4) s)) (= (s_count 3) (+ (s_count 2) 1)) (= (s_count 3) (s_count 2)))) (if_then_else (and (= (select_format 4) percent) (= (select_format 5) s)) (= (s_count 4) (+ (s_count 3) 1)) (= (s_count 4) (s_count 3)))) (if_then_else (and (= (select_format 5) percent) (= (select_format 6) s)) (= (s_count 5) (+ (s_count 4) 1)) (= (s_count 5) (s_count 4)))) (if_then_else (and (= (select_format 6) percent) (= (select_format 7) s)) (= (s_count 6) (+ (s_count 5) 1)) (= (s_count 6) (s_count 5)))) (if_then_else (and (= (select_format 7) percent) (= (select_format 8) s)) (= (s_count 7) (+ (s_count 6) 1)) (= (s_count 7) (s_count 6)))) (if_then_else (and (= (select_format 8) percent) (= (select_format 9) s)) (= (s_count 8) (+ (s_count 7) 1)) (= (s_count 8) (s_count 7)))) (if_then_else (and (= (select_format 9) percent) (= (select_format 10) s)) (= (s_count 9) (+ (s_count 8) 1)) (= (s_count 9) (s_count 8)))) (if_then_else (and (= (select_format 10) percent) (= (select_format 11) s)) (= (s_count 10) (+ (s_count 9) 1)) (= (s_count 10) (s_count 9)))) (if_then_else (and (= (select_format 0) percent) (= (select_format 1) x)) (= (x_count 0) 1) (= (x_count 0) 0))) (if_then_else (and (= (select_format 1) percent) (= (select_format 2) x)) (= (x_count 1) (+ (x_count 0) 1)) (= (x_count 1) (x_count 0)))) (if_then_else (and (= (select_format 2) percent) (= (select_format 3) x)) (= (x_count 2) (+ (x_count 1) 1)) (= (x_count 2) (x_count 1)))) (if_then_else (and (= (select_format 3) percent) (= (select_format 4) x)) (= (x_count 3) (+ (x_count 2) 1)) (= (x_count 3) (x_count 2)))) (if_then_else (and (= (select_format 4) percent) (= (select_format 5) x)) (= (x_count 4) (+ (x_count 3) 1)) (= (x_count 4) (x_count 3)))) (if_then_else (and (= (select_format 5) percent) (= (select_format 6) x)) (= (x_count 5) (+ (x_count 4) 1)) (= (x_count 5) (x_count 4)))) (if_then_else (and (= (select_format 6) percent) (= (select_format 7) x)) (= (x_count 6) (+ (x_count 5) 1)) (= (x_count 6) (x_count 5)))) (if_then_else (and (= (select_format 7) percent) (= (select_format 8) x)) (= (x_count 7) (+ (x_count 6) 1)) (= (x_count 7) (x_count 6)))) (if_then_else (and (= (select_format 8) percent) (= (select_format 9) x)) (= (x_count 8) (+ (x_count 7) 1)) (= (x_count 8) (x_count 7)))) (if_then_else (and (= (select_format 9) percent) (= (select_format 10) x)) (= (x_count 9) (+ (x_count 8) 1)) (= (x_count 9) (x_count 8)))) (if_then_else (and (= (select_format 10) percent) (= (select_format 11) x)) (= (x_count 10) (+ (x_count 9) 1)) (= (x_count 10) (x_count 9)))))
(flet ($integral (and (or (or (or (or (or (or (or (or (or (or (= fmt1 (+ fmt0 0)) (= fmt1 (+ fmt0 1))) (= fmt1 (+ fmt0 2))) (= fmt1 (+ fmt0 3))) (= fmt1 (+ fmt0 4))) (= fmt1 (+ fmt0 5))) (= fmt1 (+ fmt0 6))) (= fmt1 (+ fmt0 7))) (= fmt1 (+ fmt0 8))) (= fmt1 (+ fmt0 9))) (= fmt1 (+ fmt0 10))) (or (or (or (or (or (or (or (= arg1 (+ fmt0 0)) (= arg1 (+ fmt0 1))) (= arg1 (+ fmt0 2))) (= arg1 (+ fmt0 3))) (= arg1 (+ fmt0 4))) (= arg1 (+ fmt0 5))) (= arg1 (+ fmt0 6))) (= arg1 (+ fmt0 7)))))
(and (and (and (and (and $concval $attack) $restrict) $counterdef) $integral) (not (and (and (and (and (and (= (select_format fmt1) percent) (= (select_format (+ fmt1 1)) s)) (= (select_format arg1) adr_lo)) (= (select_format (+ arg1 1)) adr_medlo)) (= (select_format (+ arg1 2)) adr_medhi)) (= (select_format (+ arg1 3)) adr_hi))))))))))
