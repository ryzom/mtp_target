<?php

	echo "<h2>Official Server High Score</h2>\n";

	$filename = "/home/ace/cvs/mtp-target/server/mtp_target_service.cfg";

	$fp = fopen ($filename, "rb");
	if (!$fp)
	{
		echo "no info";
		return;
	}

	while (!feof($fp))
	{
		$line = fgets($fp, 10000);
		if (strstr ($line, "Accounts"))
		{
			$line = "";

			while (!feof($fp))
			{
				$nl = fgets($fp, 10000);
				if(strstr($nl,"};")) break;
				$line = $line.$nl;
			}
			$line = strtr($line, "=,{};","     ");
			$line = ereg_replace (' +', ' ', $line);
			list($user,$pass,$score) = explode(" ",$line);
			$arr = explode(" ",$line);
			$p=0;

			for ($i = 0; $i+2 < sizeof($arr);)
			{
				$my1[$p] = strtr($arr[$i],"\""," ");
				$my2[$p] = (int)(strtr($arr[$i+2],"\""," "));
				$i+=3;
				$p++;
			}
			array_multisort($my2, SORT_DESC, $my1);

			echo "<h3>Registered:</h3>";
			echo sizeof($my1)." players</p>";

			echo "<h3>Highscores:</h3>";
			for ($i = 0; $i < 1; $i++)
				echo "<font style=\"color:red\"><b>$my1[$i]</b> : $my2[$i]</font><br>";

			for (; $i < 3; $i++)
				echo "<b>$my1[$i]</b> : $my2[$i]<br>";

			for (; $i < 10; $i++)
				echo "$my1[$i] : $my2[$i]<br>";
		}
		else if (strstr ($line, "Levels"))
		{
			$line = substr($line, 26);
			$line = strtr($line, "=,{};\"","      ");
			$line = ereg_replace (' +', ' ', $line);
			$LevelNames = explode(" ",$line);
		}
		else if (strstr ($line, "LevelStats"))
		{
			$line = substr($line, 26);
			$line = strtr($line, "=,{};\"","      ");
			$line = ereg_replace (' +', ' ', $line);
			$LevelStats = explode(" ",$line);
		}
	}
	fclose($fp);

/*	echo "<p><b>Highscores by level:</b><br>";
	if (isset($LevelNames) && isset($LevelStats))
	{
	 for ($i=0; $i < sizeof($LevelNames); $i++)
	 {
	 if (strlen($LevelNames[$i])>1)
	 {
	 echo "- <b>$LevelNames[$i] :</b><br>";
		 echo $LevelStats[$i*4]." : ".$LevelStats[$i*4+1]." points<br>";
		   echo $LevelStats[$i*4+2]." : ".$LevelStats[$i*4+3]." seconds<br>";
	 }	   
	 }	   
	}
*/

?>
