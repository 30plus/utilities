<!doctype html>
<html lang=zh>
<head>
	<meta charset=utf8>
	<meta name="description" content="E-book Manager" />
	<title>E-book Manager</title>
	<link rel="stylesheet" type="text/css" href="css/navbar.css" />
	<script src="js/jquery.js" type=text/javascript></script>
</head>
<body>
<div id=nav>
<div class=l></div>
<ul class=c>
	<li><span class=v><a href="http://www.rockwork.org" target="_blank">首页</a></span> 
	<div class=kind_menu id="Layer1">电子书管理系统</div></li>

	<li><span class=v><a href="#">书目查询</a></span> 
	<div class=kind_menu  id="Layer2">
	<a href="#">精确查找</a> 
	<a href="#">关键词查询</a></div></li>

	<li><span class=v><A href="#">书目分类</a></span>
	<div class=kind_menu id="Layer3">
	<a href="#">书名</a>
	<a href="#">简短介绍</a></div></li>
  
	<li><span class=v><a href="#">数据管理</a></span> 
	<div class=kind_menu id="Layer4">
	<a href="#">增加书目</a>
	<a href="#">删除书目</a></div></li>
  
	<li><span class=v><a href="#">数据生成</a></span>
	<div class=kind_menu id="Layer5">
	<a href="#">数据更新</a>
	<a href="#">数据重构</a></div></li>
  
	<li><span class=v><a href="#">个人简介</a></span>
	<div class=kind_menu id="Layer6">
	<a href="#">个人介绍</a>
	<a href="#">个人简历</a></div></li>
	</ul>
	<div class=r></div>
</div><!--nav-->
<script type="text/javascript" src="js/navbar.js"></script>

<?php
	$dbidx = new mysqli("localhost", "rockrush", "MobilisCan", "document");
	if (mysqli_connect_errno()) {
		echo "Error connecting to database server.\n";
		exit;
	}
	$dbidx->query("set names utf8");
	$sql = "select * from prog_info";
	$result = $dbidx->query($sql);
	if($result)
		echo "Done with table list quering.<br />\n";
	while($row = $result->fetch_row()) {
		printf ("%s || %s || %s<br />\n", $row[0], $row[1], $row[2]);
	}
	$result->free();
	$db->close();
?>
</body>
</html>
