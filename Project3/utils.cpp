#include <string>


std::string string_format(const char *fmt, ...)
{
    char *ret;
    va_list ap;

    va_start(ap, fmt);
    vasprintf(&ret, fmt, ap);
    va_end(ap);

    std::string str(ret);
    free(ret);

    return str;
}
void append_format(std::string& str, const char *fmt, ...)
{
    va_list ap;
    char *ret;

    va_start(ap, fmt);
    vasprintf(&ret, fmt, ap);
    va_end(ap);

    str.append(ret);
    free(ret);
}