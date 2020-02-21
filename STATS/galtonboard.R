x=replicate(1000000,sum(sample(c(1,-1), sample(c(50,51), 1, T), T)))
hist(x,100)
