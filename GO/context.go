package main

import (
  "context"
  "errors"
  "fmt"
  "math"
  "time"
)

func isPrimeWithContext(ctx context.Context, num int) (bool, error) {
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
func main() {
  num := 2147483647

  // Create a context with no deadline (effectively infinite)
  ctx := context.Background()

  isPrime, err := isPrimeWithContext(ctx, num)
  if err != nil {
    fmt.Println("Error:", err)
  } else {
    fmt.Println(num, "is prime:", isPrime)
  }

  b := make(chan bool)
  go func(num int) {
       limit := int(math.Sqrt(float64(num)))
       for i := 2; i < limit; i++ {
          if num%i == 0 {
            b <- false
            break
          }
        }
        b <- true
   } (num)

  ctx2, cancel := context.WithTimeout(context.Background(), time.Second)
  defer cancel()
  select {
   case bb := <-b:
      fmt.Println(num, "is prime:", bb)
   case <-ctx2.Done():
      fmt.Println("Timeout")
   }
}
