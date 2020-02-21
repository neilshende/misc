standardize = function (X) 
{
   N = nrow(X)
   D = ncol(X)
   NX = X
   for (i in 1:D) {
      mXi = mean(X[,i])
      sXi = sd(X[,i])
      NX[,i]=(X[,i] - mXi)/sXi
   }
   return(NX)
}

regression = function (X)
{
   N = nrow(X)
   D = ncol(X)
   colnames(X) = c("x", "y")
   denominator =  (N * sum((X$x)^2)) - ((sum(X$x))^2)
   w0numerator = ((sum((X$x)^2)) * (sum(X$y))) - (sum(X$x) * sum(X$x * X$y))
   w0 = w0numerator / denominator
   w1numerator = N * sum(X$x * X$y) - sum(X$x) * sum(X$y)
   w1 = w1numerator / denominator
   return(c(w0, w1))

}

X = read.csv("height_weight_genders.csv")
X = X[,c(2,3)]

w = regression(X)
print(w)
RX = X
RX$Weight = w[1] + w[2] * RX$Height
NRX = standardize(RX)

plot(X, main="Data blue, Regress red", col="blue")
points(RX, col="red")

NX = standardize(X)
nw = regression(NX)
print(nw)
RNX = NX
RNX$Weight = nw[1] + nw[2] * RNX$Height

plot(NX, main="Norm X black, Norm-regress blue Regress-norm red")
lines(NRX, col="blue")
lines(RNX, col="red")

