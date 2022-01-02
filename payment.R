monthly_payment = function(principal, rate, years) {
  P = principal
  R = rate/1200
  N = years*12
  t = (1 + R) ^ N
  return (P * R * t / (t - 1))
}
principal_balance = function (monthly_payment, rate, years) {
  R = rate/1200
  N = years*12
  mp = monthly_payment
  t = (1 + R) ^ N
  return (mp *(t -1) /(t * R))
}

refi = function(curr_balance=0, curr_payment=0, curr_rate, curr_num_payments, new_rate, closing_cost,
                add_closing_cost_to_loan=TRUE) {
   if (curr_balance == 0) {
      curr_balance = principal_balance(curr_payment, curr_rate, curr_num_payments/12)
   }
   if (curr_payment == 0) {
      curr_payment = monthly_payment(curr_balance, curr_rate, curr_num_payments/12)
   }
   if (curr_balance == 0 || curr_payment == 0) {
     message("both curr_balance and curr_payment are 0\n")
   }
   if (add_closing_cost_to_loan) {
      new_balance= curr_balance+closing_cost
   } else {
     new_balance= curr_balance
   }

   ret = vector("list", 4)
   names(ret) = c("new_principal", "new_payment", "monthly_savings", "months_to_break_even")
   ret$new_principal = new_balance
   ret$new_payment = monthly_payment(new_balance, new_rate, curr_num_payments/12)
   ret$monthly_savings = curr_payment - ret$new_payment
   if (!add_closing_cost_to_loan && ret$monthly_savings > 0) {
      ret$months_to_break_even = ceiling(closing_cost/ret$monthly_savings)
   } else {
      ret$months_to_break_even = NA
   }
   return(ret)
}

amortize <- function(p_input = 25000, i_input = 7, n_years = 30,
  output = "table", index = NULL) { 

  i_input <- i_input / 100 #percent
  
  n_months <- rep(n_years*12, length(p_input))
  
  if(is.null(index)) {
    index <- matrix(rep(1:length(n_months), each = n_months[1]), 
      nrow = n_months[1])
  } else {
    index <- matrix(rep(index, each = n_months[1]), nrow = n_months[1])
  }
  
  p_input <- matrix(p_input, ncol = length(p_input))
  i_input <- matrix(i_input, ncol = length(i_input))
  i_monthly <- i_input / (12)
  payment <- p_input * i_monthly / (1 - (1 + i_monthly)^(-n_months[1]))
  
  Pt <- p_input # current principal or amount of loan
  currP <- NULL
  
  for(i in 1:n_months[1]) {
    H <- Pt * i_monthly # current monthly interest
    C <- payment - H # monthly payment minus monthly interest (principal paid for each month)
    Q <- Pt - C # new balance of principal of loan
    Pt <- Q # loops through until balance goes to zero
    currP <- rbind(currP, Pt)    
  }
  
  amortization <- rbind(p_input, currP[1:(n_months[1]-1),, drop = FALSE])
  monthly_principal <- amortization - currP
  monthly_interest <- rbind(
    (matrix(
      rep(payment, n_months[1]), 
      nrow = n_months[1], 
      byrow = TRUE) - monthly_principal)[1:(n_months[1]-1),, drop = FALSE],
    rep(0, length(n_months)))
  monthly_interest[1:nrow(monthly_interest) %% 12 == 0] <-
    monthly_principal[1:nrow(monthly_interest) %% 12 == 0] * i_monthly
  monthly_payment <- monthly_principal + monthly_interest
  installment <- matrix(rep(1 : n_months[1], length(n_months)), 
    nrow = n_months[1])
  
  input <- list(
    "amortization" = amortization,
    "payment" = monthly_payment,
    "principal" = monthly_principal,
    "interest" = monthly_interest,
    "installment" = installment,
    "index" = index)
  
  out <- switch(output, 
    "list" = input,
    "table" = as.data.frame(
      lapply(input, as.vector), 
      stringsAsFactors = FALSE),
    "balance" = as.data.frame(
      lapply(input[c("index", "amortization")], as.vector), 
      stringsAsFactors = FALSE),
    "payment" = as.data.frame(
      lapply(input[c("index", "payment")], as.vector), 
      stringsAsFactors = FALSE),
    "principal" = as.data.frame(
      lapply(input[c("index", "principal")], as.vector), 
      stringsAsFactors = FALSE), 
    "interest" = as.data.frame(
      lapply(input[c("index", "interest")], as.vector), 
      stringsAsFactors = FALSE)
  )
  
  return(out)
}
