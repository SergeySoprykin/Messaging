#pragma once

#include "i_storage.hpp"

#include <string>
#include <vector>
#include <filesystem>

namespace simple_messaging
{

namespace fs = std::filesystem;

class messages_storage : public IStorage{

public:
    messages_storage(const std::string& tempPath = "./temp_storage", 
                   const std::string& permanentPath = "./permanent_storage");
                   
    bool save_message(const std::string& message, bool is_pending = false) override;
    bool delete_message(const std::string& filename, bool isTemporary = false) override;
    std::string read_message(const std::string& filename, bool isTemporary = false) override;
    std::vector<std::string> list_messages(bool pending = true) override;
    bool move_to_delivered(const std::string& filename) override;
    
private:
    void create_directories();
    std::string get_full_path(const std::string& filename, bool pending);
    std::string generate_filename(const std::string& destination_id, const std::string& prefix = "msg");

    std::string pending_messages_path_;
    std::string delivered_messages_path_;
};

}