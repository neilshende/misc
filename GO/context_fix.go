package main

import (
  atomic "sync/atomic"
  "context"
  "errors"
  "fmt"
  "math"
  "time"
)

func isPrime(ctx context.Context, num int) (bool, error) {
  if num <= 1 {
    return false, nil // Not prime, but no error
  }

  // Only check divisibility up to the square root of num
  limit := int(math.Sqrt(float64(num)))

  for i := 2; i <= limit; i++ {
    select {
    case <-ctx.Done():
      return false, errors.New("context cancelled")
    default:
      if num%i == 0 {
        return false, nil // Not prime, but no error
      }
    }
  }
  return true, nil // Prime, no error
}

func isPrimeV2(ctx context.Context, num int) (bool, error) {
  var quit int32
  atomic.StoreInt32(&quit, 0)
  b := make(chan bool)
  go func(num int) {
       if num <= 1 {
          b <- false
          return
       }
       limit := int(math.Sqrt(float64(num)))
       for i := 2; i <= limit; i++ {
          if atomic.LoadInt32(&quit) != 0 {
             return
          }
          if num%i == 0 {
            b <- false
            return
          }
        }
        b <- true
   } (num)

  select {
   case bb := <-b:
      return bb, nil
   case <-ctx.Done():
      atomic.StoreInt32(&quit, 1)
      return false, errors.New("context cancelled")
   }
}

func main() {
  num := 9

  // Create a context with no deadline (effectively infinite)
  ctx := context.Background()
  isPrime, err := isPrime(ctx, num)
  if err != nil {
    fmt.Println("Error:", err)
  } else {
    fmt.Println(num, "is prime:", isPrime)
  }

  num = 2147483647
  // Create a context with timeout of one second.
  ctx2, cancel := context.WithTimeout(context.Background(), time.Second/1000000)
  defer cancel()
  isPrime, err = isPrimeV2(ctx2, num)
  if err != nil {
    fmt.Println("Error:", err)
  } else {
    fmt.Println(num, "is prime:", isPrime)
  }

}
