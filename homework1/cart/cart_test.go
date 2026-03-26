package cart

import (
	"regexp"
	"testing"
)

func CreateCart(customerID string) Cart {
	cart, _ := NewCart(customerID)
	return cart
}

func TestCartValidUserCreation(t *testing.T) {
	validUsers := []string{
		"Abc12345de-Q",
		"Cde45678cc-A",
		"cdg45268dz-A",
		"cze53678qq-Q",
	}

	for _, user := range validUsers {
		cart := CreateCart(user)
		if cart.id == "" {
			t.Errorf("NewCart failed with valid customer ID: %s", user)
		}
	}
}


func TestCartInvalidUserCreation(t *testing.T) {
	invalidUsers := []string{
		"Abc12345de-C",
		"Cde45678c1-A",
		"cdg45s68dz-A",
		"1ze53678qq-Q",
		"testing123",
	}

	for _, user := range invalidUsers {
		cart := CreateCart(user)
		if cart.id != "" {
			t.Errorf("NewCart created with invalid customer ID: %s", user)
		}
	}
}

func TestCardValidCreation(t *testing.T) {
	validUsername := "Abc12345de-Q"
	cart := CreateCart(validUsername)
	if cart.id == "" {
		t.Errorf("CreateCart failed with valid customer ID: %s", validUsername)
	}
	if cart.customerID != validUsername {
		t.Errorf("NewCart created cart with incorrect customer ID. Expected: %s, Got: %s", validUsername, cart.customerID)
	}
}

func TestUniqueID(t *testing.T) {
	cart1 := CreateCart("Abc12345de-Q")
	cart2 := CreateCart("Cde45678cc-A")

	if cart1.id == "" || cart2.id == "" {
		t.Errorf("CreateCart failed to create carts with valid customer IDs")
	}

	if cart1.id == cart2.id {
		t.Errorf("NewCart created carts with duplicate IDs: %s", cart1.id)
	}
}

func TestDefensiveCopy(t *testing.T) {
	cart := CreateCart("Abc12345de-Q")
	if cart.id == "" {
		t.Errorf("CreateCart failed with valid customer ID: %s", "Abc12345de-Q")
	}

	items := GetItems(cart)
	items["apple"] = 5

	if (&items == &cart.items) {
		t.Errorf("GetItems did not return a defensive copy of the items map")
	}
}

func TestIDsAreUUIDs(t *testing.T) {
	cart := CreateCart("Abc12345de-Q")

	if cart.id == "" {
		t.Errorf("CreateCart failed with valid customer ID: %s", "Abc12345de-Q")
	}

	cartID := GetID(cart)

	uuidPattern := regexp.MustCompile(`^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$`)
	if !uuidPattern.MatchString(cartID) {
		t.Errorf("cartID is not a valid UUID. Got: %s", cartID)
	}
}

func TestGetCustomerID(t *testing.T) {
	customerID := "Abc12345de-Q"
	cart := CreateCart(customerID)
	
	retrievedID := GetCustomerID(cart)
	if retrievedID != customerID {
		t.Errorf("GetCustomerID returned incorrect ID. Expected: %s, Got: %s", customerID, retrievedID)
	}
}

func TestAddItemValid(t *testing.T) {
	cart := CreateCart("Abc12345de-Q")
	
	success := AddItem(&cart, "apple", 5)
	if !success {
		t.Errorf("AddItem failed for valid item 'apple' with quantity 5")
	}
	
	items := GetItems(cart)
	if items["apple"] != 5 {
		t.Errorf("AddItem did not add item correctly. Expected quantity: 5, Got: %d", items["apple"])
	}
}

func TestAddItemInvalidNameWithNumbers(t *testing.T) {
	cart := CreateCart("Abc12345de-Q")
	
	success := AddItem(&cart, "apple123", 5)
	if success {
		t.Errorf("AddItem should have failed for item name with numbers: 'apple123'")
	}
}

func TestAddItemInvalidNameEmpty(t *testing.T) {
	cart := CreateCart("Abc12345de-Q")
	
	success := AddItem(&cart, "", 5)
	if success {
		t.Errorf("AddItem should have failed for empty item name")
	}
}

func TestAddItemInvalidQuantityZero(t *testing.T) {
	cart := CreateCart("Abc12345de-Q")
	
	success := AddItem(&cart, "apple", 0)
	if success {
		t.Errorf("AddItem should have failed for quantity 0")
	}
}

func TestAddItemInvalidQuantityNegative(t *testing.T) {
	cart := CreateCart("Abc12345de-Q")
	
	success := AddItem(&cart, "apple", -5)
	if success {
		t.Errorf("AddItem should have failed for negative quantity")
	}
}

func TestAddItemInvalidQuantityTooLarge(t *testing.T) {
	cart := CreateCart("Abc12345de-Q")
	
	success := AddItem(&cart, "apple", 2000000)
	if success {
		t.Errorf("AddItem should have failed for quantity exceeding upper bound")
	}
}

func TestAddItemMultipleItems(t *testing.T) {
	cart := CreateCart("Abc12345de-Q")
	
	AddItem(&cart, "apple", 3)
	AddItem(&cart, "orange", 2)
	
	items := GetItems(cart)
	if items["apple"] != 3 || items["orange"] != 2 {
		t.Errorf("AddItem failed to add multiple items correctly")
	}
}

func TestUpdateItemValid(t *testing.T) {
	cart := CreateCart("Abc12345de-Q")
	AddItem(&cart, "apple", 5)
	
	success := UpdateItem(&cart, "apple", 10)
	if !success {
		t.Errorf("UpdateItem failed for existing item 'apple'")
	}
	
	items := GetItems(cart)
	if items["apple"] != 10 {
		t.Errorf("UpdateItem did not update quantity correctly. Expected: 10, Got: %d", items["apple"])
	}
}

func TestUpdateItemNonExistent(t *testing.T) {
	cart := CreateCart("Abc12345de-Q")
	
	success := UpdateItem(&cart, "apple", 5)
	if success {
		t.Errorf("UpdateItem should have failed for non-existent item 'apple'")
	}
}

func TestUpdateItemInvalidQuantity(t *testing.T) {
	cart := CreateCart("Abc12345de-Q")
	AddItem(&cart, "apple", 5)
	
	success := UpdateItem(&cart, "apple", -1)
	if success {
		t.Errorf("UpdateItem should have failed for invalid quantity -1")
	}
}

func TestUpdateItemInvalidName(t *testing.T) {
	cart := CreateCart("Abc12345de-Q")
	AddItem(&cart, "apple", 5)
	
	success := UpdateItem(&cart, "apple123", 10)
	if success {
		t.Errorf("UpdateItem should have failed for invalid item name")
	}
}

func TestRemoveItemValid(t *testing.T) {
	cart := CreateCart("Abc12345de-Q")
	AddItem(&cart, "apple", 5)
	
	success := RemoveItem(&cart, "apple")
	if !success {
		t.Errorf("RemoveItem failed for existing item 'apple'")
	}
	
	items := GetItems(cart)
	if _, exists := items["apple"]; exists {
		t.Errorf("RemoveItem did not remove item 'apple' from cart")
	}
}

func TestRemoveItemNonExistent(t *testing.T) {
	cart := CreateCart("Abc12345de-Q")
	
	success := RemoveItem(&cart, "apple")
	if success {
		t.Errorf("RemoveItem should have failed for non-existent item 'apple'")
	}
}

func TestRemoveItemInvalidName(t *testing.T) {
	cart := CreateCart("Abc12345de-Q")
	
	success := RemoveItem(&cart, "apple123")
	if success {
		t.Errorf("RemoveItem should have failed for invalid item name")
	}
}

func TestRemoveItemMultipleItems(t *testing.T) {
	cart := CreateCart("Abc12345de-Q")
	AddItem(&cart, "apple", 3)
	AddItem(&cart, "orange", 2)
	
	RemoveItem(&cart, "apple")
	
	items := GetItems(cart)
	if _, exists := items["apple"]; exists {
		t.Errorf("RemoveItem did not remove 'apple'")
	}
	if items["orange"] != 2 {
		t.Errorf("RemoveItem affected other items in the cart")
	}
}