x=rpois(10000, 19)
hist(x,100)
message("probability x>22 ", sum(x>22)/length(x))
#same as:
ppois(22, 19, F)

# one in 50 youngest
qpois(.02,19)
#should be almost equal to:
y=sort(x)
y[200]
