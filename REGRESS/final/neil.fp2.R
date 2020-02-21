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

ridge = function(X, Y, lambda)
{
   B = solve(t(X) %*% X + lambda * diag(nrow(t(X) %*% X))) %*% t(X) %*% Y
   return(B)
}

computeRidgeCoefficeints = function( grid, X, Y )
{
   n = length(grid)
   B = ridge(X, Y, grid[1])
   M = cbind(B)
   for (i in 2:n) {
      lambda = grid[i]
      B = ridge(X, Y, lambda)
      M = cbind(M, B)
   }
   return(M)
}

df = read.csv("hitters.csv", header=TRUE, stringsAsFactors=FALSE)
x = df[, c(-1, -20)]
y = df[ , 20]
X = as.matrix(x)
X = standardize(X)
Y = as.matrix(y)
#center Y
ym = mean(Y)
Y = Y - ym

n = nrow(X)
I = rep(1, n)
X = cbind(I, X)
m = ncol(X)

grid = 10^seq(10, -2, length=100)
M = computeRidgeCoefficeints ( grid, X, Y )

library(glmnet)
set.seed(1)
result = cv.glmnet(X[, -1], Y, alpha=0, lambda = grid, nfolds = 10, thresh = 1e-10)
plot(result)
selectedLambda = result$lambda.min

selectedB = ridge(X, Y, selectedLambda)
message("Report Ridge model for selected lamda ", selectedLambda)
print(selectedB)
