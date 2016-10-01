<?php

define("MAX_WIDTH", (16 + 19)*2);



$imageURL = (isset($_GET["img"])) ? $_GET["img"] : "";
if(!file_exists($imageURL))
{
	die("L'image n'existe pas");
}

$strImage = "sImage = {<br />";

$im = imagecreatefromjpeg($imageURL);

list($width, $height, $type, $attr) = getimagesize($imageURL);
$xStart = $yStart = 0;
$xEnd = $yEnd = MAX_WIDTH;

if($width <= MAX_WIDTH)
{
	$xStart = (MAX_WIDTH - $width) / 2;
	$xEnd = $xStart + $width;
}
if($height <= MAX_WIDTH)
{
	$yStart = (MAX_WIDTH - $height) / 2;
	$yEnd = $yStart + $height;
}
//echo $xStart."_".$xEnd."_".$yStart."_".$yEnd."<br />";
$suffix = "";

for($y = 0; $y < MAX_WIDTH; $y++)
{
	$strImage .= $suffix;
	$strImage .= "\t{";
	for($x = 0; $x < MAX_WIDTH; $x++)
	{
		if(($x - $xStart) >= 0 && ($x - $xStart) < $width && ($y - $yStart) >= 0 && ($y - $yStart) < $height)
		{
			$rgb = imagecolorat($im, $x - $xStart, $y - $yStart);
			$r = ($rgb >> 16) & 0xFF;
			$g = ($rgb >> 8) & 0xFF;
			$b = $rgb & 0xFF;
			$strImage .= getGray($r, $g, $b).",";
		}
		else
		{
			$strImage .= "0,";
		}
	}	
	$strImage = substr($strImage, 0, strlen($strImage) - 1);
	$strImage .= "}";
	$suffix = ",<br />";
}
$strImage .= "<br />};";
echo $strImage;

function getGray($r, $g, $b)
{
	//echo "R:".$r."_"."G:".$g."_"."B:".$b."<br />";
	$r = abs($r - 255);
	$g = abs($g - 255);
	$b = abs($b - 255);			
	$grey = ($r + $g + $b) / 3;
	//echo $grey."<br />";
	$greyShade = floor($grey / 32);
	return $greyShade;
}




