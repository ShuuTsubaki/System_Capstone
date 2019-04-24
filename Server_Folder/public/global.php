<?php
if(!session_id()) session_start();
$uid = "";
if(!isset($_SESSION['uid'])) {
    $_SESSION['uid'] = $uid;
}
?>