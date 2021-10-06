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
    // cout << "A mimir (" << user << " tentando seguir " << followedUser << ")" << endl;
    // this_thread::sleep_for(chrono::milliseconds(20000));
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

    newNotification.timestamp = timestamp;
    newNotification.message = msg;
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
        create_user_if_not_found(itVec);
        this->users[itVec].pendingList.push_back(newPending);
        cout << itVec << " :" << endl;
        for(auto itVec2 : this->users[itVec].pendingList)
            cout << itVec2.sender << " " << itVec2.id << endl;
    }



}

bool NotificationManager::needsToSend(string username)
{
    bool teste;
    mtx.lock();
        teste = (this->users[username].pendingList.size() > 0);
    mtx.unlock();
    return teste;


}

packet NotificationManager::consumeTweet(string username)
{
    PendingNotification pendingNot;
    packet notificationPkt;
    pendingNot = this->users[username].pendingList.front();
    this->users[username].pendingList.erase(this->users[username].pendingList.begin());
    
    for (auto itVet : this->users[pendingNot.sender].notificationList)
    {
        if(itVet.id == pendingNot.id)
        {   
            if(itVet.remainingFollowers >0)
            {
                itVet.remainingFollowers -=1;
                strcpy(notificationPkt._payload, itVet.message.c_str());
                notificationPkt.type = TIPO_NOTI;
                notificationPkt.timestamp = itVet.timestamp;
                strcpy(notificationPkt.user, pendingNot.sender.c_str());
                notificationPkt.length = strlen(notificationPkt._payload);    
            }
            else{
                //this->users[pendingNot.sender].notificationList.erase(itVet);
            }
            break;
        } 
    }
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