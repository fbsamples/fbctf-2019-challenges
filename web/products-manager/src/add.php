<?php

require_once("db.php");
require_once("header.php");

function validate_secret($secret) {
  if (strlen($secret) < 10) {
    return false;
  }
  $has_lowercase = false;
  $has_uppercase = false;
  $has_number = false;
  foreach (str_split($secret) as $ch) {
    if (ctype_lower($ch)) {
      $has_lowercase = true;
    } else if (ctype_upper($ch)) {
      $has_uppercase = true;
    } else if (is_numeric($ch)) {
      $has_number = true;
    }
  }
  return $has_lowercase && $has_uppercase && $has_number;
}

function handle_post() {
  global $_POST;

  $name = $_POST["name"];
  $secret = $_POST["secret"];
  $description = $_POST["description"];

  if (isset($name) && $name !== ""
        && isset($secret) && $secret !== ""
        && isset($description) && $description !== "") {
    if (validate_secret($secret) === false) {
      return "Invalid secret, please check requirements";
    }

    $product = get_product($name);
    if ($product !== null) {
      return "Product name already exists, please enter again";
    }

    insert_product($name, hash('sha256', $secret), $description);

    echo "<p>Product has been added</p>";
  }

  return null;
}

$error = handle_post();
if ($error !== null) {
  echo "<p>Error: " . $error . "</p>";
}
?>
<form action="/add.php" method="POST">
  Name of your product: <input type="text" name="name" /><br />
  Secret (10+ characters, smallcase, uppercase, number) : <input type="password" name="secret" /><br />
  Description: <input type="text" name="description" /><br />
  <input type="submit" value="Add" />
</form>

<?php require_once("footer.php");
