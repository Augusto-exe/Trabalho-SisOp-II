#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <map>
#include "./sessionManager.hpp"

#define MAX_SESSIONS 2

using namespace std;

Map sessionsQty;

SessionManager::SessionManager()
{
}

// returns the sessionID or -1 if the connection failed
int SessionManager::add_session(string username)
{
    // check if exist on map
    Map::iterator it = sessionsQty.find(username);
    bool found = it != sessionsQty.end();

    // if not, add it with 1
    if (!found)
    {
        sessionsQty[username] = 1;
        Map::iterator it = sessionsQty.find(username);
        return 1;
    }

    int current = it->second;

    // if already on max sessions, return false
    if (current >= MAX_SESSIONS)
    {
        return -1;
    }

    //  increment the current value and return true
    it->second = current + 1;
    return current;
}

bool SessionManager::del_session(string username)
{
    // check if exist on map
    Map::iterator it = sessionsQty.find(username);
    bool found = it != sessionsQty.end();

    // if not, add it with 1
    cout << "found?" << found << endl;
    if (!found)
    {
        return true;
    }

    int current = it->second;
    cout << "current?" << current << endl;
    if (current == 1)
    {
        cout << "deleta" << endl;
        it->second = 0;
        sessionsQty.erase(username);
        return true;
    }

    cout << "nao entrou no if" << endl;

    it->second = current - 1;
    return true;
}