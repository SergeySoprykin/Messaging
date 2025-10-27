#pragma once

#include <string>
#include <vector>

class IStorage {
public:
    virtual bool save_message(const std::string& message, bool is_pending = false) = 0;
    virtual std::string read_message(const std::string& filename, bool isTemporary = false) = 0;
    virtual std::vector<std::string> list_messages(bool pending = true) = 0;
    virtual bool move_to_delivered(const std::string& filename) = 0;
};