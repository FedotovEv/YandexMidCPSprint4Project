
#ifdef _MSC_VER
    #define popen _popen
    #define pclose _pclose
    #define WIFEXITED(status) ((status) != -1)
    #define WEXITSTATUS(status) ((status) & 0xff)
#endif
