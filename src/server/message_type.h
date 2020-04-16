#ifndef SERVER__MESSAGE_TYPE_H_
#define SERVER__MESSAGE_TYPE_H_

// IMPORTANT: convention is
// end_type = type + 1
enum class MessageType : unsigned char {
    plain_text     = 0x00,
    end_plain_text = 0x01,
    fail           = 0x0F,
};

#endif // SERVER__MESSAGE_TYPE_H_
