<?php

$ch = curl_init();

curl_setopt($ch, CURLOPT_URL, 'http://localhost:8888/v1/chain/get_info');
curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
curl_setopt($ch, CURLOPT_POST, 1);

$result = curl_exec($ch);
if (curl_errno($ch)) {
    echo 'Error:' . curl_error($ch);
}
curl_close ($ch);
$json = json_decode($result);
echo "<pre>";
echo json_encode($json, JSON_PRETTY_PRINT);
echo "</pre>";
?>