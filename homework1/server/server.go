package server

var itemCosts = map[string]float64{
	"apple":    1.31,
	"banana":   0.60,
	"orange":   0.80,
	"avacado":  1.66,
	"potato":   0.92,
	"broccoli": 1.16,
	"milk":     3.71,
	"eggs":     3.59,
	"beef":     6.63,
	"chicken":  4.10,
	"bread":    1.84,
}

func GetItemCost(item string) (float64, bool) {
	cost, contains := itemCosts[item]
	if contains {
		return cost, true
	} else {
		return 0.00, false
	}
}

func IsValidItem(item string) bool {
	_, contains := itemCosts[item]

	return contains
}