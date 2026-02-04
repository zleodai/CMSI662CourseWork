package main

import (
	"cartmodule/cart"
	"fmt"
)

func logCart(c cart.Cart) {
	fmt.Printf("Cart ID: %d\n", cart.GetID(c))
	fmt.Printf("Customer ID: %s\n", cart.GetCustomerID(c))
	cItems := cart.GetItems(c)
	cItemsLog := ""
	for k, v := range cItems {
		cItemsLog += fmt.Sprintf(" %s:, %d\n", k, v)
	}
	fmt.Printf("Cart Items: \n%s\n", cItemsLog)
}

func main() {
	testCart := cart.NewCart("testCustomer")
	testCart = cart.AddItem(testCart, "apple", 5)
	testCart = cart.AddItem(testCart, "banana", 10)
	testCart = cart.AddItem(testCart, "orange", 1)
	testCart = cart.RemoveItem(testCart, "orange")
	logCart(testCart)
}