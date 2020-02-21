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

X = read.csv("height_weight_genders.csv")
X = X[,c(2,3)]
NX = standardize(X)
write.table(NX, "standardized_height_weight.csv", row.names=FALSE)
