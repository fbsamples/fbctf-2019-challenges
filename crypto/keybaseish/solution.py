import requests
import re

CHALLENGE_URL = 'http://localhost:8080'
VICTIM_NAME = 'baseishcoinfou1'

# From https://twitter.com/baseishcoinfou1
EXISTING_SIG = '43522081190908620239526125376626925272670879862906206214798620592212761409287968319160030205818706732092664958217053982767385296720310547463903001181881966554081621263332073144333148831108871059921677679366681345909190184917461295644569942753755984548017839561073991169528773602380297241266112083733072690367'

s = requests.Session()


def get_recover_page():
    url = '{}/recover'.format(CHALLENGE_URL)
    resp = s.get(url)
    if resp.status_code != 200:
        raise("Error {} getting {}".format(resp.status_code, url))
    return resp.text


def get_login_page():
    url = '{}/login'.format(CHALLENGE_URL)
    resp = s.get(url)
    if resp.status_code != 200:
        raise("Error {} getting {}".format(resp.status_code, url))
    return resp.text


def parse_challenge(html):
    m = re.search(r'pin: (\d{6})', html)
    pin = m.group(1)
    return int(pin)


def parse_signature(html):
    # return int(''.join(re.findall(r'PRF\d/\d:(\d+)', html))) # python2 would use long
    return int(EXISTING_SIG)


def parse_csrf_token(html):
    m = re.search(r'<input type="hidden" name="csrfmiddlewaretoken" value="([^"]+)">', html)
    return m.group(1)


def parse_temppass(html):
    m = re.search(r'Use this password to login: (\w+)', html)
    return m.group(1)


def get_password(pubkey, csrf_token):
    data = {
        'handle': VICTIM_NAME,
        'pubkey': pubkey,
        'csrfmiddlewaretoken': csrf_token,
    }
    url = '{}/temppass'.format(CHALLENGE_URL)
    resp = s.post(url,data=data)
    if resp.status_code != 200:
        raise("Error {} getting {}".format(resp.status_code, url))
    # print(resp.text)
    return parse_temppass(resp.text)


def parse_flag(html):
    m = re.search(r'this is your secret message: ([\w{}]+)', html)
    return m.group(1)


def get_flag(temppass, csrf_token):
    data = {
        'handle': VICTIM_NAME,
        'password': temppass,
        'csrfmiddlewaretoken': csrf_token,
    }
    url = '{}/messages'.format(CHALLENGE_URL)
    resp = s.post(url,data=data)
    if resp.status_code != 200:
        raise("Error {} getting {}".format(resp.status_code, url))
    return parse_flag(resp.text)


if __name__ == '__main__':
    t = get_recover_page()
    pin = parse_challenge(t)
    print('pin: {}'.format(pin))
    signature = parse_signature(t)
    print('existing signature: {}'.format(signature))
    csrf_token = parse_csrf_token(t)
    print('csrf token: {}'.format(csrf_token))
    
    e_new = 1
    n_new = signature - pin
    pubkey = '{}:{}'.format(e_new, n_new)
    print('New Public Key: {}'.format(pubkey))
    temppass = get_password(pubkey, csrf_token)
    print('Account Recovery Password: {}'.format(temppass))
    t = get_login_page()
    csrf_token = parse_csrf_token(t)
    flag = get_flag(temppass, csrf_token)
    print('Flag: {}'.format(flag))
