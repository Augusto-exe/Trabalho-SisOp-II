#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <mutex>
#include <chrono>
#include <thread>
#include <bits/stdc++.h>
#include "./notificationManager.hpp"

using namespace std;

mutex mtx;

NotificationManager::NotificationManager()
{
    // carrega a lista de seguidores salva se tiver
    this->users = openFile();
}

void NotificationManager::create_user_if_not_found(string user)
{
    // check if exist on map
    UserMap::iterator it = this->users.find(user);
    bool found = it != this->users.end();

    if (!found)
    {
        // cria o user
        UserMapContent content;
        
        vector<string> followers;
        content.followersList = followers;

        vector<Notification> notificationList;
        content.notificationList = notificationList;

        vector<PendingNotification> pendingList;
        content.pendingList = pendingList;
        
        this->users[user] = content;
    }
}

bool NotificationManager::follow(string user, string followedUser)
{
    mtx.lock();

    create_user_if_not_found(followedUser);
    if (user != followedUser) 
    {
        bool alreadyFollows = find(
            this->users[followedUser].followersList.begin(),
                this->users[followedUser].followersList.end(),
                    user) != this->users[followedUser].followersList.end();
        if (!alreadyFollows)
        {
            this->users[followedUser].followersList.push_back(user);
            updateFile(this->users);
        }
        else 
        {
            cout << user << " already follows " << followedUser << "." << endl;
        }
    }
    else
    {
        cout << user << " attempted to follow himself." << endl;
    }
    mtx.unlock();

    return true;
}
void NotificationManager::tweetReceived(string user, string msg,int timestamp)
{
    Notification newNotification,lastNotification;
    PendingNotification newPending;
    SessionMap::iterator itSes;
    bool found;


    newNotification.timestamp = timestamp;
    newNotification.message = msg; 

    mtx.lock();
    newNotification.remainingFollowers = this->users[user].followersList.size();
        
    if(!this->users[user].notificationList.size())
    {
        newNotification.id = 0;   
    }
    else
    {
        lastNotification = this->users[user].notificationList.back();
        newNotification.id = lastNotification.id + 1;    
    }


    newPending.id = newNotification.id;
    newPending.sender = user;


    this->users[user].notificationList.push_back(newNotification); 
    for(auto itVec : this->users[user].followersList)
    {
        newPending.sessions.clear();
        create_user_if_not_found(itVec);
        itSes = this->sessionsQty.find(itVec);
        found = itSes != this->sessionsQty.end();

        if(found)
        {
            newPending.sessions = this->sessionsQty[itVec];
        }
        else
        {
            newPending.sessions.push_back(1);
        }

        this->users[itVec].pendingList.push_back(newPending);

    }
    mtx.unlock();
}

bool NotificationManager::needsToSend(string username, int session)
{
    bool need = false;
    PendingNotification pendingNot;
    
    mtx.lock();

    if(this->users[username].pendingList.size() > 0){
        for(auto pendingNot : this->users[username].pendingList){
            for (auto itSes = pendingNot.sessions.begin(); itSes != pendingNot.sessions.end(); ++itSes){
                if(*itSes == session)
                {
                    need = true;
                    break;
                }
            }   
        }
    }
    mtx.unlock();
    return need;


}

packet NotificationManager::consumeTweet(string username,int session)
{
    PendingNotification foundNot;
    packet notificationPkt;
    bool shouldErase = false;
    bool shouldBreak = false;
    vector<PendingNotification>::iterator itPending;
    mtx.lock();

    for(auto pendingNot = this->users[username].pendingList.begin(); pendingNot != this->users[username].pendingList.end(); ++pendingNot ){
        for (auto itSes = (*pendingNot).sessions.begin(); itSes != (*pendingNot).sessions.end(); ++itSes){
            if(*itSes == session)
            {                
                foundNot = (*pendingNot);
                this->users[username].pendingList[distance(this->users[username].pendingList.begin(),pendingNot)].sessions.erase(itSes);
                if(this->users[username].pendingList[distance(this->users[username].pendingList.begin(),pendingNot)].sessions.size() == 0)
                {
                
                    itPending = pendingNot;
                    shouldErase = true;
                }
                
                shouldBreak = true;
                break;
            }
        }

        if (shouldBreak) 
        {
            break;
        }
    }

    for (auto itVet = this->users[foundNot.sender].notificationList.begin(); itVet != this->users[foundNot.sender].notificationList.end(); ++itVet)
    {
        if((*itVet).id == foundNot.id)
        {   
            strcpy(notificationPkt._payload, (*itVet).message.c_str());
            notificationPkt.type = TIPO_NOTI;
            notificationPkt.timestamp = (*itVet).timestamp;
            strcpy(notificationPkt.user, foundNot.sender.c_str());
            notificationPkt.length = strlen(notificationPkt._payload);

            if(shouldErase)
            {
                this->users[username].pendingList.erase(itPending);
                (*itVet).remainingFollowers -=1;
                if((*itVet).remainingFollowers == 0)
                {
                    this->users[foundNot.sender].notificationList.erase(itVet);
                }
            }
            
            break;
        } 
    }
    mtx.unlock();
    return notificationPkt;
}

void NotificationManager::updateFile(UserMap users)
{
    ofstream stream;

    stream.open("followersList.txt");
    
    if (stream)
    {
        for (auto itMap:users)
        {
            stream << endl << itMap.first << ":";
            for (auto itVec:itMap.second.followersList)
            { 
                stream << " " << itVec;
            }
        }
    }

    stream.close();
}


UserMap NotificationManager::openFile()
{
    ifstream stream;
    string username, follower, followers;

    UserMap users;

    stream.open("followersList.txt");
    stream.ignore();
    cout << "User: Followers List" << endl;
    while (stream.good())
    {
        getline(stream, username, ':');
        stream.ignore();
        getline(stream, followers);
        istringstream iss(followers);
        cout << username << ":";
        while(getline(iss, follower, ' '))
        {
            cout << " " << follower;
            users[username].followersList.push_back(follower);
        }
        cout << endl;
    }
    stream.close();

    return users;
}

// returns the sessionID or -1 if the connection failed
int NotificationManager::add_session(string username)
{
    
    // check if exist on map
    SessionMap::iterator it = this->sessionsQty.find(username);
    bool found = it != this->sessionsQty.end();

    
    // if not, add it with 1
    if (!found)
    {
        sessionsQty[username].push_back(1);
        SessionMap::iterator it = this->sessionsQty.find(username);
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
    if(current.front() == 1)  //TO-DO: arrumar pra ficar generalizado para N sessões
        it->second.push_back(2);
    else
        it->second.push_back(1);
    return it->second.back();
}

bool NotificationManager::del_session(string username,int session)
{

    // check if exist on map
    SessionMap::iterator it = this->sessionsQty.find(username);
    bool found = it != this->sessionsQty.end();

    // if not, add it with 1
    if (!found)
    {
        return true;
    }

    if (it->second.size() == 1)
    {
        sessionsQty.erase(username);
        return true;
    }

    for (auto itVec = this->sessionsQty[username].begin(); itVec != this->sessionsQty[username].end(); ++itVec)
    {
        if(*itVec == session)
        {
            this->sessionsQty[username].erase(itVec);
            break;
        }
    }
  

    return true;
}