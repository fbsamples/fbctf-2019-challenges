<?php

require_once("db.php");
require_once("header.php");

function handle_post() {
  global $_POST;

  $name = $_POST["name"];
  $secret = $_POST["secret"];

  if (isset($name) && $name !== ""
        && isset($secret) && $secret !== "") {
    if (check_name_secret($name, hash('sha256', $secret)) === false) {
      return "Incorrect name or secret, please try again";
    }

    $product = get_product($name);

    echo "<p>Product details:";
    echo "<ul><li>" . htmlentities($product['name']) . "</li>";
    echo "<li>" . htmlentities($product['description']) . "</li></ul></p>";
  }

  return null;
}

$error = handle_post();
if ($error !== null) {
  echo "<p>Error: " . $error . "</p>";
}
?>
<form action="/view.php" method="POST">
  Name: <input type="text" name="name" /><br />
  Secret: <input type="password" name="secret" /><br />
  <input type="submit" value="View" />
</form>

<?php require_once("footer.php");
