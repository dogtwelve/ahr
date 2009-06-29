for %%f in (*.ase) do cscript anim-extract.vbs %%f %%~nf.txt
type crash-frontal-70.txt crash-frontal-100.txt crash-frontal-wall-70.txt crash-frontal-wall-100.txt crash-frontal-wall-200.txt crash-wall-A.txt crash-wall-B.txt crash-wall-C.txt crash-wall-D.txt crash-wall-E.txt poussette-A.txt poussette-B.txt poussette-C.txt poussette-lateral-70.txt poussette-lateral-90.txt poussette-lateral-100.txt > CarAnimData.h
pause

