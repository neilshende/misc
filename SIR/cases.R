ncases = function(r, N0, k) {
   dn = r * N0 * ( 1 - N0/k)
   return (dn)
}
nplotter = function (r, N0, K, t, fr=0.02, lag=30) {
  k = K           #population of region under model
  TT = seq(1:t)
  NN = rep(N0, t) #active cases on ith day
  DN = rep(N0, t) #new cases
  RN = rep(0, t)  #recovered cases
  SN = rep(k, t)  #susceptibe number
  XN = rep(0, t)  #fatalities on ith day
  for (i in 2:t) {
     DN[i] = ncases(r, NN[i-1], k)
     if (i > lag) {
         RN[i] = (1-fr) * NN[i-lag]
         XN[i] = fr * DN[i-lag]
     } 
     NN[i] = NN[i-1] + DN[i] - RN[i]
     #NN[i] = NN[i-1] + DN[i]
     if (NN[i] <= 0) NN[i] = DN[i]
     SN[i] = k - NN[i]
  }
  message("Max number of fatal cases per day ", round(max(XN)))
  message("Max number of new cases per day ", round(max(DN)))
  message("Max number of active cases per day ", round(max(NN)))
  message("Total number of fatal cases ", round(sum(XN)))
  message("Total number of cases ", round(sum(DN)))
  
  plot(range(TT), range(NN), main="SIRF Model for CoVid19", xlab="Days", ylab="Number of cases.")
  lines(TT, NN, col="black")
  lines(TT, XN, col="red")
  lines (TT, RN, col="green")
  lines( TT, SN, col="blue")
  legend(0.8*max(TT), max(NN), legend=c("infected", "fatal", "recovered", "susceptible"), col=c("black", "red", "green", "blue"), lty=1:2, cex=0.8)
}
