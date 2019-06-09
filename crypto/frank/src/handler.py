#!/usr/bin/env python3

import logging
import re
import signal
import socket
import socketserver
import sys
import threading
import time

from contextlib import contextmanager
from functools import partial

import bots
import server


class RequestHandler(socketserver.StreamRequestHandler):

    def _write_out(self, m):
        out = b"%s\n" % m

        logging.debug(b"< " + out)
        self.request.sendall(out)

    def _read_in(self):
        x = self.rfile.readline().strip()
        assert re.match(r'[a-zA-Z0-9\-\s]+', x.decode('utf-8')), x
        logging.debug(b"> %s\n" % x)
        return x

    def server_loop(self, end_time):

        logging.info('recieved request from %s' % str(self.client_address))

        pubkey = self._read_in()

        # set up server
        serv = server.Server()
        client = serv.register(pubkey)
        serv._set_client(client)
        self._write_out(client)

        all_bots = [bots.ReportBot(serv, self), bots.AbuseBot(serv, self)]

        COMMANDS = {
            b"list": serv.list_users,
            b"put": serv.put_msg,
            b"get": serv.get_msg,
            b"key": serv.get_user,
            b"send": partial(serv.send, client),
            b"recv": partial(serv.recv, client),
            b"report": partial(serv.report, client),
        }

        while (time.time() < end_time):
            cmd = self._read_in().split(b' ')
            if not cmd[0] in COMMANDS:
                logging.info("INVALID COMMAND: %s" % cmd[0])
                sys.exit(1)

            # mypy doesn't like partial :(
            resp = COMMANDS[cmd[0]](*cmd[1:])  #type: ignore

            if cmd[0] == b"send":
                for bot in all_bots:
                    bot.react()

            if resp is not None:
                if cmd[0] in [b"recv", b"list"]:
                    for x in resp:
                        self._write_out(x)
                    self._write_out(b"done")
                else:
                    self._write_out(resp)

            if serv.should_exit():
                sys.exit(0)

            if cmd[0] == b"send":
                for bot in all_bots:
                    bot.react()

    def handle(self):
        try:
            self.request.settimeout(30)
            end_time = time.time() + 30
            self.server_loop(end_time)
        except socket.timeout:
            logging.info('timeout')
        except ConnectionResetError:
            logging.info('reset')
        except Exception as e:
            logging.info('other error: %s' % str(e))
        finally:
            logging.info('closing request from %s' % str(self.client_address))


class TCPThreadServer(socketserver.ThreadingMixIn, socketserver.TCPServer): pass

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == 'debug':
        logging.basicConfig(
            format="%(levelname)s:%(threadName)s:%(message)s",
            level=logging.DEBUG,
            handlers=[
                logging.FileHandler("frank.log"),
                logging.StreamHandler()
            ],
        )

    with TCPThreadServer(("0.0.0.0", 4567), RequestHandler) as s:
        s.allow_reuse_address = True
        s.serve_forever()
