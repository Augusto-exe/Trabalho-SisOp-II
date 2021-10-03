#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
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