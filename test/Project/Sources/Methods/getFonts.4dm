//%attributes = {}
$fonts:=New collection:C1472

$resPath:=Get 4D folder:C485(Current resources folder:K5:16)

$fonts[0]:=New object:C1471("name";$resPath+"AlteSchwabacher";"file";$resPath+"AlteSchwabacher.enc";"embed";1)
$fonts[1]:=New object:C1471("name";$resPath+"Helvetica";"file";"";"embed";0)

$0:=$fonts