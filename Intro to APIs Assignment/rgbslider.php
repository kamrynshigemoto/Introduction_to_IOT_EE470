<!DOCTYPE html>
<html>
<head>
    <title>RGB Slider</title>
</head>
<body>
    <h1 style="color:green;">EE 470: IoT Course - RGB Slider Control</h1>
    
    <form method="post" action="<?php echo htmlspecialchars($_SERVER['PHP_SELF']);?>">
        <!-- slider 1: control from 0 to 255 -->
        <label for="slider1">Slider 1 (0-255): </label>
        <input type="range" id="slider1" name="slider1" min="0" max="255" value="0" oninput="this.nextElementSibling.value = this.value">
        <output>0</output>
        <p></p>
        
        <!-- submit button -->
        <input type="submit" value="Submit Value">
    </form>
    
    <?php
    //show current value from file if not submitting form
    if ($_SERVER["REQUEST_METHOD"] != "POST" && file_exists("rgbslider.txt")) {
        $current = file_get_contents("rgbslider.txt");
        echo "<p>Current RGB Value: " . $current . "</p>";
    }
    
    //only process the form and write to the file if the request method is "post" (ex: when form submitted)
    if ($_SERVER["REQUEST_METHOD"] == "POST") {
        //get value from slider
        $slider1_value = isset($_POST["slider1"]) ? $_POST["slider1"] : 0;
        
        //display selected value on page
        echo "Slider 1 Value: " . $slider1_value . "<br>";
        
        //write value to file
        $myfile = fopen("rgbslider.txt", "w") or die("Error: Unable to open file.");
        fwrite($myfile, $slider1_value);
        fclose($myfile);
    }
    ?>
</body>
</html>
