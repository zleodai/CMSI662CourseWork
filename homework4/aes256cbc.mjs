import crypto from "node:crypto";

const usage = `Usage:
  node aes256cbc.mjs -e "message" <32-byte-key> <16-byte-iv>
  node aes256cbc.mjs -d <hex-ciphertext> <32-byte-key> <16-byte-iv>`;

function fail(message) {
  console.error(message);
  console.error(usage);
  process.exit(1);
}

const args = process.argv.slice(2);

if (args.length !== 4) {
  fail("Expected exactly four command line arguments.");
}

const [mode, input, key, iv] = args;

if (!["-e", "-d"].includes(mode)) {
  fail("The first argument must be -e or -d.");
}

const keyBytes = Buffer.from(key, "utf8");
const ivBytes = Buffer.from(iv, "utf8");

if (keyBytes.length !== 32) {
  fail(`AES-256-CBC requires a 32-byte key, but received ${keyBytes.length} bytes.`);
}

if (ivBytes.length !== 16) {
  fail(`AES-CBC requires a 16-byte initialization vector, but received ${ivBytes.length} bytes.`);
}

try {
  if (mode === "-e") {
    const cipher = crypto.createCipheriv("aes-256-cbc", keyBytes, ivBytes);
    const ciphertext = Buffer.concat([cipher.update(input, "utf8"), cipher.final()]);
    process.stdout.write(`${ciphertext.toString("hex")}\n`);
  } else {
    if (!/^[0-9a-fA-F]+$/.test(input) || input.length % 2 !== 0) {
      fail("Ciphertext must be an even-length hexadecimal string.");
    }
    const decipher = crypto.createDecipheriv("aes-256-cbc", keyBytes, ivBytes);
    const plaintext = Buffer.concat([
      decipher.update(Buffer.from(input, "hex")),
      decipher.final(),
    ]);
    process.stdout.write(`${plaintext.toString("utf8")}\n`);
  }
} catch (error) {
  fail(`Crypto operation failed: ${error.message}`);
}
