set JAVA=%JDK_PATH%\bin\java.exe

del *.dat

call JAVA -jar BuildBitmapFont.jar

call JAVA -jar BuildBitmapFont.jar -f font_small.def -border -t A4_Book.txt -o A4_BitmapFont_small.dat
call JAVA -jar BuildBitmapFont.jar -f font_large.def -border -t A4_Book.txt -o A4_BitmapFont_large.dat


rem copy A3_BitmapFont.dat C:\Projects_C\A3_NGI\data\bin\game\A4_BitmapFont_small.dat
rem copy A3_BitmapFont.dat C:\Projects_C\A3_NGI\data\bin\game\A4_BitmapFont_large.dat

pause