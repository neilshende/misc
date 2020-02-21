build_poly = function(x, degree)
{
   xx = rep(1, nrow(x))
   xx = as.matrix(xx)
   for (i in 1:degree) {
      xx = cbind(xx, x^i)
   }
   return (xx)
}

RMSE = function(ya, yp)
{
   N = nrow(ya)
   S = sum( (ya - yp)^2 )
   mse = S/N
   rmse = (2*mse)^0.5
   return(rmse)
}

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

GD = function(X, Y)
{
   step = .001 
   iter = 30000
   nv = ncol(X)
   B = rep(0, nv)
   B = as.matrix(B)
   n = ncol(Y)
   for (i in 1:iter) {
      Yh = X%*%B
      B = B - (step/n) * (t(X) %*% (Yh - Y)) 
      if (max(abs(Yh - Y)) < 10^-5) break
   }
   #return(list(B[1], B[-1]))
   return(B)
}

df = read.csv("polynomial.csv")
X = as.matrix(df$X)
Y = as.matrix(df$Y)

XX1 = build_poly(X, 1)
XX1 = standardize(XX1)
B = GD (XX1, Y)
B = as.matrix(B)
plot(X, Y, main="Polynomial degree 1")   
points(X, XX1%*%B, col="red")
rmse = RMSE(Y, XX1%*%B)
message("rmse for degree 1: ", rmse)

XX3 = build_poly(X, 3)
XX3 = standardize(XX3)
B = GD (XX3, Y)
B = as.matrix(B)
plot(X, Y, main="Polynomial degree 3")   
points(X, XX3%*%B, col="red")
rmse = RMSE(Y, XX3%*%B)
message("rmse for degree 3: ", rmse)

XX7 = build_poly(X, 7)
XX7 = standardize(XX7)
B = GD (XX7, Y)
B = as.matrix(B)
plot(X, Y, main="Polynomial degree 7")   
points(X, XX7%*%B, col="red")
rmse = RMSE(Y, XX7%*%B)
message("rmse for degree 7: ", rmse)

XX12 = build_poly(X, 12)
XX12 = standardize(XX12)
B = GD (XX12, Y)
B = as.matrix(B)
plot(X, Y, main="Polynomial degree 12")   
points(X, XX12%*%B, col="red")
rmse=RMSE(Y, XX12%*%B)
message("rmse for degree 12: ", rmse)
