package cart

import (
	"testing"
)

func TestCartValidUserCreation(t *testing.T) {
	validUsers := []string{
		"Abc12345de-Q",
		"Cde45678cc-A",
		"cdg45268dz-A",
		"cze53678qq-Q",
	}

	for _, user := range validUsers {
		_, success := NewCart(user)
		if !success {
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
		_, success := NewCart(user)
		if success {
			t.Errorf("NewCart created with invalid customer ID: %s", user)
		}
	}
}

func TestCardValidCreation(t *testing.T) {
	validUsername := "Abc12345de-Q"
	cart, success := NewCart(validUsername)
	if !success {
		t.Errorf("NewCart failed with valid customer ID: %s", validUsername)
	}
	if cart.customerID != validUsername {
		t.Errorf("NewCart created cart with incorrect customer ID. Expected: %s, Got: %s", validUsername, cart.customerID)
	}
}

func TestUniqueID(t *testing.T) {
	cart1, success1 := NewCart("Abc12345de-Q")
	cart2, success2 := NewCart("Cde45678cc-A")

	if !success1 || !success2 {
		t.Errorf("NewCart failed to create carts with valid customer IDs")
	}

	if cart1.id == cart2.id {
		t.Errorf("NewCart created carts with duplicate IDs: %s", cart1.id)
	}
}

func TestDefensiveCopy(t *testing.T) {
	cart, success := NewCart("Abc12345de-Q")
	if !success {
		t.Errorf("NewCart failed with valid customer ID: %s", "Abc12345de-Q")
	}

	items := GetItems(cart)
	items["apple"] = 5

	if (&items == &cart.items) {
		t.Errorf("GetItems did not return a defensive copy of the items map")
	}
}