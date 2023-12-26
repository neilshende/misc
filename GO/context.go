package main

import (
  "context"
  "errors"
  "fmt"
  "math"
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
  num := 177777777

  // Create a context with no deadline (effectively infinite)
  ctx := context.Background()

  isPrime, err := isPrimeWithContext(ctx, num)
  if err != nil {
    fmt.Println("Error:", err)
  } else {
    fmt.Println(num, "is prime:", isPrime)
  }
}
