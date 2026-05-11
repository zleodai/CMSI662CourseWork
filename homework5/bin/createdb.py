import os
import sqlite3
from passlib.hash import pbkdf2_sha256

DB_PATH = os.environ.get("BANK_DB_PATH", "bank.db")

con = sqlite3.connect(DB_PATH)
cur = con.cursor()
cur.execute('''
    CREATE TABLE users (
        email text primary key,
        name text not null,
        password text not null)''')
# makes salted password hashes
cur.execute(
    "INSERT INTO users VALUES (?, ?, ?)",
    ('alice@example.com', 'Alice Xu', pbkdf2_sha256.hash("123456")))
cur.execute(
    "INSERT INTO users VALUES (?, ?, ?)",
    ('bob@example.com', 'Bobby Tables', pbkdf2_sha256.hash("123456")))
con.commit()
con.close()
