package main
import (
   "fmt"
   "os"
)

func main () {
files, err := os.ReadDir(".")
if err != nil {
   panic(err)
}
for _, f := range files {
   if f.IsDir() {
      fmt.Printf("%s is a dir\n", f.Name())
   } else {
      fmt.Printf("%s is a file\n", f.Name())
   }
}
}
