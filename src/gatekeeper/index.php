<?php
/*
 *  OpenDiamond Gatekeeper
 *  An OpenDiamond application the generation of scoping files
 *
 *  Copyright (c) 2007  Intel Corporation
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */
/*  $Id: index.php 288 2007-08-22 19:12:19Z rgass $  */

    //  Check if we are using https
    //  Richard Gass <richard.gass@intel.com>
    //  20070802
    if ((!isset($_SERVER['HTTPS'])) || $_SERVER["HTTPS"] != "on") {
        $bname = dirname($_SERVER['SCRIPT_NAME']);
        $secure_url = "https://" . $_SERVER["HTTP_HOST"] .  $bname;
        header("Location: $secure_url");
        exit;
    }
    //  Get current logged in user;
    $me = (isset($_SERVER['PHP_AUTH_USER'])) ?  $_SERVER['PHP_AUTH_USER'] : NULL;

    include ("gatekeeper.conf");
    $G = init_globals();

    include ("php/functions.php");

    //  Layout the page
    $left_menu = "html/left_menu.html";
    $main_content = "html/front_page.html";

    include ("include/framing.php");
    $header = simple_header($G);
    $footer = simple_footer($G);

    $a = new admin($G);

    $col_list = $a->get_collections();

    $is_admin = $a->is_admin($me);
    //  Get POST vars

    $action = (isset($_GET['action'])) ?  $_GET['action'] : "front_page";
    $collection = (isset($_GET['collection'])) ?  $_GET['collection'] : "";
    $username = (isset($_GET['username'])) ?  $_GET['username'] : "";
    $add_col = (isset($_GET['add_col'])) ?  $_GET['add_col'] : "";
    $scope = (isset($_POST['scope'])) ?  $_POST['scope'] : "";


    //  Default setting 
    if ($action == "front_page") {
        $main_content = "html/front_page.html";
    }
    //  Get listing of all users
    $all_users = $a->list_users();
    //  Add the user to the selected collections
    if ($action == "add_user") {
        $main_content = "html/add_user.html";
        if (($add_col != "") && ($username != "")) {
            //  Print the add user admin stuff
            $a->add_user($username, $add_col);
            print_summary();
            //include ("add_user_to_group.html");
        }
    }

    //  Define the scope 
    if ($action == "define_scope") {
        $main_content = "html/scope.html";
        if (($scope != "") && ($me != "")) {

            foreach ($scope as $coll) {
                $scope_info = $a->get_scope($coll);
                $content = $scope_info['collection'] . " ";
                $content .= $scope_info['groupid'] . " ";
                $content .= $scope_info['server'] . "\n";
            }
            file_put_contents ("db/testfile.txt", $content);
            $filename = "diamond_config_scope";
            //  text/plain will allow you to select save or open
            #header('Content-type: text/plain');
            //  Use octet-stream if you want to only allow a user to save
            header('Content-type: application/octet-stream');
            header('Content-Disposition: attachment; filename="'.$filename.'"');
            echo $content;
            exit; 
        }
    }

    //  Check if user is admin
    if ($is_admin == 1) {
        //  Delete a user
        if ($action == "delete_user") {
            $main_content = "html/mod_user.html";
            if ($username != "") {
                $a->delete_user($username);
                print_summary();
            }
        }
        //  Delete a user from a collection
        if ($action == "delete_user_from_collection") {
            $main_content = "html/mod_user.html";
            if (($collection != "") && ($username != "")) {
                $a->delete_user($username, $collection);
                print_summary();
            }
        }
        //  Print out the summary  
        if ($action == "summary") {
            //  List the users;
            //$member_list = $a->member_of("rgass");
            $main_content = "html/summary.html";
        }

    }

    include ("html/page_layout.html");


    function print_summary() {
        $bname = dirname($_SERVER['SCRIPT_NAME']);
        $secure_url = "https://" . $_SERVER["HTTP_HOST"] .  $bname;
        header("Location: $secure_url?action=summary");
        exit;
    }

?>