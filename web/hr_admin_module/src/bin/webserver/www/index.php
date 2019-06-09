<?php

error_reporting(0);
session_start();

$user_input = $_GET['user_search'];

if ($_SESSION["sql_injection"]) {

  $user_input2 = $_SESSION["sql_injection"];

  if(preg_match("/dblink/i", $user_input2)) {
  if(preg_match("/host/i", $user_input2)) {
    $limit = 1;
  }
  $user_input2 = preg_replace('/(connect_timeout ?=)/i', '', $user_input);
  $user_input2 = preg_replace('/(host=)/i', 'connect_timeout=2 host=', $user_input);
  }

  $dbconn = pg_connect("host=postgres port=5432 dbname=docker_db user=docker password=aYRr45lTgN9I9LJcjcr0");
  pg_query($dbconn, "SET statement_timeout TO 20");
  pg_query($dbconn, "SET idle_in_transaction_session_timeout TO 20");
  pg_query($dbconn, "SET lock_timeout TO 20");

  $sql = "SELECT * FROM searches WHERE search = '".$user_input2."'";

  try {
    if (pg_prepare($dbconn, "my_query", $sql)) {
      $start = microtime(true);
      pg_send_query($dbconn, $sql);
      $error = 0;
    } else {
      pg_send_query($dbconn, "SELECT id FROM searches WHERE search = '0'");
      $error = 1;
    }
  } catch (Exception $e) {
    // Do nothing
  }
}

$_SESSION["sql_injection"] = $user_input;

?>
<!DOCTYPE html>
<html>
    <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title>Admin - HR Module</title>
        <link rel="stylesheet" href="/css/bootstrap.min.css">
    </head>
    <body>
      <div class="container">
        <?php if ($error) {echo '<div class="alert alert-warning" style="margin-top:20px" role="alert">
          This is a warning alert—check it out!
        </div>';} ?>
        <div class="row">
          <div class="col-xs-12 col-md-8"><h1>HR Module</h1></div>
          <div class="col-xs-6 col-md-4" style="margin-top: 30px;"><a href="#">Logout</a> | Hacker</div>
        </div>

        <div class="row">
          <div class="col-xs-12 col-md-12">
            <h3>Employees in transition</h3>
            <table class="table table-hover">
              <thead>
                <tr>
                  <th>#</th>
                  <th>First Name</th>
                  <th>Last Name</th>
                  <th>Department</th>
                </tr>
              </thead>
              <tbody>
                <?php
                $employees = [
                  ["Mark", "Otto", "Infrastructure"],
                  ["Jacob", "Thornton", "Infrastructure"],
                  ["Larry", "the Bird", "Front-End"],
                  ["Roy", "Johnson", "Front-End"],
                  ["Bob", "Tables", "Security"],
                  ["Rob", "Blue", "Backend"],
                ];

                $employees_html = ""; $i = 1;

                foreach ($employees as $employee) {
                  $highlight = ($_GET['transition'] == $i ? '#337ab7' : null);
                  if (!$highlight) {
                    $highlight = (strtolower($_GET['employee_search']) == strtolower($employee[0]) ||
                    strtolower($_GET['employee_search']) == strtolower($employee[1]) ? '#337ab7' : null);
                  }

                  $employees_html .= "<tr onclick=\"document.location = '?transition=".$i."';\"
                    style=\"cursor:pointer;background-color:$highlight\">
                    <th scope=\"row\">".$i."</th>
                    <td>".$employee[0]."</td>
                    <td>".$employee[1]."</td>
                    <td>".$employee[2]."</td>
                  </tr>";
                  $i++;
                }

                echo $employees_html;
                ?>
              </tbody>
            </table>
          </div>

        </div>

        <div class="row">
          <div class="col-xs-12 col-md-6">
            <h3>Search employees</h3>
            <form>
              <div class="row">
                <div class="col col-md-6">
                  <input type="text" name="employee_search" value="<?= htmlentities($_GET['employee_search']) ?>" class="form-control" placeholder="Employee's name">
                </div>
                <div class="col">
                  <button type="submit" class="btn btn-primary">Search</button>
                </div>
              </div>
            </form>
          </div>
          <div class="col-xs-12 col-md-6">
            <h3>Search users</h3>
            <form>
              <div class="row">
                <div class="col col-md-6">
                  <input type="text" value="<?= htmlentities($_GET['user_search']) ?>" name="user_search" disabled class="form-control" placeholder="User's name">
                </div>
                <div class="col">
                  <button type="submit" disabled class="btn btn-primary">Search</button>
                </div>
              </div>
            </form>
          </div>
        </div>

        <div class="row">
          <div class="col-xs-12 col-md-12">
            <h3>File manager</h3>
            <div class="alert alert-danger" role="alert">
              Error: File <b>/var/lib/postgresql/data/secret</b> cannot be displayed because the current user (Hacker) does not have sufficient permissions!
            </div>


          </div>
        </div>

        <div class="row">
          <div class="col-xs-12 col-md-12">
            <p style="color:lightgray;text-align:center">Suspicious admin panel - 2019</p>
          </div>
        </div>
      </div>
    </body>
</html>
<?php
if ($limit) {
  pg_cancel_query();
  while (($start + 2) > microtime(true)) {
    //Do nothing
  }
}
?>
