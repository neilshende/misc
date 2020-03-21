 logistic = function (t, L, k, t0) {
    return (L/(1 + exp(-k*(t -t0))))
 }
 lplotter = function (t, L, k, t0) {
    TT = seq(1:t)
    LL = logistic(TT, L, k, t0)
    plot(TT, LL)
 }

