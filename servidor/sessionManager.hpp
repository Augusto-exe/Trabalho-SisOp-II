#ifndef __session_manager_hpp__
#define __session_manager_hpp__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

struct char_cmp
{
    bool operator()(const char *a, const char *b) const
    {
        return strcmp(a, b) < 0;
    }
};

typedef std::map<const char *, int, char_cmp> Map;

class SessionManager
{
public:
    SessionManager();
    bool add_session(char *username);
    bool del_session(char *username);

private:
    /*
    sessionsQty:
        usuarios logados:
        (user -> num logins)
        @matheus -> 1
        @argelxd -> 2
    */
    
};

#endif