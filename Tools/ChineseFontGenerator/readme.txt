Generate chinese font for the unique characters in a text file (encoded by UTF-16).

Input params: -f fontdefine [-border] -t textfile -o outname

  Params			Description

  fontdefine		File contains font define. File structure:
				1st line is font NAME. Ex: MS Gothic.
				2nd line is font TYPE. Ex: N (for normal), B (for Bold) and I (for Italic).
				3rd line is font SIZE. Ex: 12.
				4th line is the X offset of space character. Ex: -2.
				5th line is the width of the space character. Ex: 10.
				6th line is the space between 2 consecutive modules.
				7th line is the extra growth of the module size.

  -border			If enter this param, character will have a border.
  
  -uppercase			Lowercase fmodules changed to uppercase modules

  textfile			Filename containing the localization texts of game (encoded by UTF-16).

  outname			Filename without extension.
					outname.png will be the image
					outname.sprite will be the sprite
					outname.txt will be the text file with the unique characters ordered as in the image (UTF-16)

