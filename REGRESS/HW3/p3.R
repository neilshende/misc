MAE = function (X)
{

   N = nrow(X)
   D = ncol(X)
   colnames(X) = c("x", "y")
   xm = mean(X$x)
   ym = mean(X$y)
   X$e = abs(X$x - xm)
   X$z = (X$y - ym) / (X$x - xm)
   X=X[order(X$z),]
   X$se = 0
   sn = 0
   for (i in 1:N) {
      sn = sn + X$e[i]
      X$se[i] = sn
   }
   for (j in 1:N) {
     if (X$se[j] > sn/2) break
   }
   w1 = X$z[j]
   w0 = ym - xm * w1
   return(c(w0, w1))
}

X = read.csv("height_weight_genders.csv")
X = X[,c(2,3)]

w = MAE(X)
print(w)
RX = X
RX$Weight = w[1] + w[2] * RX$Height

plot(X, main="Data black, MAE-regress red")
points(RX, col="red")
