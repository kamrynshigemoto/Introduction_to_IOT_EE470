<?php
//read input from PUT request
$input = file_get_contents("php://input");

//check if input is acceptable
if ($input == "on" || $input == "off") {
    file_put_contents("results.txt", $input);
    http_response_code(200);
    echo "LED status updated to: $input";
}
else {
    http_response_code(400);
    echo "Error: Invalid input.";
}
