#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <map>
#include "../common.h"

#define MAX_SESSIONS 2

using namespace std;

struct pendingNotification
{
    string sender;
    int id;
    vector<int> sessions;
};
typedef struct pendingNotification PendingNotification;

typedef std::map<string, vector<int>> SessionMap;

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
    int add_session(string username);
    bool del_session(string username, int session);
    UserMap users;
    SessionMap sessionsQty;
    
private:
    void create_user_if_not_found(string user);
    void updateFile(UserMap users);
    UserMap openFile();

};