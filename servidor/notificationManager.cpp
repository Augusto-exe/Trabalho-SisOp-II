#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include "./notificationManager.hpp"

using namespace std;

struct pendingNotification
{
    string sender;
    int id;
};
typedef struct pendingNotification PendingNotification;

struct notification
{
    int id;
    int timestamp;
    string message; //todo: tamanho maximo     int size;     int pending; };  typedef struct notification Notification;

}; 
typedef struct notification Notification;

struct userMapContent
{
    vector<string> followersList;
    vector<Notification> notificationList;
    vector<PendingNotification> pendingList;
};
typedef userMapContent UserMapContent;

NotificationManager::NotificationManager()
{
}