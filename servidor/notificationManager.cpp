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
#include <netinet/in.h>
#include <sys/socket.h>
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

bool NotificationManager::follow(string user, string followedUser,bool leader)
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
            //Se o usuario ainda nao segue, adiciona user na lista de seguidores
            this->users[followedUser].followersList.push_back(user);
            if(leader)
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
        
    //Gerando id de Notifica????o Caso n??o tenha notifica????es id=0 caso contrario id = max(id)+1    
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


    //Adiciona a nova notifica????o ao fiz da lista de notifica????oes do usu??rio que gerou
    this->users[user].notificationList.push_back(newNotification); 

    //Percorre todos os seguidores do usu??rio e adiciona uma nova notifica????o pendente ao fim da lista de notifica????es pendentes deles
    for(auto itVec : this->users[user].followersList)
    {
        newPending.sessions.clear();
        create_user_if_not_found(itVec);
        itSes = this->sessionsQty.find(itVec);
        found = itSes != this->sessionsQty.end();

        //Verifica se existem sess??es do seguidor
        if(found)
        {
            //Caso existam, adiciona a lista de sess??es ?? pendingNotification
            newPending.sessions = this->sessionsQty[itVec];
        }
        else
        {
            //Caso contr??rio, coloca a sess??o 1 (primeira que o user receber?? ao fazer login)
            newPending.sessions.push_back(1);
        }

        //Adiciona a pending notification ?? lista
        this->users[itVec].pendingList.push_back(newPending);

    }
    mtx.unlock();
}

bool NotificationManager::needsToSend(string username, int session)
{
    bool need = false;
    PendingNotification pendingNot;
    
    mtx.lock();

    //Verifica se a pending list do usu??rio possui notifica????es pendentes
    if(this->users[username].pendingList.size() > 0){
        //percorre todas as notifica????es
        for(auto pendingNot : this->users[username].pendingList){
            for (auto itSes = pendingNot.sessions.begin(); itSes != pendingNot.sessions.end(); ++itSes){
                //Caso ache uma notifica????o que inclua a sess??o atual entre as sess??es pendentes, indica que precisa enviar
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
    //percorre a pending list do usu??rio que deseja consumir um tweet
    for(auto pendingNot = this->users[username].pendingList.begin(); pendingNot != this->users[username].pendingList.end(); ++pendingNot ){
        //para cada pending notification, verifica se possui o id de sess??o igual ao da sess??o atual
        for (auto itSes = (*pendingNot).sessions.begin(); itSes != (*pendingNot).sessions.end(); ++itSes){
            if(*itSes == session)
            {   
                // ao encontrar a notifica????o desejada a seleciona e verifica se a pending notification deve ser exclu??da             
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

    //percorre as notifica????es da lista de notifica????es do usu??rio que gerou a pending notification
    for (auto itVet = this->users[foundNot.sender].notificationList.begin(); itVet != this->users[foundNot.sender].notificationList.end(); ++itVet)
    {
        //encontra a notifica????o com id indicado na pending notification
        if((*itVet).id == foundNot.id)
        {   
            //gera o pacote que ser?? enviado
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

//Atualiza o arquivo de seguidores.
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

//Abre o arquivo de seguidores e povoa o UserMap de acordo.
UserMap NotificationManager::openFile()
{
    ifstream stream;
    string username, follower, followers;
    char c;
    UserMap users;

    stream.open("followersList.txt");
    stream.ignore();
    cout << "User: Followers List" << endl;
    while (stream.good())
    {
        getline(stream, username, ':');
        stream.get(c);
        if(c == ' ')
        {
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
    }
    stream.close();

    return users;
}

// returns the sessionID or -1 if the connection failed
int NotificationManager::add_session(string username, Sockaddr_in sessionAddress)
{
    
    // check if exist on map
    SessionMap::iterator it = this->sessionsQty.find(username);
    bool found = it != this->sessionsQty.end();

    
    // if not, add it with 1
    if (!found)
    {
        sessionsQty[username].push_back(1);
        sessionAddresses[username + "#1"] = sessionAddress;
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
    if(current.front() == 1)  //TO-DO: arrumar pra ficar generalizado para N sess??es
    { 
        it->second.push_back(2);
        sessionAddresses[username + "#2"] = sessionAddress;
    }
    else 
    {
        it->second.push_back(1);
        sessionAddresses[username + "#1"] = sessionAddress;
    }

    for (auto &session : sessionsQty)
    {
        cout << "username: " << session.first << endl;
        for (auto &sessionId : session.second)
        {
            cout << "sessionId: " << sessionId << endl;
        }
    }

    for (auto &sessionAddr : sessionAddresses)
    {
        cout << "username: " << sessionAddr.first << " port: " << 
        sessionAddr.second.sin_port << " ip: " << 
        sessionAddr.second.sin_addr.s_addr << endl;
    }

    return it->second.back();
}

bool NotificationManager::del_session(string username,int session)
{

    cout << username <<"-" << endl;
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
        
        sessionAddresses.erase(username+"#"+to_string(session));
        
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

    
    for (auto itMap = this->sessionAddresses.begin() ; itMap != this->sessionAddresses.end(); ++itMap)
    {
        cout << (*itMap).first  << endl;
        if((*itMap).first == username+"#"+to_string(session))
        {
            this->sessionAddresses.erase(itMap);
            break;
        }
    }
    

    return true;
}

int NotificationManager::add_session_from_server(string username, int session, in_addr_t addr, in_port_t port)
{
    Sockaddr_in sessionAddress;
    sessionAddress.sin_port = port;
    sessionAddress.sin_addr.s_addr = addr;
    sessionAddresses[username + "#"+to_string(session)] = sessionAddress;
    this->sessionsQty[username].push_back(session);

    for (auto &sessionAddr : sessionAddresses)
    {
        cout << "username: " << sessionAddr.first << " port: " << 
        sessionAddr.second.sin_port << " ip: " << 
        sessionAddr.second.sin_addr.s_addr << endl;
    }
}