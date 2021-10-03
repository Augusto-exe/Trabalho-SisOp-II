#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <mutex>
#include <chrono>
#include <thread>
#include "./notificationManager.hpp"

using namespace std;

mutex mtx;

NotificationManager::NotificationManager()
{
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
    cout << "A mimir (" << user << " tentando seguir " << followedUser << ")" << endl;
    this_thread::sleep_for(chrono::milliseconds(20000));
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
