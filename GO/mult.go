package main
 
import "fmt"
 
// declaring a function having
// return type of int, int
func sumDiff(a int, b int) (int, int, int, int, int, int, int, int) {
 
    return (a + b), (a - b), 3, 4, 5, 6, 7, 8
 
    // this function returns sum ,
    // difference of the two numbers
}
 
func main() {
 
    // declaring two values a and b
    var a = 68
    var b = 100
 
    // calling the function
    // with multiple assignments
    var sum, diff, c, _, _, _, _, h = sumDiff(a, b)
 
    // Printing the values
    fmt.Println("Sum = ", sum, "\nDifference = ", diff, "\nc =", c, "\nh = ", h)
}
