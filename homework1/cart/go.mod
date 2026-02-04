module cartmodule/cart

go 1.22.5

replace cartmodule/server => ../server

require (
	cartmodule/server v0.0.0-00010101000000-000000000000
	github.com/google/uuid v1.6.0
)
