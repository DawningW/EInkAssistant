@echo off

SET FONT_FILE=qweather-icons.ttf
SET FONT_NAME=font_qweather_icon
SET FONT_SIZE=12 16 24 36
SET FONT_MAP=qwicon

SET FONT_FILE2=vanfont.ttf
SET FONT_NAME2=font_bili_icon
SET FONT_SIZE2=16
SET FONT_MAP2=vanfont

FOR %%s IN (%FONT_SIZE%) DO CALL:ProcessFont %FONT_FILE%,%FONT_NAME%,%FONT_MAP%,%%s  
FOR %%s IN (%FONT_SIZE2%) DO CALL:ProcessFont %FONT_FILE2%,%FONT_NAME2%,%FONT_MAP2%,%%s  
GOTO:EOF

:ProcessFont
	SET FF=%1
	SET FN=%2
	SET FM=%3
	SET FS=%4
	SET BDF=bdf/%FN%-%FS%.bdf
	echo Try to generate %BDF% ......
	otf2bdf -r 100 -p %FS% -o %BDF% font/%FF%
	CALL:CreateSourceCode %FN%,%FM%,%FS%
GOTO:EOF

:CreateSourceCode
	SET FN=%1
	SET FM=%2
	SET FS=%3
	SET BDF=bdf/%FN%-%FS%.bdf
	echo Try to generate source code code/u8g2_%FN%_%FS%_%FONT_MAP%.c
	bdfconv -b 0 -f 1 -M maps/%FM%.map -n u8g2_%FN%_%FS%_%FM% -o code/u8g2_%FN%_%FS%_%FM%.c %BDF%
GOTO:EOF
