#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include "../common.h"

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
    int remainingFollowers;
};
typedef struct notification Notification;

struct userMapContent
{
    vector<string> followersList;
    vector<Notification> notificationList;
    vector<PendingNotification> pendingList;
};
typedef userMapContent UserMapContent;

typedef std::map<string, UserMapContent> UserMap;

class NotificationManager
{
public:
    NotificationManager();
    bool follow(string user, string followedUser);
    void tweetReceived(string user, string msg,int timestamp);
    bool needsToSend(string username);
    packet consumeTweet(string username);
    UserMap users;
    
private:
    void create_user_if_not_found(string user);
    void updateFile(UserMap users);
    UserMap openFile();

};