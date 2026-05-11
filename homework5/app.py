import os

from flask import Flask, abort, g, make_response, redirect, render_template, request, url_for
from flask_wtf.csrf import CSRFError, CSRFProtect

from account_service import (
    GENERIC_TRANSFER_ERROR,
    confirmation_was_accepted,
    do_transfer,
    get_vault,
    is_valid_owner_email,
    normalize_owner_email,
    parse_transfer_amount,
)
from user_service import get_user_with_credentials, logged_in, register_user

app = Flask(__name__)

# signs csrf tokens
app.config['SECRET_KEY'] = os.environ.get(
    "FLASK_SECRET_KEY",
    "dev-only-change-me-yoursupersecrettokenhere",
)
app.config['SESSION_COOKIE_HTTPONLY'] = True
app.config['SESSION_COOKIE_SAMESITE'] = 'Lax'
app.config['WTF_CSRF_TIME_LIMIT'] = 3600
app.config['MAX_CONTENT_LENGTH'] = 16 * 1024
csrf = CSRFProtect(app)


@app.after_request
def add_security_headers(response):
    # helps prevent xss attacks by preventing script usage and preventing other attack vectors.
    response.headers['Content-Security-Policy'] = (
        "default-src 'self'; "
        "script-src 'none'; "
        "style-src 'self'; "
        "img-src 'self'; "
        "form-action 'self'; "
        "base-uri 'self'; "
        "frame-ancestors 'none'"
    )
    response.headers['X-Content-Type-Options'] = 'nosniff'
    response.headers['X-Frame-Options'] = 'DENY'
    response.headers['Referrer-Policy'] = 'no-referrer'
    response.headers['Cache-Control'] = 'no-store'
    return response


def login_response(status=200, error=None):
    return render_template("login.html", error=error), status


def register_response(status=200, error=None, values=None):
    return render_template("register.html", error=error, values=values or {}), status


def signed_in_response(user):
    response = make_response(redirect("/dashboard"))

    # stores jwt in a safe cookie
    response.set_cookie(
        "auth_token",
        user["token"],
        httponly=True,
        samesite="Lax",
        secure=request.is_secure,
        max_age=3600,
    )
    return response, 303


def require_login():
    if logged_in():
        return True
    return False


@app.route("/", methods=['GET'])
def home():
    if not require_login():
        return render_template("login.html")
    return redirect('/dashboard')


@app.route("/login", methods=["POST"])
def login():
    email = request.form.get("email")
    password = request.form.get("password")
    user = get_user_with_credentials(email, password)
    if not user:
        # prevents user enumeration with one error message
        return login_response(401, "Invalid email or password.")
    return signed_in_response(user)


@app.route("/register", methods=["GET"])
def register_form():
    if require_login():
        return redirect("/dashboard")
    return register_response()


@app.route("/register", methods=["POST"])
def register():
    if require_login():
        return redirect("/dashboard")

    values = {
        "email": request.form.get("email", ""),
        "name": request.form.get("name", ""),
    }
    user, error = register_user(
        request.form.get("email"),
        request.form.get("name"),
        request.form.get("password"),
        request.form.get("confirm_password"),
    )
    if error:
        return register_response(400, error, values)
    return signed_in_response(user)


@app.route("/dashboard", methods=['GET'])
def dashboard():
    if not require_login():
        return render_template("login.html")
    transferred = request.args.get("transferred") == "1"
    return render_template(
        "dashboard.html",
        email=g.user,
        vault=get_vault(g.user),
        transferred=transferred,
    )


@app.route("/transfer", methods=["GET"])
def transfer_form():
    if not require_login():
        return render_template("login.html")
    return render_template(
        "transfer.html",
        email=g.user,
        vault=get_vault(g.user),
        values={},
    )


@app.route("/transfer", methods=["POST"])
def transfer():
    if not require_login():
        return render_template("login.html")

    recipient = normalize_owner_email(request.form.get("recipient"))
    amount, amount_error = parse_transfer_amount(request.form.get("amount"))
    confirmed = confirmation_was_accepted(request.form.get("confirm_transfer"))
    values = {
        "recipient": recipient,
        "amount": request.form.get("amount", ""),
        "confirm_transfer": request.form.get("confirm_transfer"),
    }

    # validates transfer input on the server
    if amount_error:
        return render_transfer_error(amount_error, values)
    if not is_valid_owner_email(recipient):
        return render_transfer_error("Enter a valid recipient email.", values)
    if not confirmed:
        return render_transfer_error(
            "Confirm the warning before transferring crystals.",
            values,
        )

    if get_vault(g.user) is None:
        return render_transfer_error(GENERIC_TRANSFER_ERROR, values)

    ok, error = do_transfer(recipient, amount, g.user)
    if not ok:
        return render_transfer_error(error, values)

    response = make_response(redirect(url_for("dashboard", transferred="1")))
    return response, 303


def render_transfer_error(error, values):
    return (
        render_template(
            "transfer.html",
            email=g.user,
            vault=get_vault(g.user),
            error=error,
            values=values,
        ),
        400,
    )


@app.route("/logout", methods=['GET'])
def logout():
    response = make_response(redirect("/dashboard"))
    # deletes jwt cookie
    response.delete_cookie('auth_token', samesite="Lax", secure=request.is_secure)
    return response, 303


@app.errorhandler(CSRFError)
def csrf_error(error):
    # handles csrf errors
    return render_template(
        "error.html",
        title="Request blocked",
        message="The form expired or was submitted without a valid CSRF token.",
    ), 400


@app.errorhandler(400)
def bad_request(error):
    return render_template(
        "error.html",
        title="Bad request",
        message="The request could not be processed.",
    ), 400


@app.errorhandler(404)
def not_found(error):
    return render_template(
        "error.html",
        title="Not found",
        message="That vault was not found.",
    ), 404
