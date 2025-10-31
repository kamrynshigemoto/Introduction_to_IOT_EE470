**Code to Insert Data into Webpage (db_insert.php)**
**URL example: https://kshigemotoee.com/db_insert.php?<data>**
<?php
// get database credentials so you can access database
$host = 'localhost';
$dbname = '--';
$username = '--';
$password = '--';

//use PDO method to access database and set error mode to catch if something goes wrong
try {
    $pdo = new PDO("mysql:host=$host;dbname=$dbname", $username, $password);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    
    // modification to code to handle base640-encoded data
    //if using =data? take the base64 data, or if ?base64 then take first key which is the base64 string
    //if none of those used --> $encodedstring is null and get data from URL in another way
    $encodedstring = $_GET['data'] ?? array_keys($_GET)[0] ?? null;
    if ($encodedstring) {
        $decoded = base64_decode($encodedstring, true); //change into regular query string, check if valid
        if ($decoded !== false) {
            parse_str($decoded, $params);
            // extract the parameters from decoded base64 string by turning string into array
            $nodeID = $params['nodeID'] ?? null;        //base64 use 'nodeId' instead of 'nodeID'
            $timeReceived = $params['timeReceived'] ?? null;
            $nodeTemp = $params['nodeTemp'] ?? null;
            $humidity = $params['humidity'] ?? null;
        }
    }
    //if nodeID not set (base64 not found) use this code
    if (!isset($nodeID)) {
        //use original GET parameters -->
        // get parameters from URL using GET method, set default value to null
        $nodeID = $_GET['nodeID'] ?? null;
        $timeReceived = $_GET['timeReceived'] ?? null;
        $nodeTemp = $_GET['nodeTemp'] ?? null;
        $humidity = $_GET['humidity'] ?? null;
    }
    
    // make sure node value entered
    if (!$nodeID) {
        die("Missing nodeID (node_name) parameter.");
    }
    
    // make sure temperature in range (-10 to 100) C
    if ($nodeTemp !== null && ($nodeTemp < -10 || $nodeTemp > 100)) {
        die("Temperature not within accepted range of -10 to 100 C.");
    }
    
    // make sure humidity in range (0 - 100)
    if ($humidity !== null && ($humidity < 0 || $humidity > 100)) {
        die("Humidity not within accepted range of 0 to 100.");
    }
    
    // check if node is in sensor_register table
    $nodeCheck = $pdo->prepare("SELECT COUNT(*) FROM sensor_register WHERE node_name = :nodeID");
    $nodeCheck->execute([':nodeID' => $nodeID]);
        // runs the query using the value from nodeID
    if ($nodeCheck->fetchColumn() == 0) {
        die("Node '$nodeID' is not a registered node.");
    }
    //if the count is 0 that means there is no registered node because no matching node was found
    
    // check if time is included in URL. if equal to null then give the current timestamp
    if (!$timeReceived) {
        date_default_timezone_set('America/Los_Angeles');  //Pacific Time (California)
        $timeReceived = date('Y-m-d H:i:s');
    }
    
    // make sure no duplicate date for same node accepted
    $noDuplicate = $pdo->prepare("SELECT COUNT(*) FROM sensor_data WHERE node_name = :nodeID AND time_received = :timeReceived");
        //prepare query to count the entries for the entered node name and time
    $noDuplicate->execute([
        ':nodeID' => $nodeID,
        ':timeReceived' => $timeReceived
        ]);
        //run the query using the entered values for time received and node ID
    if ($noDuplicate->fetchColumn() > 0) {
        die("Duplicate entry for node '$nodeID' at time '$timeReceived'.");
    }
    
    // insert data using URL
    $insertData = $pdo->prepare("INSERT INTO sensor_data (node_name, time_received, temperature, humidity) VALUES (:nodeID, :timeReceived, :nodeTemp, :humidity)");
    $insertData->execute([
        ':nodeID' => $nodeID,
        ':timeReceived' => $timeReceived,
        ':nodeTemp' => $nodeTemp,
        ':humidity' => $humidity
        ]);
        //run the query to get the URL parameters and enter it into the data table
    
    //success message
    echo "Data successfully inserted into data table for '$nodeID' at '$timeReceived'.";
}
//catch database errors and display the error message
catch (PDOException $e) {
    die("Database Error: " . $e->getMessage());
}
?>
