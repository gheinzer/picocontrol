#include <string>

typedef size_t WebFS_FileSize;
typedef std::string WebFS_Path;

typedef struct {
    WebFS_Path path;
    WebFS_FileSize size;
    const char* content;
    std::string mime_type;
} WebFS_Handle;

class __WebFS {
    public:
        void init();
        WebFS_Handle* getFile(WebFS_Path path);
};

extern __WebFS WebFS;