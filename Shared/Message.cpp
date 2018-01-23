#include "Message.h"


std::string Message::Serialize() {
    std::stringstream ss;
    ss << id << DELIM_SERIALIZE;
    ss << username << DELIM_SERIALIZE;
    ss << date_time << DELIM_SERIALIZE;
    ss << len << DELIM_SERIALIZE;
    ss << state << DELIM_SERIALIZE;
    std::replace(body.begin(), body.end(), DELIM_SERIALIZE, ' ');
    ss << body << DELIM_SERIALIZE;
    return ss.str();
}

void Message::Clear() {
    id = 0;
    username = "";
    date_time = "";
    len = 0;
    state = MSTATE_NORMAL;
    body = "";
};


bool Message::Deserialize(const std::string &input) {
    std::vector<std::string> args;
    size_t pos = 0;
    std::string token;
    std::string str = input;
    while ((pos = str.find(DELIM_SERIALIZE)) != std::string::npos) {
        token = str.substr(0, pos);
        args.emplace_back(token);
        str = str.substr(pos + sizeof(DELIM_SERIALIZE));
    }
    if (args.size() != MESSAGE_FIELDS_COUNT) return false;
    id = std::stoul(args[0]);
    username = args[1];
    date_time = args[2];
    len = std::stoul(args[3]);
    state = std::stoi(args[4]);
    body = args[5];
    return true;
};
