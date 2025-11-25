<?php
header('Content-Type: application/json');
$host = "localhost"; $db = ""; $user = ""; $pass = "";
try {
  $pdo = new PDO("mysql:host=$host;dbname=$db;charset=utf8mb4", $user, $pass, [
    PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION
  ]);

 // Use client timestamp if present; otherwise fall back to server timestamp
  $pot = $pdo->query("
    SELECT COALESCE(ts_client, ts_server) AS ts, volts
    FROM pot_readings
    ORDER BY id DESC
    LIMIT 200
  ")->fetchAll(PDO::FETCH_ASSOC);

  $led = $pdo->query("
    SELECT COALESCE(ts_client, ts_server) AS ts, state
    FROM led_events
    ORDER BY id DESC
    LIMIT 200
  ")->fetchAll(PDO::FETCH_ASSOC);

  echo json_encode(['pot'=>$pot, 'led'=>$led]);
} catch (Exception $e) {
  http_response_code(500);
  echo json_encode(['error'=>'DB error']);
}
?>
