const P =
  100392089237316158323570985008687907853269981005640569039457584007913129640081n;
const Q =
  90392089237316158323570985008687907853269981005640569039457584007913129640041n;
const E = 65537n;
const BLOCK_SIZE = 60;
const MODULUS_BYTES = 64;
const DEFAULT_MESSAGE = "Scaramouche, Scaramouche, will you do the Fandango? ";

function egcd(a, b) {
  if (b === 0n) {
    return [a, 1n, 0n];
  }
  const [g, x1, y1] = egcd(b, a % b);
  return [g, y1, x1 - (a / b) * y1];
}

function modInverse(a, m) {
  const [g, x] = egcd(a, m);
  if (g !== 1n) {
    throw new Error("No modular inverse exists.");
  }
  return (x % m + m) % m;
}

function modPow(base, exponent, modulus) {
  let result = 1n;
  let factor = base % modulus;
  let power = exponent;
  while (power > 0n) {
    if (power & 1n) {
      result = (result * factor) % modulus;
    }
    factor = (factor * factor) % modulus;
    power >>= 1n;
  }
  return result;
}

function pkcs7Pad(buffer, blockSize) {
  const remainder = buffer.length % blockSize;
  const padLength = remainder === 0 ? blockSize : blockSize - remainder;
  return Buffer.concat([buffer, Buffer.alloc(padLength, padLength)]);
}

function pkcs7Unpad(buffer) {
  const padLength = buffer[buffer.length - 1];
  if (padLength < 1 || padLength > BLOCK_SIZE) {
    throw new Error("Invalid PKCS#7 padding.");
  }
  for (let i = buffer.length - padLength; i < buffer.length; i += 1) {
    if (buffer[i] !== padLength) {
      throw new Error("Invalid PKCS#7 padding.");
    }
  }
  return buffer.subarray(0, buffer.length - padLength);
}

function chunkBuffer(buffer, size) {
  const chunks = [];
  for (let offset = 0; offset < buffer.length; offset += size) {
    chunks.push(buffer.subarray(offset, offset + size));
  }
  return chunks;
}

function blockToBigInt(block) {
  return BigInt(`0x${block.toString("hex")}`);
}

function bigIntToBuffer(value, size) {
  return Buffer.from(value.toString(16).padStart(size * 2, "0"), "hex");
}

const N = P * Q;
const PHI = (P - 1n) * (Q - 1n);
const D = modInverse(E, PHI);

function encryptMessage(message) {
  const plaintext = Buffer.from(message, "utf8");
  const padded = pkcs7Pad(plaintext, BLOCK_SIZE);
  const ciphertext = chunkBuffer(padded, BLOCK_SIZE).map((block) =>
    bigIntToBuffer(modPow(blockToBigInt(block), E, N), MODULUS_BYTES),
  );
  return {
    plaintext,
    padded,
    ciphertext,
    ciphertextHex: Buffer.concat(ciphertext).toString("hex"),
  };
}

function decryptCiphertext(ciphertextHex) {
  const ciphertext = Buffer.from(ciphertextHex, "hex");
  if (ciphertext.length % MODULUS_BYTES !== 0) {
    throw new Error("Ciphertext length must be a multiple of 64 bytes.");
  }
  const paddedPlaintext = Buffer.concat(
    chunkBuffer(ciphertext, MODULUS_BYTES).map((block) =>
      bigIntToBuffer(modPow(blockToBigInt(block), D, N), BLOCK_SIZE),
    ),
  );
  return {
    paddedPlaintext,
    plaintext: pkcs7Unpad(paddedPlaintext).toString("utf8"),
  };
}

function printReport(message) {
  const encrypted = encryptMessage(message);
  const decrypted = decryptCiphertext(encrypted.ciphertextHex);

  console.log(`N: ${N}`);
  console.log(`d: ${D}`);
  console.log(`message: ${JSON.stringify(message)}`);
  console.log(`plaintext bytes: ${encrypted.plaintext.length}`);
  console.log(`padded block hex: ${encrypted.padded.toString("hex")}`);
  console.log(`ciphertext hex: ${encrypted.ciphertextHex}`);
  console.log(`decrypted padded hex: ${decrypted.paddedPlaintext.toString("hex")}`);
  console.log(`recovered plaintext: ${JSON.stringify(decrypted.plaintext)}`);
}

const args = process.argv.slice(2);

try {
  if (args.length === 0) {
    printReport(DEFAULT_MESSAGE);
  } else if (args[0] === "--message" && args[1]) {
    printReport(args[1]);
  } else if (args[0] === "--cipher" && args[1]) {
    const decrypted = decryptCiphertext(args[1]);
    console.log(`decrypted padded hex: ${decrypted.paddedPlaintext.toString("hex")}`);
    console.log(`recovered plaintext: ${JSON.stringify(decrypted.plaintext)}`);
  } else {
    console.error(`Usage:
  node rsa512.mjs
  node rsa512.mjs --message "text"
  node rsa512.mjs --cipher <hex>`);
    process.exit(1);
  }
} catch (error) {
  console.error(`RSA simulation failed: ${error.message}`);
  process.exit(1);
}
