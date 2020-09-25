#include "database.h"
#include <stdexcept>

namespace sqlite3db {

int bindArg(Statement& s, int64_t arg)
{
    return sqlite3_bind_int64(s.stmt, s.pos++, arg);
}

int bindArg(Statement& s, const char* arg)
{
    return sqlite3_bind_text(s.stmt, s.pos++, arg, arg ? (int)strlen(arg) : 0,
                             SQLITE_TRANSIENT);
}

int bindArg(Statement& s, const std::string& arg)
{
    return sqlite3_bind_text(s.stmt, s.pos++, arg.c_str(), (int)arg.length(),
                             SQLITE_TRANSIENT);
}

void check_type(sqlite3_stmt* s, int pos, int typ = -1)
{
    auto t = sqlite3_column_type(s, pos);
    if (t == SQLITE_NULL)
        throw db_exception("Null value");
    if (typ != -1 && t != typ)
        throw std::invalid_argument("Not correct type");
}

template <> int stepper(sqlite3_stmt* s, int pos)
{
    check_type(s, pos, SQLITE_INTEGER);
    return sqlite3_column_int(s, pos);
}

template <> uint64_t stepper(sqlite3_stmt* s, int pos)
{
    check_type(s, pos, SQLITE_INTEGER);
    return (uint64_t)sqlite3_column_int(s, pos);
}

template <> double stepper(sqlite3_stmt* s, int pos)
{
    check_type(s, pos, SQLITE_FLOAT);
    return sqlite3_column_double(s, pos);
}

template <> const char* stepper(sqlite3_stmt* s, int pos)
{
    check_type(s, pos, SQLITE_TEXT);
    return (const char*)sqlite3_column_text(s, pos);
}

template <> std::string stepper(sqlite3_stmt* s, int pos)
{
    auto t = sqlite3_column_type(s, pos);
    if (t == SQLITE_NULL)
        return "";
    auto* x = (const char*)sqlite3_column_text(s, pos);
    return std::string(x ? x : "");
}

template <> std::vector<uint8_t> stepper(sqlite3_stmt* s, int pos)
{
    check_type(s, pos);
    const void* ptr = sqlite3_column_blob(s, pos);
    int size = sqlite3_column_bytes(s, pos);

    std::vector<uint8_t> res(size);
    memcpy(&res[0], ptr, size);
    return res;
}

template <> std::vector<uint64_t> stepper(sqlite3_stmt* s, int pos)
{
    check_type(s, pos);
    const void* ptr = sqlite3_column_blob(s, pos);
    int size = sqlite3_column_bytes(s, pos);

    std::vector<uint64_t> res(size / 8);
    memcpy(&res[0], ptr, size);
    return res;
}

} // namespace sqlite3db
