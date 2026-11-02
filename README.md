# 4d-plugin-pslib

pslib generates PostScript documents from a description you build as a 4D object. It wraps [pslib](http://pslib.sourceforge.net) (linked into the plugin): you pass the fonts to load and a list of pages, each with text, boxed (justified, hyphenated) text and simple drawing commands, and get the PostScript program back as a `BLOB`. A second command measures text without producing a document, so you can lay out multi-column text before you generate it.

| Command | Returns | Purpose |
|---|---|---|
| [pslib generate](#pslib-generate) | `BLOB` | Build a PostScript document from fonts and a page description |
| [pslib geometry](#pslib-geometry) | `Collection` | Measure text, or fit text into a box and get what doesn't fit |

**Platforms:** macOS, Windows (64-bit)

---

## Requirements & platform notes

- **Both commands take two mandatory parameters**, and the first one is always the same fonts collection (`pslib generate(&C;&J):O` and `pslib geometry(&C;&C):C` in the manifest).
- **Fonts are Type 1 font files on disk.** Each font is given by a path to its metric file *without* the extension; pslib reads the `.afm` file next to it, and the `.pfb` file too when the font is embedded. See [Fonts collection](#fonts-collection).
- **On macOS, font, encoding and hyphenation-dictionary paths are system (HFS, colon-separated) paths**, such as the ones `Get 4D folder` returns. The plugin converts them to POSIX. **On Windows** they are passed as is.
- **Text is converted from Unicode to the font's 8-bit encoding** with iconv, using the `encoding` you pass (for example `"latin1"`). Characters that don't exist in that encoding make the conversion fail, and that text is then not drawn. Document metadata (`title`, `author`, …) is always converted to Latin-1.
- **Failure is mostly silent.** Unknown keys are ignored, text that can't be converted is skipped, and a font index that doesn't exist is ignored. pslib's own warnings go to the standard error stream, not to 4D.
- **Preemptive processes:** the manifest declares both commands thread-safe, but pslib switches the process-wide numeric locale every time it writes a number (`setlocale(LC_NUMERIC, …)` in its output routine). Running these commands in several preemptive processes at the same time can produce numbers formatted with the wrong decimal separator, and the switch affects the rest of 4D as well. Prefer calling them from one process at a time.
- **Universal (Apple silicon) build pending.** The prebuilt libraries in `a/` (pslib, iconv, libpng, libtiff, libjpeg, zlib, jsoncpp) have no `arm64` slice, so the macOS build links for Intel (`x86_64`) only until they are rebuilt.

---

## Fonts collection

Both commands take the same first parameter: a collection of font objects. The position of a font in this collection is its **font index**, which you use in the `font` property of text items.

| Property | Type | Description |
|---|---|---|
| `name` | Text | Path to the font files **without** extension: pslib loads `name.afm`, and `name.pfb` when `embed` is `1` |
| `file` | Text | Path to an encoding file (`.enc`), or `""` to use the font's built-in encoding |
| `embed` | Longint | `1` to embed the font in the PostScript output, `0` to only reference it by name |

Every element must be an object. A non-object element is skipped, which shifts the index of every font after it.

From the plugin's own test method (`getFonts.4dm`):

```4d
$fonts:=New collection:C1472

$resPath:=Get 4D folder:C485(Current resources folder:K5:16)

$fonts[0]:=New object:C1471("name";$resPath+"AlteSchwabacher";"file";$resPath+"AlteSchwabacher.enc";"embed";1)
$fonts[1]:=New object:C1471("name";$resPath+"Helvetica";"file";"";"embed";0)

$0:=$fonts
```

---

## pslib generate

### Syntax

```
pslib generate ( fonts ; command ) → ps
```

| Parameter | Type | Description |
|---|---|---|
| `fonts` | Collection | Fonts to load, see [Fonts collection](#fonts-collection) |
| `command` | Object | Document metadata and pages, see below |
| Result | BLOB | The PostScript document |

### Description

`pslib generate` creates a document, sets its metadata, loads the fonts, then renders every page of `command.pages` in order. Save the result with `BLOB TO DOCUMENT` (as a `.ps` file) or send it to a PostScript printer.

**Document properties** (all optional):

| Property | Type | Description |
|---|---|---|
| `title`, `author`, `subject`, `keywords`, `creator` | Text | Document information, converted to Latin-1 |
| `boundingBox` | Text | PostScript bounding box, e.g. `"0 0 596 842"` |
| `orientation` | Text | Passed to pslib's `Orientation` header comment |
| `pages` | Collection | One object per page |

**Page object:**

| Property | Type | Description |
|---|---|---|
| `width` | Real | Page width in points. Default `596` (A4) |
| `height` | Real | Page height in points. Default `842` (A4) |
| `data` | Collection | Items to render on the page, in order |

**Items in `data`.** Each item is an object. The plugin handles, in this order, whichever of these keys it contains:

| Key | Other properties used | Effect |
|---|---|---|
| `font` | `size` | Select the font with this index and size for the following text |
| `text` | `encoding`, `x`, `y`, `params`, `values` | Draw a single line of text with its baseline at (`x`, `y`) |
| `textArea` | `encoding`, `x`, `y`, `width`, `height`, `mode`, `feature`, `params`, `values` | Draw text wrapped into the box whose lower left corner is (`x`, `y`) |
| `draw` | `x`, `y`, `width`, `height` | `"rect"` adds a rectangle path; `"stroke"` strokes the current path |

Coordinates are PostScript points, with the origin at the lower left corner of the page.

`mode` is pslib's alignment for boxed text: `"left"`, `"right"`, `"center"`, `"justify"` or `"fulljustify"`. `feature` is passed to pslib as is (for example `""`); check the pslib documentation for the values your version supports.

`params` and `values` set pslib parameters before the text is drawn, and they stay in effect for the rest of the document:

- `params`: an object of **text** values, passed to `PS_set_parameter`. Example: `"hyphenation";"true"`, `"hyphendict";<path to a .dic file>` (the path is converted on macOS).
- `values`: an object of **numeric** values, passed to `PS_set_value`. Examples: `charspacing`, `leading`, `parindent`, `parskip`, `parindentskip`, `hyphenminchars`.

The names are pslib's own; see the pslib documentation for the full list.

### Example

From the plugin's own test method (`TEST.4dm`), abridged. `geometry` and `getFonts` are the test project's own methods (see [pslib geometry](#pslib-geometry)):

```4d
$geometry:=geometry 
$fonts:=getFonts 

$resourcesFolderPath:=Get 4D folder:C485(Current resources folder:K5:16)

$command:=New object:C1471
$command.creator:="4D"
$command.author:="4D"
$command.title:="4D"
$command.keywords:="4D"
$command.boundingBox:="0 0 596 842"

$command.pages:=New collection:C1472

$page:=0

$command.pages[$page]:=New object:C1471
$command.pages[$page].width:=596
$command.pages[$page].height:=842

$command.pages[$page].data:=New collection:C1472

  //font
$command.pages[$page].data.push(New object:C1471("font";0;"size";15))

$data:=$geometry[0]

  // ... (title and first text area)

C_BLOB:C604($ps)

$ps:=pslib generate ($fonts;$command)

$path:=System folder:C487(Desktop:K41:16)+"test.ps"

BLOB TO DOCUMENT:C526($path;$ps)

OPEN URL:C673($path)
```

A single page with a heading and a framed box:

```4d
$command:=New object("title";"Report")
$page:=New object("width";596;"height";842;"data";New collection)

$page.data.push(New object("font";1;"size";18))
$page.data.push(New object("text";"Monthly report";"encoding";"latin1";"x";72;"y";770))

$page.data.push(New object("draw";"rect";"x";72;"y";500;"width";452;"height";240))
$page.data.push(New object("draw";"stroke"))

$page.data.push(New object("font";1;"size";10))
$page.data.push(New object("textArea";$body;"encoding";"latin1";\
	"x";80;"y";508;"width";436;"height";224;"mode";"justify"))

$command.pages:=New collection($page)
$ps:=pslib generate($fonts;$command)
```

---

## pslib geometry

### Syntax

```
pslib geometry ( fonts ; texts ) → geometry
```

| Parameter | Type | Description |
|---|---|---|
| `fonts` | Collection | Fonts to load, see [Fonts collection](#fonts-collection) |
| `texts` | Collection | Text objects to measure, see below |
| Result | Collection | One result object per text object, at the same index |

### Description

`pslib geometry` renders the texts off-screen and reports their size. Each object in `texts` is measured in one of two ways:

- **Without `width`**: the text is measured as a single line.
- **With `width`**: the text is fitted into a box of `width` × `height`, exactly as a `textArea` item of [`pslib generate`](#pslib-generate) would lay it out, and you get back the part that fits and the part that doesn't.

**Text object:**

| Property | Type | Description |
|---|---|---|
| `value` | Text | The text to measure |
| `encoding` | Text | 8-bit encoding of the font, e.g. `"latin1"` |
| `font` | Longint | Font index in the fonts collection |
| `size` | Real | Font size in points |
| `width`, `height` | Real | Box size, for the boxed mode |
| `mode`, `feature`, `params`, `values` | | Same meaning as for a `textArea` item of [`pslib generate`](#pslib-generate) |

**Result object, single-line mode:**

| Property | Type | Description |
|---|---|---|
| `encoding` | Text | The encoding you passed |
| `text` | Text | The text you passed |
| `width` | Real | Width of the text in points |
| `asc` | Real | Ascender height |
| `desc` | Real | Descender depth |

**Result object, boxed mode:**

| Property | Type | Description |
|---|---|---|
| `encoding` | Text | The encoding you passed |
| `text` | Text | The part of the text that fits in the box |
| `rest` | Text | The part that doesn't fit, or `""` |
| `boxheight` | Real | Height actually used by the text |
| `width`, `height` | Real | The box size you passed |

If the text can't be converted to `encoding`, the result object contains only `encoding`. With the fixed source in this repository, the same happens when `font` is not a valid index; earlier builds could stop without returning a value in that case. Non-object elements of `texts` leave their position in the result unset.

### Example

From the plugin's own test method (`geometry.4dm`): fit a long text into 200 × 630 point columns, calling the command again with the rest until everything is placed:

```4d
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
```

Center a heading using its measured width:

```4d
$g:=pslib geometry($fonts;New collection(New object("value";"Contents";"font";1;"size";18;"encoding";"latin1")))
$x:=(596-$g[0].width)/2
```

---

## Error handling & troubleshooting

- **Nothing is drawn for a text item.** The text contains characters that don't exist in `encoding`, so the iconv conversion failed. Use an encoding that covers the text, or remove those characters.
- **Text appears in the wrong font, or not at all.** The `font` index doesn't match the fonts collection (indices start at `0`, and non-object elements shift them), or pslib couldn't load the `.afm` file at `name`. pslib reports load failures on the standard error stream only.
- **Paths are not found on macOS.** Pass system (colon-separated) paths; the plugin converts them to POSIX itself. A POSIX path passed in would be converted again and break.
- **Hyphenation doesn't happen.** Set `"hyphenation";"true"` and `"hyphendict"` in `params`. In `pslib geometry` each call creates a new document, so pass `params` again on every call (the test method does this).
- **Metadata shows wrong characters.** `title`, `author`, `subject`, `keywords` and `creator` are converted to Latin-1; anything outside it can't be represented.
- **Numbers in the output use a comma as decimal separator.** Another process was running a pslib command at the same time and switched the numeric locale (see the preemptive note in [Requirements & platform notes](#requirements--platform-notes)). Run the commands one at a time.
- **Empty BLOB or result.** pslib couldn't create a document (out of memory), or, with the fixed source, an unexpected error occurred inside the command.

---

## Quick reference

```4d
$fonts:=New collection(New object("name";$res+"Helvetica";"file";"";"embed";0))

$g:=pslib geometry($fonts;New collection(\
	New object("value";$text;"font";0;"size";10;"encoding";"latin1";"width";200;"height";630;"mode";"justify")))
// $g[0].text fits, $g[0].rest doesn't

$page:=New object("width";596;"height";842;"data";New collection(\
	New object("font";0;"size";10);\
	New object("textArea";$g[0].text;"encoding";"latin1";"x";72;"y";100;"width";200;"height";630;"mode";"justify")))
$ps:=pslib generate($fonts;New object("title";"Doc";"pages";New collection($page)))
BLOB TO DOCUMENT(System folder(Desktop)+"doc.ps";$ps)
```
