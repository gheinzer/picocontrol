#include <vector>
#include <cstring>
#include <WebFS.h>

__WebFS WebFS;

extern char webfs_start;
extern char webfs_end;

std::vector<WebFS_Handle> webfs_files;
size_t webfs_file_count = 0;

void __WebFS::init() {
    // Read the file list from webfs
    char* webfs_seek = &webfs_start;
    
    webfs_file_count = *((size_t*)webfs_seek);
    webfs_seek += sizeof(size_t);

    // Read the file header
    for(int i = 0; i < webfs_file_count; i++) {
        WebFS_Handle file;

        // Read file path and MIME type
        file.path = (WebFS_Path)webfs_seek;
        webfs_seek += file.path.length() + 1;

        file.mime_type = (std::string)webfs_seek;
        webfs_seek += file.mime_type.length() + 1;

        // The file size is aligned to 4-byte boundaries
        webfs_seek += 4 - ((size_t)(webfs_seek) % 4);
        file.size = *((WebFS_FileSize*)webfs_seek);
        webfs_seek += sizeof(WebFS_FileSize);

        file.content = 0;
        webfs_files.push_back(file);
    }

    // Assign the file content pointer
    for(int i = 0; i < webfs_files.size(); i++) {
        webfs_files[i].content = webfs_seek;
        webfs_seek += webfs_files[i].size;
    }
}

WebFS_Handle* __WebFS::getFile(WebFS_Path path) {
    for(WebFS_Handle& handle : webfs_files) {
        if(handle.path == path) return &handle;
    }
    return 0;
}