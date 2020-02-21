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
      # else keep the column unchanged. All 1s case.
   }
   return(NX)
}

GDA = function(X, Y)
{
   step = .01
   iter = 30000
   n = ncol(X)
   B = rep(0, n)
   B = as.matrix(B)
   for (i in 1:iter) {
      Yh = X%*%B
      B = B + step * sign(t(X) %*% (Y - Yh))
      if (max(abs(Yh - Y)) < 10^-2) break
   }
   return(list(B[1], B[-1]))
}

df = read.csv("education.csv")
x = df[, c(4,5,6)]
x = standardize(x)
X = as.matrix(x)
I = rep(1, nrow(X))
X = cbind(I, X)

y = df$Y
Y = as.matrix(y)

BL = GDA(X, Y)
message("Using MAE, intercept and Coefficient vector are: ", BL[1], " ", BL[2])
