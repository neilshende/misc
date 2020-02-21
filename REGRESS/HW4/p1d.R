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

GD = function(X, Y)
{
   step = .01
   iter = 1500
   n = ncol(X)
   B = rep(0, n)
   B = as.matrix(B)
   for (i in 1:iter) {
      #B = B + step * sign(t(X) %*% (Y - X%*%B))
      B = B - step * (1/n) * (t(X) %*% (X%*%B - Y))
      if (i == 50) {
         plot(Y, X%*%B)
      } else if (i == 100) {
         points(Y, X%*%B, col="blue")
      } else if (i == 1500) {
         points(Y, X%*%B, col="red")
      }
   }
   print(B)
   return(list(B[1], B[-1]))
}

df = read.csv("education.csv");
x = df[, c(4,5,6)]
x = standardize(x);
X = as.matrix(x)
I = rep(1, nrow(X))
X = cbind(I, X)

y = df$Y
Y = as.matrix(y)
Y=standardize(Y)

BL = GD(X, Y)
print(BL)
