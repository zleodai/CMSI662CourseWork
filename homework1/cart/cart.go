package cart

import (
	"cartmodule/server"

	"fmt"
	"regexp"
	"strings"

	"github.com/google/uuid"
)

type Cart struct {
	id string
	customerID string
	items map[string]int
}

var validCustomerID = regexp.MustCompile(`^[a-zA-Z]{3}[0-9]{5}[a-zA-Z]{2}-[AQ]`)
func isValidCustomerID(customerID string) bool {
	return validCustomerID.MatchString(customerID)
}

func NewCart(customerID string) (Cart, bool) {
	if !isValidCustomerID(customerID) {
		fmt.Printf("New Cart failed. Provided customer ID %s is invalid\n", customerID)
		return Cart{}, false
	}

	newID := uuid.New().String()

	if newID == "" {
		fmt.Println("New Cart failed. UUID generated was invalid")
		return Cart{}, false
	}

	return Cart{
		id: newID,
		customerID: customerID,
		items: make(map[string]int),
	}, true
}

func GetCustomerID(cart Cart) string {
	return strings.Clone(cart.customerID)
}

func GetID(cart Cart) string {
	return strings.Clone(cart.id)
}

func GetItems(cart Cart) map[string]int {
	items := make(map[string]int)
	for k, v := range cart.items {
		key := strings.Clone(k)
		items[key] = v
	}
	return items
}

const maxStringLength = 100
var validItemName = regexp.MustCompile(`^[a-zA-Z ]+$`)
func passesItemNameCheck(item string) bool {
	if len(item) > maxStringLength {
		fmt.Printf("Item Name Check Failed. Item name exceeded max string length of %d\n", maxStringLength)
		return false
	}

	if !validItemName.MatchString(item) {
		fmt.Println("Item Name Check Failed. Item name not valid")
		return false
	}

	return true
}

const costLowerBound float64 = 0.00
const costUpperBound float64 = 1000000000.00
func passesCostBoundsCheck(cost float64) bool {
	return cost >= costLowerBound && cost <= costUpperBound
}
// Returns total cost and bool indicating success
func GetTotalCost(cart Cart) (float64, bool) {
	totalCost := 0.0
	
	cartItems := GetItems(cart)
	for item, quantity := range cartItems {
		itemCost, success := server.GetItemCost(item)

		if (!success) {
			fmt.Printf("Get Total Cost failed. Provided item %s not found in the current catalog\n", item)
			return 0.0, false
		}

		if !passesCostBoundsCheck(itemCost) {
			fmt.Printf("Get Total Cost failed. Item cost %f is out of bounds\n", itemCost)
			return 0.0, false
		}

		totalCost += float64(quantity) * itemCost
	}

	if passesCostBoundsCheck(totalCost) {
		return totalCost, true
	} else {
		fmt.Printf("Get Total Cost failed. Calculated cost %f is out of bounds\n", totalCost)
		return 0.0, false
	}
}

const itemQuantityLowerBound int = 1
const itemQuantityUpperBound int = 1000000
func passesItemBoundsCheck(quantity int) bool {
	return quantity >= itemQuantityLowerBound && quantity <= itemQuantityUpperBound
}

func UpdateItem(cart Cart, item string, quantity int) (Cart, bool) {
	if !passesItemBoundsCheck(quantity) {
		fmt.Printf("Update Item failed. Attempted to add an invalid quantity of %s to the cart\n", item)
		return cart, false
	}
	
	if !passesItemNameCheck(item) {
		fmt.Println("Update Item failed.")
		return cart, false
	}

	cartItems := GetItems(cart)
	itemKey := strings.Clone(item)

	if _, exists := cartItems[itemKey]; exists {
		cartItems[itemKey] = quantity
	} else {
		fmt.Printf("Update Item failed. Attempted to update non-existent item %s in the cart\n", item)
		return cart, false
	}

	return Cart{
		id: GetID(cart),
		customerID: GetCustomerID(cart),
		items: cartItems,
	}, true
}

func AddItem(cart Cart, item string, quantity int) (Cart, bool) {
	if !passesItemBoundsCheck(quantity) {
		fmt.Printf("Add Item failed. Attempted to add an invalid quantity of %s to the cart\n", item)
		return cart, false
	}

	if !passesItemNameCheck(item) {
		fmt.Println("Add Item failed.")
		return cart, false
	}

	if !server.IsValidItem(item) {
		fmt.Printf("Add Item failed. Item %s was not in the catalog.\n", item)
		return cart, false
	}

	cartItems := GetItems(cart)
	itemKey := strings.Clone(item)

	if existingQuantity, exists := cartItems[itemKey]; exists {
		newCount := existingQuantity + quantity
		if passesItemBoundsCheck(newCount) {
			cartItems[itemKey] = newCount
		} else {
			fmt.Printf("Add Item failed. Attempt to add will result in an invalid quantity of %s to the cart\n", item)
			return cart, false
		}
	} else {
		cartItems[itemKey] = quantity
	}

	return Cart{
		id: GetID(cart),
		customerID: GetCustomerID(cart),
		items: cartItems,
	}, true
}

func RemoveItem(cart Cart, item string) (Cart, bool) {
	if !passesItemNameCheck(item) {
		fmt.Println("Update Item failed.")
		return cart, false
	}

	cartItems := GetItems(cart)

	if _, exists := cartItems[item]; exists {
		delete(cartItems, item)
	} else {
		fmt.Printf("Remove Item failed. Attempted to remove non-existent item %s from the cart\n", item)
		return cart, false
	}

	return Cart{
		id: GetID(cart),
		customerID: GetCustomerID(cart),
		items: cartItems,
	}, true
}