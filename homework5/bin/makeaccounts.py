import os
import sqlite3

DB_PATH = os.environ.get("BANK_DB_PATH", "bank.db")

con = sqlite3.connect(DB_PATH)
cur = con.cursor()
cur.execute('''
    CREATE TABLE accounts (
        id text primary key,
        owner text not null unique,
        balance integer not null check(balance >= 0),
        foreign key(owner) references users(email))''')
# makes one vault for each user
cur.execute(
    "INSERT INTO accounts VALUES (?, ?, ?)",
    ('100', 'alice@example.com', 7700))
cur.execute(
    "INSERT INTO accounts VALUES (?, ?, ?)",
    ('998', 'bob@example.com', 1000))
con.commit()
con.close()
