
struct ArchiveFileStruct
{
    bool    bMustBeCompressed;
    char*   cFileName;
};

void PrintUsage();
void ClearDataFiles();
void ArchiveFiles(std::deque<ArchiveFileStruct*>*);


