standardize = function (X) 
{
   N = nrow(X)
   D = ncol(X)
   NX = X 
   for (i in 1:D) {
      mXi = mean(X[,i])
      sXi = sd(X[,i])
      if (sXi != 0) {
         NX[,i]=(X[,i] - mXi)/sXi
      }
      #else keep the column unchanged. all 1s case.
   }   
   return(NX)
}

RIDGE = function(X, Y, lambda)
{
   step = .01 
   iter = 1000
   nv = ncol(X)
   B = rep(0, nv)
   B = as.matrix(B)
   n = ncol(Y)
   for (i in 1:iter) {
      B = B - (step/n) * ((t(X) %*% (X%*%B - Y)) - lambda * B)
      if (max(abs(X%*%B - Y)) < 10^-5) break
   }
   #return(list(B[1], B[-1]))
   return(B)
}

RIDGE2 = function(X, Y, lambda)
{
   step = 0.01
   iter = 2000
   n = ncol(Y)
   nv = ncol(X)
   B = rep(0, nv)
   B = as.matrix(B)
   B = t(B)
   e = t(Y) - B %*% t(X)
   gr = -(2/n) * e %*% X + lambda * B
   B = B - (step/n) * gr
   for (i in 1:iter) {
     e = t(Y) - B %*% t(X)
     gr = -(2/n) * e %*% X + lambda * B
     B = B - 2 * (step/n) * gr
     #if (gr < 10^-3) break
   }
   return(t(B))
}
df = read.csv("hitters.csv", header=TRUE, stringsAsFactors=FALSE)
x = df[, c(-1, -20)]
y = df[ , 20]
X = as.matrix(x)
X = standardize(X)
Y = as.matrix(y)
Y = standardize(Y)
n = nrow(X)
I = rep(1, n)
X = cbind(I, X)
m = ncol(X)


grid = 10^seq(10, -2, length=100)
B = RIDGE2(X, Y, grid[1])
M = cbind(B)
#for (i in 2:100) {
#  lambda = grid[i]
#  B = RIDGE(X, Y, lambda)
#  M = cbind(M, B)
#}
