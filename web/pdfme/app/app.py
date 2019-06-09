import subprocess
import sys
import tempfile

from flask import Response, Flask, redirect, render_template, request

app = Flask(__name__)
app.config["SECRET_KEY"] = "18y4nc902v78134n75v913480mcx,12345c7-"
app.config["UPLOAD_FOLDER"] = "/tmp/ctf"
app.config["MAX_CONTENT_LENGTH"] = 64 * 1024  # 64kb


ALLOWED_EXTENSIONS = {"fods", "txt"}
IS_MAC = sys.platform == "darwin"


def convert_and_nuke(path, outdir):
    convert_to_pdf = subprocess.Popen(
        [
            "/Applications/LibreOffice.app/Contents/MacOS/soffice"
            if IS_MAC
            else "/opt/libreoffice6.0/program/soffice",
            "--convert-to",
            "pdf",
            path,
            "--outdir",
            outdir,
        ]
    )
    try:
        convert_to_pdf.wait(timeout=15)
    except subprocess.TimeoutExpired:
        return -1


def allowed_file(filename):
    dot_count = 0
    for char in filename:
        if char == ".":
            dot_count += 1
        elif not (char.isalpha() or char.isdigit() or char == "_"):
            return False
    return dot_count == 1 and filename.rsplit(".")[1] in ALLOWED_EXTENSIONS


@app.route("/", methods=["GET", "POST"])
def home():
    resp = None
    if request.method == "POST":
        # check if the post request has the file part
        if "file" not in request.files:
            return redirect(request.url)
        file = request.files["file"]
        # if user does not select file, browser also
        # submit an empty part without filename
        if file.filename == "":
            return redirect("/")
        if file and allowed_file(file.filename):
            outfile = tempfile.NamedTemporaryFile(dir=app.config["UPLOAD_FOLDER"])
            f = tempfile.NamedTemporaryFile(dir=app.config["UPLOAD_FOLDER"])

            # read the uploaded file into memory and convert it
            f.write(file.read())
            f.seek(0)
            convert_and_nuke(f.name, app.config["UPLOAD_FOLDER"])

            with open(f.name + ".pdf", "rb") as converted_pdf:
                outfile.write(converted_pdf.read())

            # return the PDF directly
            outfile.seek(0)
            resp = Response(outfile)
            resp.headers['Content-Disposition'] = "inline; filename=%s" % "1.pdf"
            resp.mimetype = 'application/pdf'
            return resp

    return render_template("index.html")


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=80, debug=False)
