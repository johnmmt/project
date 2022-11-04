<!DOCTYPE html>
<html><body>
<?php
/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-mysql-database-php/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

$servername = "localhost";

// REPLACE with your Database name
$dbname = "new_esp_data";
// REPLACE with Database user
$username = "root";
// REPLACE with Database user password
$password = "myProject";

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);
// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 

$sql = "SELECT longitude, latitude, reading_time FROM SensorData ORDER BY reading_time DESC";

echo '<table cellspacing="5" cellpadding="5">
      <tr>
        <td>Timestamp</td>
        <td>Longitude</td>
        <td>Latitude</td>
      </tr>';
 
if ($result = $conn->query($sql)) {
    while ($row = $result->fetch_assoc()) {
        $row_reading_time = $row["reading_time"];
        $row_longitude = $row["longitude"];
        $row_latitude = $row["latitude"];
        // Uncomment to set timezone to - 1 hour (you can change 1 to any number)
        //$row_reading_time = date("Y-m-d H:i:s", strtotime("$row_reading_time - 1 hours"));
      
        // Uncomment to set timezone to + 4 hours (you can change 4 to any number)
        //$row_reading_time = date("Y-m-d H:i:s", strtotime("$row_reading_time + 4 hours"));
      
        echo '<tr>
                <td>' . $row_reading_time . '</td>
                <td>' . $row_longitude . '</td>
                <td>' . $row_latitude . '</td>
              </tr>';
    }
    $result->free();
}

$conn->close();
?>
</table>
</body>
</html>
