#pragma once

#include <vector>
#ifndef __CHANNEL_HPP__
#define __CHANNEL_HPP__

#include "../../include/IRC.h"
#include <ctime>

class Client;

namespace CHANNEL_MODE {
enum Modes {
    NO_MODE = 0,
    SET_INVITE_ONLY = 2,
    SET_TOPIC = 4,
    SET_KEY = 8,
    GIVE_PRIVILEGE = 16,
    SET_LIMIT = 32,
};
}

namespace MEMBER_PERMISSION {
enum Flags { OPERATOR = 1, REGULAR = 2 };
}

class Channel {
  public:
    Channel();
    Channel(const std::string& name, const std::string& password = "",
            CHANNEL_MODE::Modes mode = CHANNEL_MODE::NO_MODE);
    Channel(const Channel& channel);

    Channel& operator=(const Channel& channel);

    void   add(Client* newMember, MEMBER_PERMISSION::Flags premission);
    void   setMode(CHANNEL_MODE::Modes mode);
    void   unsetMode(CHANNEL_MODE::Modes mode);
    void   updateMode(CHANNEL_MODE::Modes mode); // temporary
    size_t getNumberOfMembers() const;
    void   sendToAll(Client* sender, const std::string& message);
    void   remove(const std::string& username);
    void   remove(Client* client);
    void   clear();
    bool   empty() const;
    bool   exist(Client* client);
    bool   exist(const std::string& nickname);
    const std::string& name() const;
    bool               modeIsSet(CHANNEL_MODE::Modes mode);
    bool               flagIsSet(Client* client, MEMBER_PERMISSION::Flags flag);
    Client*            getClient(const std::string& nickname);
    const std::string& getPassword() const;
    void               setPassword(const std::string& password);
    void               invite(Client* client);
    bool               isInvited(Client* client);
    void               eraseFromInviteeslist(Client* client);
    const std::string& getTopic() const;
    void               setTopic(const std::string& topic);
    void setPermission(Client* client, MEMBER_PERMISSION::Flags flag);
    const uint16_t& getLimit() const;
    void            setLimit(const uint16_t& limit);
    Client*         getTopicSetter();
    void            setTopicSetter(Client* setter);
    const time_t&   getTime() const;
    void            setTime(time_t time);
    const std::map<Client*, MEMBER_PERMISSION::Flags>& getMembers() const;

  private:
    std::string                                 _name;
    std::string                                 _password;
    std::map<Client*, MEMBER_PERMISSION::Flags> _members;
    std::vector<Client*>                        _invited;
    CHANNEL_MODE::Modes                         _mode;
    std::string                                 _topic;
    time_t                                      _timeTopic;
    Client*                                     _topicSetter;
    uint16_t                                    _limit;
};

CHANNEL_MODE::Modes operator|(CHANNEL_MODE::Modes a, CHANNEL_MODE::Modes b);
CHANNEL_MODE::Modes operator&(CHANNEL_MODE::Modes a, CHANNEL_MODE::Modes b);

#endif
