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
	fmt.Printf("Cart Items: \n%s", cItemsLog)
}

func main() {
	testCart := cart.NewCart("testCustomer")
	testCart, success := cart.AddItem(testCart, "apple", 5)
	testCart, success = cart.AddItem(testCart, "banana", 10)
	testCart, success = cart.AddItem(testCart, "orange", 1)
	testCart, success = cart.RemoveItem(testCart, "orange")
	logCart(testCart)

	totalCost, success := cart.GetTotalCost(testCart)
	if success {
		fmt.Printf("Total Cost: $%.2f\n", totalCost)
	} else {
		fmt.Println("Failed to calculate total cost.")
	}
}