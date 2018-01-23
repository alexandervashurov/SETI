#ifndef TCPCLIENT_MESSAGE_H
#define TCPCLIENT_MESSAGE_H

#include <string>
#include <sstream>
#include <algorithm>
#include <vector>

const short MESSAGE_FIELDS_COUNT = 6;


const char DELIM_PARSE = '|';
const char DELIM_SERIALIZE = '^';


const std::string MES_ID = "<id>";
const std::string MES_ADDR = "<from>";
const std::string MES_DATE_TIME = "<date/time>";
const std::string MES_LEN = "<len>";
const std::string MES_STATE = "<state>";             // message read/unread/deleted

static const std::string MESSAGE_STATES[3] = {"Normal", "Deleted", "Unread"};

const short MSTATE_NORMAL = 0;     // position of normal value
const short MSTATE_DELETED = 1;    // position of deleted value
const short MSTATE_UNREAD = 2;     // position of unread value

class Message {
public:
    unsigned long id = 0;
    std::string username;
    std::string date_time;
    unsigned long len = 0;
    int state = MSTATE_NORMAL;
    std::string body;

    std::string Serialize();

    bool Deserialize(const std::string &input);

    void Clear();

};

#endif //TCPCLIENT_MESSAGE_H
