<?php

require_once("db.php");

$products = get_top_products();

require_once("header.php");
?>

<p>
  <ul>
<?php
foreach ($products as $product) {
  echo "<li>" . htmlentities($product['name']) . "</li>";
}
?>
  </ul>
</p>

<?php require_once("footer.php");
