import sys

from client import Client

class Bot(object):
    def __init__(self, server, handler):
        self._client = Client(server)
        self._handler = handler

    def react(self):
        msgs = self._client.recv()
        for (who, mid, msg) in msgs:
            response = self.react_to(who, mid, msg)
            if response:
                self._client.send(response, who)


class ReportBot(Bot):
    def react_to(self, who, mid, msg):
        if b"abuse" in msg.lower():
            self._handler._write_out(self._client.report(who, mid))
            if self._client.server.should_exit():
                sys.exit(0)
            return None
        else:
            return b"Cool story bro"



class AbuseBot(Bot):
    def react_to(self, who, mid, msg):
        return b"This message is abuse"
