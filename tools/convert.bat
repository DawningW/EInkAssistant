@echo off
echo PLEASE EDIT THIS BAT FILE WITH CORRECT CONFIGURATION BEFORE RUN IT.
echo 请在运行本BAT文件前先修改为正确的设置


REM True type font / 字体文件
SET FONT_FILE=MFXinRan_Noncommercial-Regular.ttf
REM font name(please just use English chars) / 字体名称(请仅使用英文字符). 
SET FONT_NAME=mfxinran
REM font size / 要生成的字体大小
SET FONT_SIZE=16 24 36 60 80 84 88 92

FOR %%s IN (%FONT_SIZE%) DO CALL:ProcessFont %FONT_FILE%,%FONT_NAME%,%%s  

FOR /F "delims==" %%i IN ('dir code\u8g2_%FONT_NAME%_*.c /b') DO (CALL:FinishSourceCode %FONT_NAME%,%%~ni)


GOTO:EOF

:FinishSourceCode
	SET FN=%1
	SET CN=%2

	ECHO #include "%CN%.h" > code\temp.c
	TYPE code\%CN%.c >> code\temp.c
	DEL code\%CN%.c
	MOVE /y code\temp.c code\%CN%.c > NUL
	
	CALL :ToUpper %CN%,cnLowCase
        ECHO #ifndef _%cnLowCase%_H > code\temp.h
	ECHO #define _%cnLowCase%_H >> code\temp.h
	TYPE header1.txt >> code\temp.h
	ECHO extern const uint8_t %CN%[] U8G2_FONT_SECTION("%CN%"); >> code\temp.h
	TYPE header2.txt >> code\temp.h
	MOVE /y code\temp.h code\%cn%.h > NUL

GOTO:EOF

:ToUpper
	set upper=
	set "str=%~1%"
	for /f "skip=2 delims=" %%I in ('tree "\%str%"') do if not defined upper set "upper=%%~I"
	set "upper=%upper:~3%"
	set "%~2=%upper%"
GOTO:EOF

:ProcessFont
	SET FF=%1
	SET FN=%2
	SET FS=%3
	SET BDF=bdf/%FN%-%FS%.bdf
	echo Try to generate %BDF% ......
	otf2bdf -r 72 -p %FS% -o %BDF% font/%FF%
	CALL:CreateSourceCode %FN%,%FS%
GOTO:EOF

:CreateSourceCode
	SET FN=%1
	SET FS=%2
	SET BDF=bdf/%FN%-%FS%.bdf
	echo Try to generate source code code/u8g2_%FN%_%FS%_number.c
	bdfconv -b 0 -f 1 -m "48-57" -n u8g2_%FN%_%FS%_number -o code/u8g2_%FN%_%FS%_number.c %BDF%
	echo Try to generate source code code/u8g2_%FN%_%FS%_tu.c
	bdfconv -b 0 -f 1 -m "32-95" -n u8g2_%FN%_%FS%_tu -o code/u8g2_%FN%_%FS%_tu.c %BDF%
	echo Try to generate source code code/u8g2_%FN%_%FS%_tr.c
	bdfconv -b 0 -f 1 -m "32-127" -n u8g2_%FN%_%FS%_tr -o code/u8g2_%FN%_%FS%_tr.c %BDF%
	echo Try to generate source code code/u8g2_%FN%_%FS%_gb2312.c
	bdfconv -b 0 -f 1 -M maps/gb2312.map -n u8g2_%FN%_%FS%_gb2312 -o code/u8g2_%FN%_%FS%_gb2312.c %BDF%
	echo Try to generate source code code/u8g2_%FN%_%FS%_gb2312a.c
	bdfconv -b 0 -f 1 -M maps/gb2312a.map -n u8g2_%FN%_%FS%_gb2312a -o code/u8g2_%FN%_%FS%_gb2312a.c %BDF%
	echo Try to generate source code code/u8g2_%FN%_%FS%_gb2312b.c
	bdfconv -b 0 -f 1 -M maps/gb2312b.map -n u8g2_%FN%_%FS%_gb2312b -o code/u8g2_%FN%_%FS%_gb2312b.c %BDF%
	echo Try to generate source code code/u8g2_%FN%_%FS%_chinese1.c
	bdfconv -b 0 -f 1 -M maps/chinese1.map -n u8g2_%FN%_%FS%_chinese1 -o code/u8g2_%FN%_%FS%_chinese1.c %BDF%
	echo Try to generate source code code/u8g2_%FN%_%FS%_chinese2.c
	bdfconv -b 0 -f 1 -M maps/chinese2.map -n u8g2_%FN%_%FS%_chinese2 -o code/u8g2_%FN%_%FS%_chinese2.c %BDF%
	echo Try to generate source code code/u8g2_%FN%_%FS%_chinese3.c
	bdfconv -b 0 -f 1 -M maps/chinese3.map -n u8g2_%FN%_%FS%_chinese3 -o code/u8g2_%FN%_%FS%_chinese3.c %BDF%
GOTO:EOF