#explore
for (i in 1:10) {
  X = rchisq(n=10000, df=i, ncp = 0)
  message("i=", i, " E=", mean(X), " Variance=", sd(X)^2)
}

# additive property
x2=rchisq(n=10000, df=2, ncp = 0)
x3=rchisq(n=10000, df=3, ncp = 0)
xx=x2+x3
message( "E=", mean(xx), " Var=", sd(xx)^2)

# let's try more combinations to confirm.
for (i in 1:5) {
   X = rchisq(n=10000, df=i, ncp = 0)
   for (j in 1:5) {
      Y = rchisq(n=10000, df=j, ncp = 0)
      Z = X+Y
      message("i=", i, " j=", j, " E=", mean(Z), " Variance=", sd(Z)^2)
   }
}
