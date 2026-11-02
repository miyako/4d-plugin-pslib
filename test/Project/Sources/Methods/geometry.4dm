//%attributes = {}
C_COLLECTION:C1488($0;$geometry)

$fonts:=getFonts 

$texts:=New collection:C1472
$texts[0]:=New object:C1471("value";"Was ist Relativitäts-Theorie?";"font";0;"size";15;"encoding";"latin1")
$texts[1]:=New object:C1471("value";"Albert Einstein";"font";0;"size";15;"encoding";"latin1")

$resourcesFolderPath:=Get 4D folder:C485(Current resources folder:K5:16)
$text:=Document to text:C1236($resourcesFolderPath+"einstein.txt";"latin1";Document with LF:K24:22)

$params:=New object:C1471("hyphenation";"true";\
"hyphendict";$resourcesFolderPath+"hyph_de.dic")
$values:=New object:C1471("hyphenminchars";2;\
"parindent";12;\
"parskip";14;\
"leading";14;\
"charspacing";0;\
"parindentskip";0)

$texts[2]:=New object:C1471("value";$text;"font";0;"size";10;"encoding";"latin1";\
"width";200;"height";630;"mode";"fulljustify";"params";$params;"values";$values)

C_OBJECT:C1216($textArea)

$geometry:=pslib geometry ($fonts;$texts)
$textArea:=$geometry[$geometry.length-1]

$values.parindentskip:=1

While ($textArea.rest#"")
	  //need hyphenation table (params) for each call
	$g:=pslib geometry ($fonts;New collection:C1472(New object:C1471("value";$textArea.rest;\
		"font";0;"size";10;"encoding";"latin1";\
		"width";200;"height";630;"mode";"fulljustify";"params";$params;"values";$values)))
	$geometry.push($g[0])
	$textArea:=$g.pop()
End while 

$0:=$geometry