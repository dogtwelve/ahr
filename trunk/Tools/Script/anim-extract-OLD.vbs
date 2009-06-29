set arg = WScript.Arguments
set shell = CreateObject("WScript.Shell")
set fso = CreateObject("Scripting.FileSystemObject")


source=arg(0)
dest=arg(1)

if fso.FileExists(dest) then fso.DeleteFile(dest)
set file = fso.OpenTextFile(dest,8,true)

set ifile = fso.OpenTextFile(source)
do while not ifile.AtEndOfStream
line = ifile.ReadLine
line=trim(replace(line,"	"," "))
sp = split(line," ")
if trim(sp(0))="*CONTROL_ROT_SAMPLE" then
	if cint(sp(1)) > cint(count) then count = cint(sp(1))
end if
if trim(sp(0))="*CONTROL_POS_SAMPLE" then
	if cint(sp(1)) > cint(count) then count = cint(sp(1))
end if
loop
ifile.close
fcount=count
count = count / 240





set ifile = fso.OpenTextFile(source)
file.WriteLine("enum { k_nFrameCount=" &count+1& "};")
file.WriteLine("const int frot [k_nFrameCount*4] = {")

rotcount=0
do while not ifile.AtEndOfStream
line = ifile.ReadLine

line=trim(replace(line,"	"," "))
sp = split(line," ")

if trim(sp(0))="*CONTROL_ROT_SAMPLE" then
	do while rotcount <> cint(sp(1))
		file.WriteLine("	0, 0, 16384, 0,")
		rotcount = rotcount + 240
	loop

	x = clng(sp(2) * 16384)
	y = clng(sp(3) * 16384)
	z = clng(sp(4) * 16384)
	s = clng((sp(5) * 1024) \ 3.14159)


	file.WriteLine("	" &x& ", " &y& ", " &z& ", " &s& ",")

	rotcount = rotcount + 240


end if

loop

do while rotcount <= fcount
	file.WriteLine("		0, 0, 16384, 0,")
	rotcount = rotcount + 240
loop


file.WriteLine("};")


ifile.close


set ifile = fso.OpenTextFile(source)
file.WriteLine("const int ftrans [k_nFrameCount*3] = {")
x=0
y=0
z=0
transcount=0
do while not ifile.AtEndOfStream
line = ifile.ReadLine

line=trim(replace(line,"	"," "))
sp = split(line," ")
if trim(sp(0))="*CONTROL_POS_SAMPLE" then

	do while transcount <> cint(sp(1))
		file.WriteLine("	" &x& ", " &y& ", " &z& ",")
		transcount = transcount + 240
	loop


	x = clng(sp(2) * 256)
	y = clng(sp(3) * 256)
	z = clng(sp(4) * 256)

	file.WriteLine("	" &x& ", " &y& ", " &z& ",")

	transcount = transcount + 240

end if

loop

do while transcount <= fcount
	file.WriteLine("	" &x& ", " &y& ", " &z& ",")
	transcount = transcount + 240
loop

ifile.close

file.WriteLine("};")





