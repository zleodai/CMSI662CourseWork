import os
import re
import sqlite3
import secrets
from datetime import datetime, timedelta, timezone
from passlib.hash import pbkdf2_sha256
from flask import request, g
import jwt

DB_PATH = os.environ.get("BANK_DB_PATH", "bank.db")

# signs jwt tokens
SECRET = os.environ.get(
    "JWT_SECRET",
    "dev-only-change-me-bfg28y7efg238re7r6t32gfo23vfy7237yibdyo238do2v3",
)

EMAIL_PATTERN = re.compile(r"^[^@\s]{1,64}@[^@\s]{1,190}\.[^@\s]{2,63}$")
NAME_PATTERN = re.compile(r"^[A-Za-z][A-Za-z .'-]{0,79}$")
MAX_PASSWORD_LENGTH = 256
MIN_PASSWORD_LENGTH = 8
STARTING_CRYSTALS = 500

# helps prevent user enumeration
DUMMY_PASSWORD_HASH = (
    "$pbkdf2-sha256$29000$w5hzTmnNOYew9n6P8T4nJA$"
    "VrQ43pEpTBgVQRfMWg2G1fKSbwcZ/dBTS2n3RyX1yW0"
)


def _connect():
    con = sqlite3.connect(DB_PATH)
    con.execute("PRAGMA foreign_keys = ON")
    return con


def _normalize_email(email):
    return (email or "").strip().lower()


def _valid_email_shape(email):
    return bool(EMAIL_PATTERN.fullmatch(email))


def _normalize_name(name):
    return " ".join((name or "").strip().split())


def _valid_name_shape(name):
    return bool(NAME_PATTERN.fullmatch(name))


def _password_validation_error(password, confirmation):
    password = password or ""
    confirmation = confirmation or ""
    if len(password) < MIN_PASSWORD_LENGTH:
        return f"Password must be at least {MIN_PASSWORD_LENGTH} characters."
    if len(password) > MAX_PASSWORD_LENGTH:
        return "Password is too long."
    if password != confirmation:
        return "Passwords must match."
    return None


def _new_account_id(cur):
    for _ in range(20):
        account_id = f"{secrets.randbelow(900000) + 100000}"
        cur.execute("SELECT 1 FROM accounts WHERE id=?", (account_id,))
        if cur.fetchone() is None:
            return account_id
    raise sqlite3.IntegrityError("could not allocate account id")


def register_user(email, name, password, confirmation):
    """Create a user and their one vault while storing only a salted PBKDF2 hash."""
    email = _normalize_email(email)
    name = _normalize_name(name)
    password_error = _password_validation_error(password, confirmation)

    if not _valid_email_shape(email):
        return None, "Enter a valid email address."
    if not _valid_name_shape(name):
        return None, "Enter a name using letters, spaces, apostrophes, periods, or hyphens."
    if password_error:
        return None, password_error

    con = _connect()
    try:
        cur = con.cursor()
        cur.execute("BEGIN IMMEDIATE")

        # makes salted pbkdf2 hash
        password_hash = pbkdf2_sha256.hash(password)
        account_id = _new_account_id(cur)

        # prevents sql injection by using placeholders
        cur.execute(
            """
            INSERT INTO users (email, name, password)
            VALUES (?, ?, ?)
            """,
            (email, name, password_hash),
        )
        cur.execute(
            """
            INSERT INTO accounts (id, owner, balance)
            VALUES (?, ?, ?)
            """,
            (account_id, email, STARTING_CRYSTALS),
        )
        con.commit()
        return {"email": email, "name": name, "token": create_token(email)}, None
    except sqlite3.IntegrityError:
        con.rollback()
        # prevents registration enumeration with one error
        return None, "Registration could not be completed."
    except sqlite3.Error:
        con.rollback()
        return None, "Registration could not be completed."
    finally:
        con.close()


def get_user_with_credentials(email, password):
    email = _normalize_email(email)
    password = password or ""
    # prevents huge passwords
    if len(password) > MAX_PASSWORD_LENGTH:
        # helps prevent user enumeration
        pbkdf2_sha256.verify("", DUMMY_PASSWORD_HASH)
        return None

    row = None
    password_hash = DUMMY_PASSWORD_HASH
    try:
        con = _connect()
        if _valid_email_shape(email):
            cur = con.cursor()
            # prevents sql injection by using placeholders
            cur.execute(
                """
                SELECT email, name, password
                FROM users
                WHERE email=?
                """,
                (email,),
            )
            row = cur.fetchone()
            if row is not None:
                password_hash = row[2]
    finally:
        con.close()

    try:
        password_is_valid = pbkdf2_sha256.verify(password, password_hash)
    except (TypeError, ValueError):
        password_is_valid = False

    if row is None or not password_is_valid:
        # helps prevent user enumeration
        pbkdf2_sha256.verify("", DUMMY_PASSWORD_HASH)
        return None

    email, name, _ = row
    return {"email": email, "name": name, "token": create_token(email)}


def logged_in():
    token = request.cookies.get('auth_token')
    if not token:
        return False
    try:
        data = jwt.decode(token, SECRET, algorithms=['HS256'])
        subject = data.get('sub')
        if not _valid_email_shape(subject or ""):
            return False
        # saves the logged in user for this request
        g.user = subject
        return True
    except jwt.InvalidTokenError:
        return False


def create_token(email):
    now = datetime.now(timezone.utc)
    payload = {'sub': email, 'iat': now, 'exp': now + timedelta(minutes=60)}
    
    # creates jwt token
    token = jwt.encode(payload, SECRET, algorithm='HS256')
    return token
