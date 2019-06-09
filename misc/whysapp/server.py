import SocketServer
import random
import hashlib
import operator as op
from plyplus import Grammar, STransformer
from Crypto.Cipher import AES

calc_grammar = Grammar("""
    start: add;
    // Rules
    ?add: (add add_symbol)? mul;
    ?mul: (mul mul_symbol)? atom;
    @atom: neg | number | '\(' add '\)';
    neg: '-' atom;
    // Tokens
    number: '[\d.]+';
    mul_symbol: '\*' | '/';
    add_symbol: '\+' | '-';
    WS: '[ \t]+' (%ignore);
""")

class Calc(STransformer):

    def _bin_operator(self, exp):
        arg1, operator_symbol, arg2 = exp.tail

        operator_func = { '+': op.add,
                          '-': op.sub,
                          '*': op.mul,
                          '/': op.div }[operator_symbol]

        return operator_func(arg1, arg2)

    number      = lambda self, exp: float(exp.tail[0])
    neg         = lambda self, exp: -exp.tail[0]
    __default__ = lambda self, exp: exp.tail[0]

    add = _bin_operator
    mul = _bin_operator

calc = Calc()


def validate_format(line):
    try:
        user_id, msg_type, msg = line.split(':', 2)
        user_id = int(user_id)
        return user_id, msg_type, msg.strip()
    except:
        return False, False, False


def get_random_message():
    # types = ['msg', 'cmd', 'math', 'cats', 'ping']
    types = ['msg', 'math', 'cats', 'ping']
    t = random.choice(types)
    if t == 'msg':
        quotes = [
            'Senator, we serve ads.',
            "The question isn't, 'What do we want to know about people?', It's, 'What do people want to tell about themselves?'",
            "Figuring out what the next big trend is tells us what we should focus on.",
            "I look at Google and think they have a strong academic culture. Elegant solutions to complex problems.",
            "No one has done a study on this, as far as I can tell, but I think Facebook might be the first place where a large number of people have come out.",
            "I think we basically saw that the messaging space is bigger than we'd initially realized, and that the use cases that WhatsApp and Messenger have are more different than we had thought originally.",
        ]
        msg = random.choice(quotes)
    # elif t == 'cmd':
    #     cmds = [
    #         'python --version'
    #     ]
    #     msg = random.choice(cmds)
    elif t == 'math':
        x = random.randint(1, 100)
        y = random.randint(1, 100)
        op = random.choice(('-', '+', '*'))
        msg = '{}{}{}'.format(x, op, y)
    elif t == 'cats':
        msg = 'cats'
    elif t == 'ping':
        msg = 'ping'
    return "{type}:{msg}\n".format(type=t, msg=msg)


def check_response(msg, resp):
    msg_type, msg = msg.split(':')
    resp_id, resp_msg_type, resp_msg = validate_format(resp)
    if msg_type == resp_msg_type:
        if msg_type == 'msg':
            return True

        # elif msg_type == 'cmd':
        #     if msg_type == 'whoami':
        #         return resp_msg == 'root'

        elif msg_type == 'math':
            return calc.transform(calc_grammar.parse(msg.strip())) == float(resp_msg)

        elif msg_type == 'cats':
            return resp_msg == 'cats'

        elif msg_type == 'ping':
            return resp_msg == 'pong'
    return False


class HandleRequest(SocketServer.BaseRequestHandler, object):
    def __init__(self, *args, **kwargs):
        self.KEY = "yeetyeetyeetyeet"
        self.CIPHER = AES.new(self.KEY, AES.MODE_ECB)
        super(HandleRequest, self).__init__(*args, **kwargs)

    def aes_send(self, msg):
        r = self.request
        msg = msg.encode('utf-8')
        print msg
        enc = self.CIPHER.encrypt(msg + ((16 - len(msg) % 16) * '\x00'))
        enc = enc.encode('base64')
        print enc
        r.sendall(enc)

    def aes_recv(self, n=1024):
        r = self.request
        response = r.recv(n).strip()
        response = response.decode('base64')
        response = self.CIPHER.decrypt(response)
        return response.strip('\x00')

    def handle(self):
        self.request.settimeout(5)
        r = self.request
        for _ in range(100):
            random_message = get_random_message().strip()
            self.aes_send(random_message)
            response = self.aes_recv()
            print(repr(response))

            user_id, msg_type, msg = validate_format(response)
            if check_response(random_message, response):
                pass
            else:
                self.aes_send("Invalid Whysapp client!!!!!")
                return

        for _ in range(100):
            random_message = get_random_message().strip()
            self.aes_send(random_message)
            response = self.aes_recv()
            print response
            user_id, msg_type, msg = validate_format(response)

            if check_response(random_message, response):
                if user_id >= 500:
                    self.aes_send("You've exhausted your free messages limit. Only users with low IDs can communicate beyond this point.")
                    return
                else:
                    pass
            else:
                self.aes_send("Invalid Whysapp client!!!!!")
                return

        # This means the user has built or spoofed a reasonable client
        self.aes_send('flag:Almost there. Only zuck can issue the flag command.\n')
        user_id, msg_type, msg = validate_format(self.aes_recv())
        if int(user_id) == 4 and msg_type == 'flag':
            self.aes_send('msg:fb{whys_app_when_you_can_whats_app}\n')
        else:
            self.aes_send("msg:You're not zuck!!!!!!!!\n")


class ThreadedServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
    pass


if __name__ == "__main__":
    HOST, PORT = "", 5242
    server = ThreadedServer((HOST, PORT), HandleRequest)
    server.allow_reuse_address = True
    server.serve_forever()
