from django.shortcuts import render

# Create your views here.
from django.http import HttpResponse
from django.template import loader

from random import randint, choice
import string

from Crypto.PublicKey import RSA


SIGNATURE = "43522081190908620239526125376626925272670879862906206214798620592212761409287968319160030205818706732092664958217053982767385296720310547463903001181881966554081621263332073144333148831108871059921677679366681345909190184917461295644569942753755984548017839561073991169528773602380297241266112083733072690367"
ADMIN_USER = 'baseishcoinfou1'
FLAG = 'flag{6F4EF3C06D00C14731B424069225CC6CAE96CD869E03C1B45A5D94A08F679DA2}'


def error_template(request, msg):
    template = loader.get_template('error.html')
    context = {
        'msg': msg
    }
    return HttpResponse(template.render(context, request))


def validate_sig(msg, n, e, sig):
    K2 = RSA.construct((n, e))
    print("Constructed Key with e={}, n={}".format(e, n))
    signature = (int(sig), ) # pycrypto expected format
    print("Validating msg {} against sig={}".format(msg, signature))
    valid = K2.verify(int(msg), signature)
    print("Sig was: {}".format(valid))
    return valid

def recover(request):
    signature = SIGNATURE
    pin = ''.join(str(randint(0,9)) for _ in range(6))
    request.session['pin'] = pin
    template = loader.get_template('recovery.html')
    context = {
        "pin": pin,
        "signature": signature,
    }
    return HttpResponse(template.render(context, request))


def temppass(request):
    handle = request.POST.get('handle', '')
    if handle != ADMIN_USER:
        return error_template(request, 'Could not find signature in feed of user @{}'.format(handle))

    publickey_str = request.POST.get('pubkey')
    if ":" not in publickey_str:
        return error_template(request, 'Incorrect public key format, should be "e:n"')
    e, n = ([int(x) for x in publickey_str.split(":")])
    if e <= 3:
        return error_template(request, 'Invalid public key, e is too small.')
    pin = request.session.get('pin')
    if not pin:
        return error_template(request, "No validation pin in your session, aborting.")

    if (validate_sig(pin, n, e, SIGNATURE)):
        letters = string.ascii_lowercase + string.digits + string.ascii_uppercase
        password = ''.join(choice(letters) for i in range(15))
        request.session['pass'] = password
        template = loader.get_template('temppass.html')
        context = {
            "password": password
        }
        return HttpResponse(template.render(context, request))

    return error_template(request, "Signature could not be verified with that public key.")

def messages(request):
    handle = request.POST.get('handle', '')
    password = request.POST.get('password', '')
    pass_check = request.session.get('pass')

    if handle != ADMIN_USER:
        return error_template(request, 'Bad password for @{}'.format(handle))

    if password != pass_check:
        return error_template(request, 'Bad password for @{}'.format(handle))

    request.session['solved'] = 1
    template = loader.get_template('messages.html')
    context = {
        "flag": FLAG
    }
    return HttpResponse(template.render(context, request))
