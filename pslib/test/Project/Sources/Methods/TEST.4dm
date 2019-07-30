//%attributes = {}
$geometry:=geometry 
$fonts:=getFonts 

$resourcesFolderPath:=Get 4D folder:C485(Current resources folder:K5:16)

$command:=New object:C1471
$command.creator:="4D"
$command.author:="4D"
$command.title:="4D"
$command.keywords:="4D"
$command.boundingBox:="0 0 596 842"

$leftmargin:=100
$boxwidth:=200
$boxheight:=630
$colsep:=20
$baseline:=100
$boxed:=0
$fontsize:=10

$command.pages:=New collection:C1472

$page:=0

$command.pages[$page]:=New object:C1471
$command.pages[$page].width:=596
$command.pages[$page].height:=842

$command.pages[$page].data:=New collection:C1472

  //font
$command.pages[$page].data.push(New object:C1471("font";0;"size";15))

$data:=$geometry[0]

$params:=New object:C1471()
$values:=New object:C1471(\
"charspacing";2;\
"params";$params;\
"values";$values)

  //text

$command.pages[$page].data.push(New object:C1471("text";$data.text;\
"encoding";$data.encoding;\
"x";($command.pages[$page].width/2)-($colsep/2)-($data.width/2);\
"y";($baseline+$boxheight+20);\
"params";$params;"values";$values))

$data:=$geometry[2]

$params:=New object:C1471("hyphenation";"true";\
"hyphendict";$resourcesFolderPath+"hyph_de.dic")
$values:=New object:C1471("hyphenminchars";2;\
"parindent";12;\
"parskip";14;\
"leading";14;\
"charspacing";0;\
"parindentskip";0)

  //font
$command.pages[$page].data.push(New object:C1471("font";0;"size";10))

$command.pages[$page].data.push(New object:C1471("textArea";$data.text;\
"encoding";$data.encoding;\
"x";$leftmargin;\
"y";$baseline;\
"width";200;"height";630;\
"mode";"fulljustify";\
"params";$params;"values";$values))

$values.parindentskip:=1

C_OBJECT:C1216($data)
For ($i;3;$geometry.length-1)
	
	$data:=$geometry[$i]
	
	If ($i%2=0)
		
		  //left column
		$command.pages[$page].data.push(New object:C1471("textArea";$data.text;\
			"font";0;"size";10;"encoding";$data.encoding;\
			"x";$leftmargin;\
			"y";$baseline;\
			"width";200;"height";630;\
			"mode";"fulljustify";"values";$values))
		
	Else 
		
		  //right column
		$command.pages[$page].data.push(New object:C1471("textArea";$data.text;\
			"font";0;"size";10;"encoding";$data.encoding;\
			"x";$leftmargin+$boxwidth+$colsep;\
			"y";$baseline;\
			"width";200;"height";630;\
			"mode";"fulljustify";"values";$values))
		
		If ($i<($geometry.length-1))
			  //page break
			$page:=$page+1
			$command.pages[$page]:=New object:C1471
			$command.pages[$page].width:=596
			$command.pages[$page].height:=842
			$command.pages[$page].data:=New collection:C1472
		End if 
		
	End if 
	
End for 

C_BLOB:C604($ps)

$ps:=pslib generate ($fonts;$command)

$path:=System folder:C487(Desktop:K41:16)+"test.ps"

BLOB TO DOCUMENT:C526($path;$ps)

OPEN URL:C673($path)