<?php

/*
CREATE TABLE products (
  name char(64),
  secret char(64),
  description varchar(250)
);

INSERT INTO products VALUES('facebook', sha256(....), 'FLAG_HERE');
INSERT INTO products VALUES('messenger', sha256(....), ....);
INSERT INTO products VALUES('instagram', sha256(....), ....);
INSERT INTO products VALUES('whatsapp', sha256(....), ....);
INSERT INTO products VALUES('oculus-rift', sha256(....), ....);
*/
error_reporting(0);
require_once("config.php"); // DB config

$db = new mysqli($MYSQL_HOST, $MYSQL_USERNAME, $MYSQL_PASSWORD, $MYSQL_DBNAME);

if ($db->connect_error) {
  die("Connection failed: " . $db->connect_error);
}

function check_errors($var) {
  if ($var === false) {
    die("Error. Please contact administrator.");
  }
}

function get_top_products() {
  global $db;
  $statement = $db->prepare(
    "SELECT name FROM products LIMIT 5"
  );
  check_errors($statement);
  check_errors($statement->execute());
  $res = $statement->get_result();
  check_errors($res);
  $products = [];
  while ( ($product = $res->fetch_assoc()) !== null) {
    array_push($products, $product);
  }
  $statement->close();
  return $products;
}

function get_product($name) {
  global $db;
  $statement = $db->prepare(
    "SELECT name, description FROM products WHERE name = ?"
  );
  check_errors($statement);
  $statement->bind_param("s", $name);
  check_errors($statement->execute());
  $res = $statement->get_result();
  check_errors($res);
  $product = $res->fetch_assoc();
  $statement->close();
  return $product;
}

function insert_product($name, $secret, $description) {
  global $db;
  $statement = $db->prepare(
    "INSERT INTO products (name, secret, description) VALUES
      (?, ?, ?)"
  );
  check_errors($statement);
  $statement->bind_param("sss", $name, $secret, $description);
  check_errors($statement->execute());
  $statement->close();
}

function check_name_secret($name, $secret) {
  global $db;
  $valid = false;
  $statement = $db->prepare(
    "SELECT name FROM products WHERE name = ? AND secret = ?"
  );
  check_errors($statement);
  $statement->bind_param("ss", $name, $secret);
  check_errors($statement->execute());
  $res = $statement->get_result();
  check_errors($res);
  if ($res->fetch_assoc() !== null) {
    $valid = true;
  }
  $statement->close();
  return $valid;
}
