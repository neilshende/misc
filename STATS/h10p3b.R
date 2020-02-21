s40 = function() {
   x = rnorm(46, 96, 13)
   return (sum(x[1:40])/40)
}
N = replicate(10000, s40())
message("sd and mean of N ", sd(N), ",", mean(N))
hist(N, 25)
