#pragma once

#include <sys/_default_fcntl.h>

namespace zest::fs {
    struct FileFlags {
        FileFlags(int flags)
            : read(flags & O_RDONLY),
              write(flags & O_WRONLY || flags & O_CREAT),
              append(flags & O_APPEND),
              truncate(flags & O_TRUNC),
              create_new(flags & O_CREAT && flags & O_EXCL) {}
    
        bool read : 1;
        bool write : 1;
        bool append : 1;
        bool truncate : 1;
        bool create_new : 1;
    };
    } // namespace zest::fs