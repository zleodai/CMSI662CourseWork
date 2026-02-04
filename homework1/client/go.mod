module cartmodule/client

go 1.22.5

replace cartmodule/cart => ../cart

replace cartmodule/server => ../server

require cartmodule/cart v0.0.0-00010101000000-000000000000

require (
	cartmodule/server v0.0.0-00010101000000-000000000000 // indirect
	github.com/google/uuid v1.6.0 // indirect
)
