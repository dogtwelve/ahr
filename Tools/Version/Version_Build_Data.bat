call Version_SetVar

pushd ..\..\Data
	call MakeClean
	call MakeData
	call MakeData
	call ArchiveFiles
popd