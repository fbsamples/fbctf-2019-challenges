import hashlib

from flask import Flask, make_response, redirect, render_template, request
from flask.sessions import SecureCookieSessionInterface
from flask_login import LoginManager, UserMixin, current_user, login_user, logout_user
from flask_sqlalchemy import SQLAlchemy

app = Flask(__name__)
app.config["SESSION_COOKIE_NAME"] = "events_sesh_cookie"
app.config["SQLALCHEMY_DATABASE_URI"] = "sqlite:///my.db"
app.config["SQLALCHEMY_TRACK_MODIFICATIONS"] = False
app.config["SECRET_KEY"] = "fb+wwn!n1yo+9c(9s6!_3o#nqm&&_ej$tez)$_ik36n8d7o6mr#y"
SESSION_TYPE = "sqlalchemy"
db = SQLAlchemy(app)
login = LoginManager(app)


# COOKIE UTILS
def unsign(val):
    scsi = SecureCookieSessionInterface()
    ser = scsi.get_signing_serializer(app)
    return ser.loads(val)


def sign(val):
    scsi = SecureCookieSessionInterface()
    ser = scsi.get_signing_serializer(app)
    return ser.dumps(val)


# END COOKIE UTILS


# MODEL
class User(db.Model, UserMixin):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(100), unique=True, nullable=False)
    password = db.Column(db.String(255), nullable=False)
    events = db.relationship("Event", backref="owner", lazy=True)

    def __repr__(self):
        return "%s" % self.username


class Event(db.Model, object):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(100), nullable=False)
    address = db.Column(db.String(100), nullable=False)
    show = db.Column(db.String(200), nullable=False)
    owner_id = db.Column(db.Integer, db.ForeignKey("user.id"), nullable=False)
    fmt = db.Column(db.String(300), nullable=False)

    def __repr__(self):
        return "Just another event"


# END MODEL
db.create_all()


@login.user_loader
def load_user(id):
    return User.query.get(int(id))


@app.route("/", methods=["GET", "POST"])
def home():
    if request.method == "POST" and current_user.is_authenticated:
        event_name = request.form["event_name"]
        event_address = request.form["event_address"]
        event_important = request.form.get("event_important")

        if event_name and event_address and event_important:
            fmt = "{0.%s}" % event_important

            e = Event(
                name=event_name,
                address=event_address,
                show=event_important,
                fmt=fmt,
                owner_id=current_user.id,
            )

            try:
                e.fmt.format(e)
            except Exception:
                return redirect("/")

            db.session.add(e)
            db.session.commit()
    if current_user.is_authenticated:
        events = (
            Event.query.filter(Event.owner_id == current_user.id)
            .order_by(Event.id.desc())
            .limit(100)
            .all()
        )
    else:
        events = []

    if events:
        events[0].__init__.__globals__["app"] = app
    return render_template("index.html", current_user=current_user, events=events)


@app.route("/login", methods=["POST"])
def login():
    if request.method == "POST":
        username = request.form.get("username", None)
        password = request.form.get("password", None)

        if username == "admin":
            return redirect("/")

        if username and password:
            if User.query.filter_by(username=username).first() is None:
                user = User(
                    username=username,
                    password=hashlib.sha256(str(password).encode("utf-8")).hexdigest(),
                )
                db.session.add(user)
                db.session.commit()

            else:
                user = User.query.filter_by(
                    username=username,
                    password=hashlib.sha256(str(password).encode("utf-8")).hexdigest(),
                ).first()

            if user is not None:
                login_user(user)
                response = make_response(redirect("/"))
                response.set_cookie("user", sign(username))
                return response
            else:
                return redirect("/")
        else:
            return redirect("/")


@app.route("/logout")
def logout():
    logout_user()
    response = make_response(redirect("/"))
    for cookie_name in request.cookies.keys():
        response.delete_cookie(cookie_name)
    response.delete_cookie("session")
    return response


@app.route("/flag")
def get_flag():
    if not current_user.is_authenticated:
        return render_template("admin.html", text="You have to login first.")

    user_cookie_present = request.cookies.get("user")

    try:
        user = unsign(user_cookie_present)
    except Exception:
        return render_template("admin.html", text="I do not recognise this user.")

    if user != "admin":
        return render_template(
            "admin.html",
            text="You do not seem to be an admin, {}!".format(current_user.username),
        )
    return render_template("admin.html", text=open("./flag").read())


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=80, debug=False)
