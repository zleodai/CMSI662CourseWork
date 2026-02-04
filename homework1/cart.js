import { randomUUID } from 'crypto';

export class Cart {
    #id;  // Makes the id a private field
    constructor(customerId) {
        this.#id = randomUUID();
        this.customerId = customerId;
        this.items = [];
        Object.freeze(this); // Makes the instance immutable
    }
}

const cart = new Cart('customer123');
console.log(cart);