usage:
dnltool.exe configfile outputfile


http://tdw0.gameloft.net/dw64.php----------------------------------------url
X:\RAINBOWSIXIV3D\Release\R6\------------------------------------path for following resource.
3----------------------------------------------------------------------------------resource num
res1.bar res1 0--------------------------------------------------------------res1.bar(resource name) res1(url menu id) 0/-1/1(need download/already on disk and can't be delete/already on disk.)
res2.bar res2 -1
res3.bar res3 1


dnl.bar format

+-----+
|        |
+-----+    stand for one int.

+=====+
| ...........|
+=====+   stand for some bytes.


+----------+
|   GGI   |
+----------+
+--------------------+==============================+
|     length of url   |     url string.......(length bytes)..................|
+--------------------+==============================+
+------------------------------+
|   The number of level   |
+------------------------------+

//chunk of one file record
+-------------------------------+===========================+
|   Length of file name    |   file name.........(length bytes)............|
+-------------------------------+===========================+
+-------------------------------+===========================+
|   Length of menu id       |   menu id string...(length bytes).........|
+-------------------------------+===========================+
+-------------------------------+-------------------------------+
|   length of file                 |   status                           +
+-------------------------------+-------------------------------+