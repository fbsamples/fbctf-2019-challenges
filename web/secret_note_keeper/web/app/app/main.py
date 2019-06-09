from flask import Flask, render_template, request, redirect, make_response
import hashlib
from flask_sqlalchemy import SQLAlchemy
from flask_login import LoginManager, UserMixin, current_user, login_user, logout_user
import hmac
import time
import random
import sys
import string
import random
import hashlib
import re

def check(inp, prefix):
    if not re.match('^[a-zA-Z0-9]+$', inp):
        return False
    digest = hashlib.md5(inp.encode("utf-8")).hexdigest()
    return prefix == digest[:len(prefix)]

ALPHA = "0123456789abcdef" #string.ascii_lowercase + string.ascii_uppercase + string.digits
def rand_string(length):
    return ''.join([random.choice(ALPHA) for _ in range(length)])

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'mysql://chal1:getbvllnbunnhriunuglchillki@127.0.0.1/notes'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
SESSION_TYPE = 'sqlalchemy'
app.secret_key = 'yAcFlCxR97w20Qyc#f4$AUNcG3ZTvG&g*RZTz%8tNZ0q0eBHlOL35EP3EPtE$jLJx001eGV&&hU*RE'
db = SQLAlchemy(app)
login = LoginManager(app)


#################################################
# models
class User(db.Model, UserMixin):
    id = db.Column(db.Integer, primary_key=True)
    title = db.Column(db.String(255), nullable=False, default="Mr")
    username = db.Column(db.String(100), unique=True, nullable=False)
    password = db.Column(db.String(255), nullable=False)
    notes = db.relationship('Note', backref='owner', lazy=True)
    bugs = db.relationship('Bug', backref='owner', lazy=True)

    def __repr__(self):
        return '%s' % self.username

class Note(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    title = db.Column(db.String(100), nullable=False)
    body = db.Column(db.Text, nullable=False)
    owner_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)

    def __repr__(self):
        return '<Post %r>' % self.title

class Bug(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    title = db.Column(db.String(100), nullable=False)
    body = db.Column(db.Text, nullable=False)
    link = db.Column(db.String(100), nullable=False)
    owner_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)
    reviewed = db.Column(db.Boolean, default=False)



@login.user_loader
def load_user(id):
    return User.query.get(int(id))


@app.route("/")
def home():
    return render_template("index.html", current_user=current_user)


@app.route("/login", methods=['POST'])
def login():
    if request.method == "POST":
        title = request.form.get("title", "Mr")
        username = request.form.get('username', None)
        password = request.form.get('password', None)
        if username and password:
            if User.query.filter_by(username=username).first() is None:
                user = User(username=username, password=hashlib.sha256(str(password).encode('utf-8')).hexdigest(), title=title)
                db.session.add(user)
                db.session.commit()
                login_user(user)
                return redirect('/notes')
            else:
                user = User.query.filter_by(username=username, password=hashlib.sha256(str(password).encode('utf-8')).hexdigest()).first()
                if user is not None:
                    login_user(user)
                    return redirect('/notes')
                else:
                    return 'Failed', 500

@app.route("/notes", methods=['GET', 'POST'])
def get_notes():
    if current_user.is_authenticated:
        if request.method == "GET":
            notes = current_user.notes
            return render_template("notes.html", notes=notes)
        elif request.method == "POST":
            title = request.form.get('title', None)
            body = request.form.get('body', None)
            if "fb{" in body:
                return redirect('/notes')
            user_note = Note(title=title, body=body, owner_id=current_user.id)
            db.session.add(user_note)
            db.session.commit()
            return redirect('/notes')

    return render_template("notes.html", current_user=current_user)


@app.route("/report_bugs", methods=['GET', 'POST'])
def report_bugs():
    if current_user.is_authenticated:
        MAC_KEY = "kfggfgiihuerbtjgrjrABCDD"
        if request.method == "GET":
            bugs = Bug.query.filter(Bug.owner_id == current_user.id).order_by(Bug.id.desc()).limit(100).all()

            timeout = int(time.time()) + 120
            rand_val = rand_string(5)
            real_pow = "%s_%s" %(rand_val, str(timeout))
            mac = hmac.HMAC(bytes(MAC_KEY, "utf-8"), bytes(real_pow, "utf-8")).hexdigest()
            resp = make_response(render_template("report_bugs.html", bugs=bugs, real_pow=real_pow.split("_")[0]))
            resp.set_cookie('pow', real_pow+":"+mac)
            return resp
        elif request.method == "POST":
            # Read the pow cookie
            try:
                user_pow, user_mac = request.cookies.get('pow').split(":")
                real_mac = hmac.HMAC(bytes(MAC_KEY, "utf-8"), bytes(user_pow, "utf-8")).hexdigest()
                if real_mac != user_mac:
                    return 'pow is incorrect', 500
                try:
                    timeout = int(user_pow.split("_")[1])
                except Exception as e:
                    print("inside casting user_pow timeout")
                    print(e)
                    return 'pow is incorrect', 500

                if timeout < int(time.time()):
                    return 'pow is old!', 500
                # if solve(pow, res) != True:
                #    exit
                pow_sol = request.form.get('pow_sol', None)
                if pow_sol is None:
                    return 'pow is incorrect', 500
                if check(pow_sol, user_pow.split("_")[0]) == False:
                     return 'pow is incorrect', 500

                title = request.form.get('title', None)
                body = request.form.get('body', None)
                # TODO do some validation to make sure that this is either empty or absoulte URI
                # or relative uri make next inside the login that is vulnerable to open redirect
                link = request.form.get('link', None)
                bug = Bug(title=title, body=body, link=link, owner_id=current_user.id)
                db.session.add(bug)
                db.session.commit()
            except Exception as e:
                print("from POST submit_bug")
                print(e)
                return 'Error!', 500
            return redirect('/report_bugs')

    return render_template("report_bugs.html", current_user=current_user)

@app.route('/logout')
def logout():
    logout_user()
    return redirect('/')

@app.route('/search')
def search_notes():
    notes = []
    if current_user.is_authenticated:
        query = request.args.get('query', None)
        if query is not None:
            query = '%%%s%%' % str(query)
            notes = Note.query.filter(Note.body.like(query), Note.owner_id == current_user.id).limit(100).all()
    return render_template("search.html", notes = notes, current_user = current_user)



@app.route('/note/<int:note_id>')
def get_note(note_id):
    if current_user.is_authenticated:
        n = Note.query.filter(Note.id == note_id, Note.owner_id == current_user.id).first()
        return render_template("note.html", note=n)
    return 'Not logged in!'


def remove_note():
    # TODO remove a note from the db
    pass


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=80, debug=False)
