MAE = function (X, ZEindependent)
{

   N = nrow(X)
   D = ncol(X)
   colnames(X) = c("x", "y")
   xm = mean(X$x)
   ym = mean(X$y)
   X$e = abs(X$x - xm)
   X$z = (X$y - ym) / (X$x - xm)
   Xz = X$z[order(X$z)]
   if (ZEindependent == 1) {
      Xe = X$e[order(X$e)]
   } else {
      Xe = X$e[order(X$z)]
   } 
   Xse = rep(0, N)
   sn = 0
   for (i in 1:N) {
      sn = sn + Xe[i]
      Xse[i] = sn
   }
   for (j in 1:N) {
     if (Xse[j] > sn/2) break
   }
   w1 = Xz[j]
   w0 = ym - xm * w1
   return(c(w0, w1))
}

X = read.csv("height_weight_genders.csv")
X = X[,c(2,3)]

w = MAE(X, 0)
print(w)
RX0 = X
RX0$Weight = w[1] + w[2] * RX0$Height

w = MAE(X, 1)
print(w)
RX1 = X
RX1$Weight = w[1] + w[2] * RX1$Height

plot(X, main="Data black, MAE red(order z)/blue(order z,e)")
points(RX0, col="red")
points(RX1, col="blue")
