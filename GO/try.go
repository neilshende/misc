package main
import (
  "fmt"
  "math/rand"
  "runtime/debug"
)

func PrintRandomDiv() {
  fmt.Println(1 / rand.Intn(2))
}
func TryCatch(f func()) func() error {
  return func() (err error) {
    defer func() {
      if panicInfo := recover(); panicInfo != nil {
        err = fmt.Errorf("%v, %s", panicInfo, string(debug.Stack()))
        return
      }
    }()
    f() // calling the decorated function
    return err
  }
}
func TryCatchLoop(f func()) func() {
  return func() {
    for {
      if err := TryCatch(f)(); err != nil {
        fmt.Println(err)
      } else {
        return
      }
    }
  }
}
func main() {
  for i := 0; i < 10; i++ {
    if err := TryCatch(PrintRandomDiv)(); err != nil {
      fmt.Println(err)
    }
  }
  TryCatchLoop(PrintRandomDiv)()
}
