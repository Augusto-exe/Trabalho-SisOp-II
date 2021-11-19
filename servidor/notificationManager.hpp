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
typedef struct sockaddr_in Sockaddr_in;

typedef std::map<string, vector<int>> SessionMap;
typedef std::map<string, Sockaddr_in> SessionAddressMap;

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
    bool follow(string user, string followedUser,bool leader);
    void tweetReceived(string user, string msg,int timestamp);
    bool needsToSend(string username,int session);
    packet consumeTweet(string username,int session);
    int add_session(string username, Sockaddr_in a);
    bool del_session(string username, int session);
    UserMap users;
    SessionMap sessionsQty;
    SessionAddressMap sessionAddresses;
    
private:
    void create_user_if_not_found(string user);
    void updateFile(UserMap users);
    UserMap openFile();

};