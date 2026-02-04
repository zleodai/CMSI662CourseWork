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
	testCart, sucess := cart.NewCart("Abc12345de-Q")
	if !sucess { return }

	testCart, sucess = cart.AddItem(testCart, "apple", 5)
	if !sucess { return }

	testCart, sucess = cart.AddItem(testCart, "banana", 10)
	if !sucess { return }

	testCart, sucess = cart.AddItem(testCart, "orange", 1)
	if !sucess { return }

	testCart, sucess = cart.RemoveItem(testCart, "orange")
	if !sucess { return }
	
	logCart(testCart)

	totalCost, sucess := cart.GetTotalCost(testCart)
	if !sucess { return }

	fmt.Printf("Total Cost: $%.2f\n", totalCost)
}