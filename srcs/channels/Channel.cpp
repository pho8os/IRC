#include "Channel.hpp"
#include "../client/Client.hpp"
#include <netdb.h>

Channel::Channel() : _timeTopic(0), _topicSetter(NULL), _limit(3) {}

Channel::Channel(const std::string& name, const std::string& password,
                 CHANNEL_MODE::Modes mode)
    : _name(name), _password(password), _mode(mode), _timeTopic(0),
      _topicSetter(NULL), _limit(3) {}

Channel::Channel(const Channel& channel)
    : _name(channel._name), _password(channel._password),
      _members(channel._members), _invited(channel._invited),
      _mode(channel._mode), _topic(channel._topic), _limit(channel._limit) {}

Channel& Channel::operator=(const Channel& channel) {
    if (this == &channel)
        return *this;
    _name = channel._name;
    _password = channel._password;
    _members = channel._members;
    _invited = channel._invited;
    _mode = channel._mode;
    _topic = channel._topic;
    _limit = channel._limit;
    return *this;
}

void Channel::setPermission(Client* client, MEMBER_PERMISSION::Flags flag) {
    if (_members.find(client) != _members.end())
        _members[client] = flag;
}

void Channel::add(Client* newMember, MEMBER_PERMISSION::Flags premission) {
    _members.insert(
        std::pair<Client*, MEMBER_PERMISSION::Flags>(newMember, premission));
}

void Channel::setMode(CHANNEL_MODE::Modes mode) {
    _mode = _mode | mode;
}

void Channel::unsetMode(CHANNEL_MODE::Modes mode) {
    _mode = static_cast<CHANNEL_MODE::Modes>(_mode & ~mode);
}

void Channel::updateMode(CHANNEL_MODE::Modes mode) {
    _mode = mode;
}

size_t Channel::getNumberOfMembers() const {
    return _members.size();
}

void Channel::sendToAll(Client* sender, const std::string& message) {
    std::map<Client*, MEMBER_PERMISSION::Flags>::iterator it = _members.begin();

    for (; it != _members.end(); ++it) {
        if (it->first == sender)
            continue;
        send(it->first->getSockfd(), message.c_str(), message.length(), 0);
    }
}

void Channel::remove(const std::string& nick) {
    std::map<Client*, MEMBER_PERMISSION::Flags>::iterator it = _members.begin();

    for (; it != _members.end(); ++it) {
        if (it->first->getUserInfo().getNickname() == nick) {
            _members.erase(it);
            return;
        }
    }
    throw std::runtime_error("No such client.");
}

void Channel::remove(Client* client) {
    std::map<Client*, MEMBER_PERMISSION::Flags>::iterator client_pos =
        _members.find(client);

    if (client_pos == _members.end())
        throw std::runtime_error("No such client.");
    _members.erase(client_pos);
}

void Channel::clear() {
    _members.clear();
}

bool Channel::empty() const {
    return _members.empty();
}

bool Channel::exist(Client* client) {
    return _members.find(client) != _members.end();
}

bool Channel::exist(const std::string& nickname) {
    std::map<Client*, MEMBER_PERMISSION::Flags>::const_iterator it =
        _members.begin();

    for (; it != _members.end(); ++it) {
        if (it->first->getUserInfo().getNickname() == nickname) {
            return true;
        }
    }
    return false;
}

const std::string& Channel::name() const {
    return _name;
}

bool Channel::modeIsSet(CHANNEL_MODE::Modes mode) {
    return _mode & mode;
}

bool Channel::flagIsSet(Client* client, MEMBER_PERMISSION::Flags flag) {
    std::map<Client*, MEMBER_PERMISSION::Flags>::iterator it =
        _members.find(client);

    return (it == _members.end()) ? false : it->second & flag;
}

Client* Channel::getClient(const std::string& nickname) {
    std::map<Client*, MEMBER_PERMISSION::Flags>::iterator it = _members.begin();

    for (; it != _members.end(); ++it) {
        if (it->first->getUserInfo().getNickname() == nickname)
            return it->first;
    }
    return NULL;
}

const std::string& Channel::getPassword() const {
    return _password;
}

void Channel::setPassword(const std::string& password) {
    _password = password;
}

void Channel::invite(Client* client) {
    _invited.push_back(client);
}

bool Channel::isInvited(Client* client) {
    return std::find(_invited.begin(), _invited.end(), client) !=
           _invited.end();
}

const std::string& Channel::getTopic() const {
    return _topic;
}

void Channel::setTopic(const std::string& topic) {
    _topic = topic;
}

CHANNEL_MODE::Modes operator|(CHANNEL_MODE::Modes a, CHANNEL_MODE::Modes b) {
    return static_cast<CHANNEL_MODE::Modes>(static_cast<int>(a) |
                                            static_cast<int>(b));
}

CHANNEL_MODE::Modes operator&(CHANNEL_MODE::Modes a, CHANNEL_MODE::Modes b) {
    return static_cast<CHANNEL_MODE::Modes>(static_cast<int>(a) &
                                            static_cast<int>(b));
}

void Channel::eraseFromInviteeslist(Client* client) {
    std::vector<Client*>::iterator it =
        std::find(_invited.begin(), _invited.end(), client);

    if (it == _invited.end())
        throw std::runtime_error("No such client in invited list.");
    _invited.erase(it);
}

const uint16_t& Channel::getLimit() const {
    return _limit;
}

void Channel::setLimit(const uint16_t& limit) {
    _limit = limit;
}

Client* Channel::getTopicSetter() {
    return _topicSetter;
}
void Channel::setTopicSetter(Client* setter) {
    _topicSetter = setter;
}
const time_t& Channel::getTime() const {
    return _timeTopic;
}

void Channel::setTime(time_t time) {
    _timeTopic = time;
}

const std::map<Client*, MEMBER_PERMISSION::Flags>& Channel::getMembers() const {
    return _members;
}
