<?php
include_once("stat_function.php");
include_once("stat_game.php");
include_once("helpers.php");

	$cacheFileName = $cache_dir."/stat_player_of_the_day_".$lang.".html";
	boxBegin(true);

	if(isCacheFileUpToDateDuration($cacheFileName,$defaultScoresCacheFileDuration))
	{
		include($cacheFileName);
	}
	else if($enableSql)
	{

		$html_fp = fopen($cacheFileName, "wt");
	
	
		$requete = "SELECT user.Login,SUM(user_session.Score) as s,user.UId FROM session,user_session,user WHERE session.Id=user_session.SessionId AND user_session.UId=user.UId AND TO_DAYS(NOW())-TO_DAYS(session.Date)=0 GROUP BY user_session.UId ORDER BY s DESC LIMIT 0,3 ;";
		$result=exec_game_db_requete($requete);
		if($line = mysql_fetch_array($result))
		{
			fprintf($html_fp,"<center>");
			fprintf($html_fp,$potdTitle);
			fprintf($html_fp,$potdTop1,$line[2],$line[0],$line[1]);
			fprintf($html_fp,"</center>");
			fprintf($html_fp,"<br>");
		}
	
		fprintf($html_fp,"<div id=\"small\">");
		$nth = 2;
		while($line = mysql_fetch_array($result))
		{
			fprintf($html_fp,$potdTops,$nth,$line[2],$line[0],$line[1]);
			$nth++;
		}
	
	
		$requete = "SELECT * FROM user ORDER BY UId DESC LIMIT 0,1 ;";
		$result=exec_game_db_requete($requete);
		if($line = mysql_fetch_array($result))
		{
			fprintf($html_fp,$potdNewUser,$line[0],$line[1]);
			fprintf($html_fp,"<br>");
		}
	
		fprintf($html_fp,"</div>");
		fclose($html_fp);	  
		include($cacheFileName);
	}
	
	/*
	$requete = "SELECT COUNT(*) FROM game_user WHERE UNIX_TIMESTAMP(NOW())-UNIX_TIMESTAMP(LastLoginDate)<$webUserIdleTime;";
	$result=exec_requete($requete);
	if($line = mysql_fetch_array($result))
	{
		printf($potdUserOnWebCount,$line[0]);
		printf("<br>");
	}
	*/

	boxEnd();
?>
