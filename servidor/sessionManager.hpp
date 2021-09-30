#ifndef __session_manager_hpp__
#define __session_manager_hpp__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>

using namespace std;

typedef std::map<string, int> Map;

class SessionManager
{
public:
    SessionManager();
    int add_session(string username);
    bool del_session(string username);

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