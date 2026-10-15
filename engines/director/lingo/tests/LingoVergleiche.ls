put member(1) = 0
-- 0
put 0 = member(1)
-- 0

put member(1) = 1
-- 0
put 1 = member(1)
-- 0
put member(1) = member(1)
-- 1
put member(1) = member(2)
-- 0
put member(99) = 0
-- 0
put ilk(member(1))
-- #member
x = member(1)
put ilk(x)
-- #member
put x = 0
-- 0
put "abc" = #abc
-- 0
put #abc = "abc"
-- 0
put "none" = #none
-- 0
put #none = "none"
-- 0
put EMPTY = #none
-- 0
put symbol("abc") = #abc
-- 1
put string(#abc) = "abc"
-- 1
put VOID = 0
-- 1
put 0 = VOID
-- 1
put VOID = EMPTY
-- 0
put EMPTY = VOID
-- 0
put [1, 2] = [1, 2]
-- 1
put point(1, 2) = point(1, 2)
-- 1
put point(1, 2) = [1, 2]
-- 1
put [1, 2] = point(1, 2)
-- 1
put rect(0, 0, 1, 1) = [0, 0, 1, 1]
-- 1
put "abc" = "ABC"
-- 1
put #abc = #abc
-- 1
put member(1) = VOID
-- 0