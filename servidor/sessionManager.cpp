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
        sessionsQty[username].push_back(1);
        Map::iterator it = sessionsQty.find(username);
        return 1;
    }

    auto current = it->second;

    // if already on max sessions, return false
    if (current.size() >= MAX_SESSIONS)
    {
        cout << "limite excedido " << current.size() << endl;
        return -1;
    }

    //  increment the current value and return true
    if(current.front() == 1)
        it->second.push_back(2);
    else
        it->second.push_back(1);
    return it->second.back();
}

bool SessionManager::del_session(string username,int session)
{
    // check if exist on map
    Map::iterator it = sessionsQty.find(username);
    bool found = it != sessionsQty.end();

    // if not, add it with 1
    if (!found)
    {
        return true;
    }

    vector<int> current = it->second;
    if (current.size() == 1)
    {
        sessionsQty.erase(username);
        return true;
    }

    for (auto itVec = it->second.begin(); itVec != it->second.end(); ++itVec)
    {
        if(*itVec == session)
            it->second.erase(itVec);
    }

  

    return true;
}