**Code to Display Data Tables and Graph (displaydatatables.php)**
**URL: https://kshigemotoee.com/displaydatatables.php**
<?php
// get database credentials to connect to MySQL database
$host = 'localhost';
$dbname = '--';
$username = '--';
$password = '--';

// create PDO (php data object) to connect to database
try {
    $pdo = new PDO("mysql:host=$host;dbname=$dbname", $username, $password);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);  //throw exception if something goes wrong
    
    // get sensor_register data
    $registerQuery = "SELECT node_name, manufacturer, longitude, latitude FROM sensor_register ORDER BY node_name ASC";  
        //takes data from sensor_register and orders in ascending order by node name
    $registerStmt = $pdo->prepare($registerQuery);
        //get query ready to execute, returns statement object
    $registerStmt->execute();
        //runs the prepared query
    $registerData = $registerStmt->fetchAll(PDO::FETCH_ASSOC);
        //retrieves all the rows returned from the query in associative array
        
    // get sensor_data data
    $dataQuery = "SELECT node_name, time_received, temperature, humidity FROM sensor_data ORDER BY node_name ASC, time_received ASC";
        //takes data from sensor_data and orders in ascending order by node name and time time_received
    $dataStmt = $pdo->prepare($dataQuery);
        //get query ready to execute to extract data
    $dataStmt->execute();
        //run the prepared query
    $dataResults = $dataStmt->fetchAll(PDO::FETCH_ASSOC);
        //retrieves all the rows returned from the query in associative array
        
    // calculate average temperature and humidity for node 1 from sensor data table
    $averageQuery = "SELECT AVG(temperature) AS avg_temp, AVG(humidity) AS avg_humidity FROM sensor_data WHERE node_name = 'node_1'";
        //get average temperature and humidity from sensor data table and store as variables
    $averageStmt = $pdo->prepare($averageQuery);
        //get average query ready to execute
    $averageStmt->execute();
        //run the prepared query
    $averageResult = $averageStmt->fetch(PDO::FETCH_ASSOC);
        //retrieve data returned as associative array
        
    // get data from node 1 to graph
    $graphQuery = "SELECT time_received, temperature FROM sensor_data WHERE node_name = 'node_1' ORDER BY time_received ASC";
        // get time and temperature data from node in ascending order for the graph
    $graphStmt = $pdo->prepare($graphQuery);
    $graphStmt->execute();
        //run the query
    $graphData = $graphStmt->fetchAll(PDO::FETCH_ASSOC);
        //retrieve all the query data and return as associative array
        
    // get data from node 1 to display in JSON format
    //$node1Query = "SELECT node_name, time_received, temperature, humidity FROM sensor_data WHERE node_name = 'node_1' ORDER BY time_received ASC";
    //$node1Stmt = $pdo->prepare($node1Query);
    //$node1Stmt->execute();
    //$node1Data = $node1Stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // output node 1 data as JSON
    //header('Content-Type: application/json');
    //echo json_encode($node1Data, JSON_PRETTY_PRINT);
    
    }
    
catch (PDOException $e) {
    die("Database connection failed: " . $e->getMessage());
    //PDOException is a special type of error related to database operations with PDO, $e is the variable that holds the error object
    //die stops the script then will display message with actual error
}

// prepare data for Google Charts
$dataArray = [['Time Received', 'Temperature']];
foreach ($graphData as $row) {
    $dataArray[] = [$row['time_received'], (float) $row['temperature']];
}

//convert data to JSON
$dataJSON = json_encode($dataArray);
?>

<!DOCTYPE html>
<html>
<head>
    <title>Welcome to SSU IoT Lab</title>
    <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: white;
            padding: 20px;
        }
        
        h1 {
            color: #003366;
        }
        
        h2 {
            color: #0055aa;
            margin-top: 40px;
        }
        
        table {
            border-collapse: collapse;
            width: 100%;
            margin-top: 10px;
        }
        
        th, td {
            border: 1px solid #aaa;
            padding: 8px;
            text-align: left;
        }
        
        th {
            background-color: #cce5ff;
        }
        
        tr:nth-child(even) {
            background-color: #e6f2ff;
        }
    </style>
</head>
<body>
    <h1>Welcome to SSU IoT Lab</h1>
    
    <h2>Registered Sensor Nodes</h2>
    <table>
        <tr>
            <th>Node Name</th>
            <th>Manufacturer</th>
            <th>Longitude</th>
            <th>Latitude</th>
        </tr>
        <?php foreach ($registerData as $row): ?>
            <tr>
                <td><?= htmlspecialchars($row['node_name']) ?></td>
                <td><?= htmlspecialchars($row['manufacturer']) ?></td>
                <td><?= htmlspecialchars($row['longitude']) ?></td>
                <td><?= htmlspecialchars($row['latitude']) ?></td>
            </tr>
        <?php endforeach; ?>
    </table>
    
    <h2>Data Received</h2>
    <table>
        <tr>
            <th>Node Name</th>
            <th>Time Received</th>
            <th>Temperature</th>
            <th>Humidity</th>
        </tr>
        <?php foreach ($dataResults as $row): ?>
            <tr>
                <td><?= htmlspecialchars($row['node_name']) ?></td>
                <td><?= htmlspecialchars($row['time_received']) ?></td>
                <td><?= htmlspecialchars($row['temperature']) ?></td>
                <td><?= htmlspecialchars($row['humidity']) ?></td>
            </tr>
        <?php endforeach; ?>
    </table>
    
    <p>The average temperature for node 1 has been: <?= round($averageResult['avg_temp'], 2) ?> C</p>
    <p>The average humidity for node 1 has been: <?= round($averageResult['avg_humidity'], 2) ?> %</p>
    
    <h2>Sensor Node 1: Temperature Chart</h2>
    <!-- code from IoTCourse GitHub -->
    <script type="text/javascript">
        google.charts.load('current', {'packages':['corechart']});
        google.charts.setOnLoadCallback(drawChart);
        
        function drawChart() {
            var data = google.visualization.arrayToDataTable(<?php echo $dataJSON; ?>);
            
            var options = {
                title: 'Temperature vs Time',
                vAxis: { title: 'Temperature' },
                hAxis: { title: 'Time', slantedText: true },
                series: {0: { pointSize: 6 }},
                //curveType: 'function',
                legend: { position: 'none' },
                bar: {groupWidth: '75%' },
                colors: ['green']   //change to green
            };
            
            //var chart = new google.visualization.LineChart(document.getElementById('chart_div'));
            var chart = new google.visualization.ColumnChart(document.getElementById('chart_div'));
            
            
            chart.draw(data, options);
        }
    </script>
    <div id="chart_div" style="width: 800px; height: 400px;"></div>
    
</body>
</html>
