#If you can not run this script
	#please use the command
	#Set-ExecutionPolicy Unrestricted
		#Then enter A

$MIP_MAP_LEVEL = 0
$COMPRESSION_TYPE = 'BC7_UNORM'

#-----------------------------------------------------------------	HIGH
$files = Get-ChildItem *.png -Recurse -Force
$files | Foreach-Object  {
	$name = $_.FullName
	$dir = Split-Path -Path $name	
	
	.\texconv.exe $name -o $dir -pow2 -m $MIP_MAP_LEVEL -f $COMPRESSION_TYPE -y
}
$files = Get-ChildItem *.jpg -Recurse -Force
$files | Foreach-Object  {
	$name = $_.FullName
	$dir = Split-Path -Path $name	
	
	.\texconv.exe $name -o $dir -pow2 -m $MIP_MAP_LEVEL -f $COMPRESSION_TYPE -y
}
$files = Get-ChildItem *.bmp -Recurse -Force
$files | Foreach-Object  {
	$name = $_.FullName
	$dir = Split-Path -Path $name	
	
	.\texconv.exe $name -o $dir -pow2 -m $MIP_MAP_LEVEL -f $COMPRESSION_TYPE -y
}