accumulatorFactory <- function(init) {
  currentSum <- init
  function(add) {
    currentSum <<- currentSum + add
    currentSum
  }
}
obj <- accumulatorFactory(1) #instantiate
obj(5) # will print 6
