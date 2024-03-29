#include "Message.hpp"
#include "../commands/INVITE.hpp"
#include "../commands/JOIN.hpp"
#include "../commands/KICK.hpp"
#include "../commands/MODE.hpp"
#include "../commands/NICK.hpp"
#include "../commands/NOTICE.hpp"
#include "../commands/PASS.hpp"
#include "../commands/PRIVMSG.hpp"
#include "../commands/QUIT.hpp"
#include "../commands/TOPIC.hpp"
#include "../commands/USER.hpp"

uint8_t Message::_nbrOfParams = 0;

TYPES::TokenType Message::_commandTypes[] = {
    TYPES::PASS,    TYPES::NICK,   TYPES::USER,  TYPES::JOIN,
    TYPES::KICK,    TYPES::INVITE, TYPES::TOPIC, TYPES::MODE,
    TYPES::PRIVMSG, TYPES::NOTICE, TYPES::QUIT,  TYPES::PONG,
    TYPES::INVITE,  TYPES::MODE,   TYPES::KICK,  TYPES::TOPIC};

std::string Message::_commandsStr[] = {
    "PASS",    "NICK",   "USER", "JOIN", "KICK",   "INVITE", "TOPIC", "MODE",
    "PRIVMSG", "NOTICE", "QUIT", "PONG", "INVITE", "MODE",   "KICK",  "TOPIC"};

Message::Message() : _client(NULL), _cmdfunc(NULL), _delete(false) {}

Message::~Message() {
    _reset();
}

void Message::run(Client* client) {
    if (client->getMessage().empty() || client->getMessage() == CR_LF ||
        client->getMessage() == "\n")
        return;
    _client = client;
    std::string temp(_client->getMessage());
    std::string msg;
    try {
        do {
            msg = _getMessage(temp);
            _parse(msg);
            _execute(msg);
            _reset();
            temp = temp.substr(msg.length());
        } while (!temp.empty());
    } catch (const std::exception& e) {
    }
    _client->finish();
    if (_delete) {
        Reactor::getInstance().removeClient(client);
        _delete = false;
    }
}

void Message::_parse(const std::string& message) {
    if (message.empty() || message == CR_LF)
        return;
    Parser::init(message);
    Parser::skipSpaces();
    _command();
    _params();
    _crlf();
}

void Message::_crlf() {
    if (!Parser::match(TYPES::CRLF)) {
        Reply::errUnknownCommand(_client, _cmd);
        throw std::exception();
    }
}

std::string Message::_getMessage(std::string& msg) {
    size_t crlf_pos;

    crlf_pos = msg.find(CR_LF);
    if (crlf_pos == std::string::npos) {
        size_t lf_pos = msg.find("\n");
        msg.insert(lf_pos, "\r");
        return msg.substr(0, lf_pos + 2);
    }
    return msg.substr(0, crlf_pos + 2);
}

void Message::_command() {
    if (!_isCommand()) {
        Reply::errUnknownCommand(_client, Parser::peek().lexeme());
        throw std::exception();
    }
    _cmd = Parser::advance().lexeme();
}

void Message::_params() {
    if (Parser::check(TYPES::CRLF) || _nbrOfParams >= MAX_PARAMS)
        return;
    if (!Parser::skipSpaces()) {
        Reply::errUnknownCommand(_client, _cmd);
        throw std::exception();
    }
    ++_nbrOfParams;
    if (Parser::match(TYPES::COLON)) {
        _parameters.push_back(Parser::previous().lexeme());
        _trailing();
    } else if (_nospcrlfcl()) {
        _middle();
        _params();
    }
}

void Message::_trailing() {
    if (Parser::isAtEnd() || Parser::check(TYPES::CRLF))
        return;
    std::string param;
    do {
        param.append(Parser::advance().lexeme());
    } while (_nospcrlfcl() || Parser::check(TYPES::SPACE) ||
             Parser::check(TYPES::COLON));
    _parameters.push_back(param);
}

void Message::_middle() {
    std::string param;
    while (_nospcrlfcl() || Parser::check(TYPES::COLON))
        param.append(Parser::advance().lexeme());
    _parameters.push_back(param);
}

bool Message::_nospcrlfcl() {
    switch (Parser::peek().type()) {
    case TYPES::CR:
    case TYPES::LF:
    case TYPES::SPACE:
    case TYPES::COLON:
    case TYPES::CRLF:
        return false;
    default:
        break;
    }
    return true;
}

bool Message::_isCommand() {
    switch (Parser::peek().type()) {
    case TYPES::PASS:
    case TYPES::NICK:
    case TYPES::USER:
    case TYPES::JOIN:
    case TYPES::KICK:
    case TYPES::INVITE:
    case TYPES::TOPIC:
    case TYPES::MODE:
    case TYPES::PRIVMSG:
    case TYPES::NOTICE:
    case TYPES::QUIT:
    case TYPES::PONG:
        return true;
    default:
        break;
    }
    return false;
}

void Message::_execute(const std::string& msg) {
    if (msg.empty() || msg == CR_LF)
        return;
    switch (_whichCommand()) {
    case TYPES::PASS:
        _cmdfunc = new PASS();
        break;
    case TYPES::NICK:
        _cmdfunc = new NICK();
        break;
    case TYPES::USER:
        _cmdfunc = new USER();
        break;
    case TYPES::JOIN:
        _cmdfunc = new JOIN();
        break;
    case TYPES::PRIVMSG:
        _cmdfunc = new PRIVMSG();
        break;
    case TYPES::NOTICE:
        _cmdfunc = new NOTICE();
        break;
    case TYPES::QUIT:
        _delete = true;
        _cmdfunc = new QUIT();
        break;
    case TYPES::INVITE:
        _cmdfunc = new INVITE();
        break;
    case TYPES::MODE:
        _cmdfunc = new MODE();
        break;
    case TYPES::KICK:
        _cmdfunc = new KICK();
        break;
    case TYPES::TOPIC:
        _cmdfunc = new TOPIC();
        break;
    case TYPES::PONG:
        return;
    default:
        break;
    }
    try {
        _cmdfunc->execute(_client, _parameters);
    } catch (...) {
    }
}

TYPES::TokenType Message::_whichCommand() {
    for (int i = 0; i < CMDS_NBR; ++i) {
        if (_cmd == _commandsStr[i])
            return _commandTypes[i];
    }
    return TYPES::END;
}

void Message::_reset() {
    _nbrOfParams = 0;
    _parameters.clear();
    delete _cmdfunc;
    _cmdfunc = NULL;
}
