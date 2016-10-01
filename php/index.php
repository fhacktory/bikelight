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
$xStart =($width >= MAX_WIDTH) ? 0 : floor((MAX_WIDTH - $width) / 2);
$xEnd =($width >= MAX_WIDTH) ? MAX_WIDTH : $xStart + $width;
$yStart =($height >= MAX_WIDTH) ? 0 : floor((MAX_WIDTH - $height) / 2);
$yEnd =($height >= MAX_WIDTH) ? MAX_WIDTH : $yStart + $height;
//echo $xStart."_".$xEnd."_".$yStart."_".$yEnd."<br />";
$suffix = "";
for($y = $yStart; $y < $yEnd; $y++)
{
	$strImage .= $suffix;
	$strImage .= "\t{";
	for($x = $xStart; $x < $xEnd; $x++)
	{
		$rgb = imagecolorat($im, $x - $xStart, $y - $yStart);
		$r = ($rgb >> 16) & 0xFF;
		$g = ($rgb >> 8) & 0xFF;
		$b = $rgb & 0xFF;
		$strImage .= getGray($r, $g, $b).",";
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



