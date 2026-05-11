import os
import re
import sqlite3


# lets tests use a temp database
DB_PATH = os.environ.get("BANK_DB_PATH", "bank.db")

# checks account ids before using them
ACCOUNT_ID_PATTERN = re.compile(r"^\d{3,12}$")

# checks recipient emails before transfer
OWNER_EMAIL_PATTERN = re.compile(r"^[^@\s]{1,64}@[^@\s]{1,190}\.[^@\s]{2,63}$")

# limits one transfer amount
MAX_TRANSFER_AMOUNT = 1000

GENERIC_TRANSFER_ERROR = (
    "Transfer could not be completed. Check the vault details and try again."
)


def _connect():
    con = sqlite3.connect(DB_PATH)
    # turns on foreign key checks
    con.execute("PRAGMA foreign_keys = ON")
    return con


def normalize_account_id(account_number):
    return (account_number or "").strip()


def is_valid_account_id(account_number):
    return bool(ACCOUNT_ID_PATTERN.fullmatch(normalize_account_id(account_number)))


def normalize_owner_email(email):
    return (email or "").strip().lower()


def is_valid_owner_email(email):
    return bool(OWNER_EMAIL_PATTERN.fullmatch(normalize_owner_email(email)))


def parse_transfer_amount(raw_amount):
    """Validate the amount field and return an int safe for SQL parameters."""
    amount_text = (raw_amount or "").strip()
    if not amount_text:
        return None, "Enter a whole number of crystals."
    if not amount_text.isdecimal():
        return None, "Transfer amount must be a positive whole number."

    amount = int(amount_text)
    if amount <= 0:
        return None, "Transfer amount must be at least 1 crystal."
    if amount > MAX_TRANSFER_AMOUNT:
        return None, f"Transfers are limited to {MAX_TRANSFER_AMOUNT} crystals."
    return amount, None


def confirmation_was_accepted(raw_confirmation):
    # checks the warning box on the server
    return raw_confirmation == "yes"


def get_vault(owner):
    """Return the authenticated user's single vault."""
    owner = normalize_owner_email(owner)
    if not is_valid_owner_email(owner):
        return None

    try:
        con = _connect()
        cur = con.cursor()
        # prevents sql injection by using placeholders
        cur.execute(
            """
            SELECT id, balance
            FROM accounts
            WHERE owner=?
            ORDER BY id
            LIMIT 1
            """,
            (owner,),
        )
        row = cur.fetchone()
        if row is None:
            return None
        if not is_valid_account_id(row[0]):
            return None
        return {"id": row[0], "balance": row[1]}
    finally:
        con.close()


def do_transfer(recipient_owner, amount, owner):
    """Atomically move crystals from the logged-in user's vault to another user.

    The source vault is never supplied by the browser; it is derived from the
    authenticated JWT subject. Recipient errors are intentionally generic so
    attackers cannot enumerate registered emails or discover who has a vault by
    watching transfer error text.
    """
    owner = normalize_owner_email(owner)
    recipient_owner = normalize_owner_email(recipient_owner)
    if not is_valid_owner_email(owner) or not is_valid_owner_email(recipient_owner):
        return False, GENERIC_TRANSFER_ERROR
    if owner == recipient_owner:
        return False, "Choose another user's vault."
    if not isinstance(amount, int) or amount <= 0 or amount > MAX_TRANSFER_AMOUNT:
        return False, "Transfer amount is outside the allowed range."

    con = _connect()
    try:
        cur = con.cursor()
        # locks the database before changing balances
        cur.execute("BEGIN IMMEDIATE")

        cur.execute(
            """
            SELECT id, balance
            FROM accounts
            WHERE owner=?
            ORDER BY id
            LIMIT 1
            """,
            (owner,),
        )
        source_row = cur.fetchone()
        if source_row is None:
            con.rollback()
            return False, GENERIC_TRANSFER_ERROR

        cur.execute(
            """
            SELECT id
            FROM accounts
            WHERE owner=?
            ORDER BY id
            LIMIT 1
            """,
            (recipient_owner,),
        )
        target_row = cur.fetchone()
        if target_row is None:
            con.rollback()
            return False, GENERIC_TRANSFER_ERROR

        source_id, source_balance = source_row
        target_id = target_row[0]
        if not is_valid_account_id(source_id) or not is_valid_account_id(target_id):
            con.rollback()
            return False, GENERIC_TRANSFER_ERROR

        if source_balance < amount:
            con.rollback()
            return False, "That vault does not hold enough crystals."

        # prevents negative balances
        cur.execute(
            """
            UPDATE accounts
            SET balance=balance-?
            WHERE id=? AND owner=? AND balance>=?
            """,
            (amount, source_id, owner, amount),
        )
        if cur.rowcount != 1:
            con.rollback()
            return False, GENERIC_TRANSFER_ERROR

        cur.execute(
            """
            UPDATE accounts
            SET balance=balance+?
            WHERE id=? AND owner=?
            """,
            (amount, target_id, recipient_owner),
        )
        if cur.rowcount != 1:
            con.rollback()
            return False, GENERIC_TRANSFER_ERROR

        con.commit()
        return True, None
    except sqlite3.Error:
        # hides database errors from users
        con.rollback()
        return False, GENERIC_TRANSFER_ERROR
    finally:
        con.close()
