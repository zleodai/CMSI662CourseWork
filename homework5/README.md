# Crystal Vault

Ultra safe super secure crystal vault for all your intergalatic trade needs. Users can transfer crystals to another user's vault.

## Run locally

```powershell
pip install -r requirements.txt
python .\bin\createdb.py
python .\bin\makeaccounts.py
flask --app app run
```

Then visit `http://127.0.0.1:5000`.


Demo users:

| Email | Password |
| --- | --- |
| `alice@example.com` | `123456` |
| `bob@example.com` | `123456` |

New users can register from the login page :)