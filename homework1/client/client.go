package main

import (
	"cartmodule/cart"

	"fmt"
)

func logCart(c cart.Cart) {
	fmt.Printf("Cart ID: %s\n", cart.GetID(c))
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

	sucess = cart.AddItem(&testCart, "apple", 5)
	if !sucess { return }

	sucess = cart.UpdateItem(&testCart, "apple", 10)
	if !sucess { return }

	sucess = cart.AddItem(&testCart, "banana", 10)
	if !sucess { return }

	sucess = cart.AddItem(&testCart, "potato", 12)
	if !sucess { return }

	sucess = cart.AddItem(&testCart, "broccoli", 3)
	if !sucess { return }

	sucess = cart.AddItem(&testCart, "milk", 1)
	if !sucess { return }

	sucess = cart.AddItem(&testCart, "eggs", 1)
	if !sucess { return }

	sucess = cart.AddItem(&testCart, "beef", 2)
	if !sucess { return }

	sucess = cart.AddItem(&testCart, "bread", 3)
	if !sucess { return }

	sucess = cart.RemoveItem(&testCart, "bread")
	if !sucess { return }
	
	logCart(testCart)

	totalCost, sucess := cart.GetTotalCost(testCart)
	if !sucess { return }

	fmt.Printf("Total Cost: $%.2f\n", totalCost)
}