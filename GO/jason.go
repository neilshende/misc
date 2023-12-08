package main

import (
    "fmt"
    "encoding/json"
)

type User struct {
    Name string
    Salary int
    Address string
    Ages[3] int
}

func main() {
    user := &User{Name:"Frank", Salary:100, Address:"200 North st.", Ages:[3]int{1,2,3}}
    b, err := json.Marshal(user)
    if err != nil {
        fmt.Printf("Error: %s", err)
        return;
    }
    fmt.Println(string(b))
}
