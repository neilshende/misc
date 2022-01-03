payment  <- function(loan, apr, months) {
    rate <- 1 + apr / 100 / 12
    loan * rate^months * (rate - 1) / (rate^months - 1)
}

amortize <- function(loan, payment, apr, months) {
    rate <- 1 + apr / 100 / 12
    month <- 0:months
    balance <- loan * rate^month - payment * (rate^month - 1) / (rate - 1)
    complete  <- match(TRUE, balance <= 0)
    balance <- ifelse(month < month[complete], balance, 0)
    principal <- loan - balance
    interest  <- payment * month - principal
    interest  <- ifelse(month < month[complete], interest, 
        interest[complete-1])
    amrt <- list(month = month[-1], balance=balance[-1], 
        principal = principal[-1], interest = interest[-1],
        paid = principal[-1] + interest[-1], loan=loan,
        payment=payment, apr=apr)
    class(amrt) <- "amortization"
    return(amrt)
    
}

summary.amortization <- function(amrt) {
    cat("  loan amount: ", amrt$loan, "\n")
    cat("          APR: ", amrt$apr, "\n")
    cat("      payment: ", amrt$payment, "\n")
    cat("       months: ", match(TRUE, amrt$balance <= 0), "\n")
    cat("interest paid: ", max(amrt$interest), "\n")
    cat("   total paid: ", max(amrt$paid), "\n")
}

plot.amortization <- function(amrt) {
    year <- amrt$month / 12
    plot(year, amrt$balance, type="l", ylab="Dollars ($)",
        main=paste("Amortization ($", format(amrt$loan, big.mark=","),
        ", ", amrt$apr, "%, $", format(amrt$payment, big.mark=",",
        nsmall=2, digits=2), "/mo)", sep=""), ylim=c(0,max(amrt$paid)),
        xlab="Year")
    lines(year, amrt$interest, col="red", lty=2)
    lines(year, amrt$principal, col="green", lty=2)
    lines(year, amrt$paid, col="blue", lty=2)
    legend(x=1, y=max(amrt$paid),
        legend=c("balance", "interest", "principal", "paid"), 
        col=c("black","red","green","blue"), lty=c(1,2,2,2))
}

