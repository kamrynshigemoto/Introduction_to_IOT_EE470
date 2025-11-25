<?php
header('Content-Type: application/json');

$host = 'localhost';
$db = '';
$user = '';
$pass = '';

try {
  $pdo = new PDO("mysql:host=$host;dbname=$db;charset=utf8mb4", $user, $pass,
    [PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION]);
} catch (Exception $e) {
  http_response_code(500);
  echo json_encode(["ok"=>false, "error"=>"DB connect failed"]);
  exit;
}

// Get POST data
$raw   = intval($_POST['raw'] ?? -1);
$volts = floatval($_POST['volts'] ?? -1);
$ts    = $_POST['ts'] ?? null;
$type  = $_POST['type'] ?? 'pot';
$state = $_POST['state'] ?? null;

try {
  if ($type === 'pot') {
    // Insert potentiometer reading
    $stmt = $pdo->prepare("INSERT INTO pot_readings (raw, volts, ts_client) VALUES (?, ?, ?)");
    $stmt->execute([$raw, $volts, $ts]);
    echo json_encode(["ok"=>true, "type"=>"pot"]);
  } elseif ($type === 'led') {
    // Insert LED event
    $stmt = $pdo->prepare("INSERT INTO led_events (state, ts_client) VALUES (?, ?)");
    $stmt->execute([$state, $ts]);
    echo json_encode(["ok"=>true, "type"=>"led"]);
  } else {
    http_response_code(400);
    echo json_encode(["ok"=>false, "error"=>"Unknown type"]);
  }
} catch (Exception $e) {
  http_response_code(500);
  echo json_encode(["ok"=>false, "error"=>"Insert failed"]);
}
?>
