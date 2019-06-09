<?hh
$password="e7800587094bbca584218771ec7a264e";
$pattern = "/^($password)($password)?($password)?($password)?($password)?$/";
$flag="fb{fromMyPoint0fViewTheEVILClubIsEvil}";
$input = '';
if(isset($_GET['password'])) {
  $input = $_GET['password'];
  $match = '';
  if (preg_match($pattern, $input, &$match)) {
    echo "Right! The passphrase of EVIL club is '$flag'\n";
  } else {
    echo "Wrong password... Are you sure you're me?\n";
  }
} else {
  echo "Hey me, please enter password!
For increased usability, I\'ve implemented a regular expression that
detects and accepts if I accidentally enter the correct password up to five
times, so feel free to pass the password up to <a href=\"/?password=five\">five</a> times! It\'s like fuzzy
password matching but better! I\'m welcome!\n";
}

