#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include "./notificationManager.hpp"

using namespace std;

UserMap users;

NotificationManager::NotificationManager()
{
}

void NotificationManager::create_user_if_not_found(string user)
{
    // check if exist on map
    UserMap::iterator it = users.find(user);
    bool found = it != users.end();

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
        users[user] = content;
    }
}

bool NotificationManager::follow(string user, string followedUser)
{
    create_user_if_not_found(followedUser);

    users[followedUser].followersList.push_back(user);

    return true;
}
