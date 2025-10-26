#include "messages_storage.hpp"

#include <iostream>
#include <fstream>

namespace simple_messaging
{

messages_storage::messages_storage(const std::string& tempPath, const std::string& permanentPath) 
    : pending_messages_path_(tempPath), delivered_messages_path_(permanentPath) {
    create_directories();
}

bool messages_storage::save_message(const std::string& message, bool is_pending) {
    try {
        std::string filename = generate_filename(is_pending ? "temp" : "perm");
        std::string full_path = get_full_path(filename, is_pending);
        
        std::ofstream file(full_path);
        if (!file.is_open()) {
            std::cerr << "Unable to create file " << full_path << std::endl;
            return false;
        }
        
        file << message;
        file.close();
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving message: " << e.what() << std::endl;
        return false;
    }
}

bool messages_storage::delete_message(const std::string& filename, bool isTemporary) {
    try {
        std::string fullPath = get_full_path(filename, isTemporary);
        
        if (!fs::exists(fullPath)) {
            std::cerr << "Error: file " << fullPath << " does not exist" << std::endl;
            return false;
        }
        
        if (fs::remove(fullPath)) {
            return true;
        } else {
            std::cerr << "Error: unable remove file " << filename << std::endl;
            return false;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error deleting message: " << e.what() << std::endl;
        return false;
    }
}

std::string messages_storage::read_message(const std::string& filename, bool isTemporary) {
    try {
        std::string fullPath = get_full_path(filename, isTemporary);
        
        if (!fs::exists(fullPath)) {
            std::cerr << "Error: file " << fullPath << " does not exist" << std::endl;
            return "";
        }
        
        std::ifstream file(fullPath);
        if (!file.is_open()) {
            std::cerr << "Error: unable to open file " << fullPath << std::endl;
            return "";
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        file.close();
        
        return content;
    }
    catch (const std::exception& e) {
        std::cerr << "Error reading message: " << e.what() << std::endl;
        return "";
    }
}

std::vector<std::string> messages_storage::list_messages(bool pending) {
    std::vector<std::string> messages;
    try {
        const std::string& storagePath = pending ? pending_messages_path_ : delivered_messages_path_;
        for (const auto& entry : fs::directory_iterator(storagePath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                messages.push_back(entry.path().filename().string());
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Messages list error: " << e.what() << std::endl;
    }
    
    return messages;
}

bool messages_storage::move_to_delivered(const std::string& filename) {
    try {
        std::string tempPath = get_full_path(filename, true);
        std::string permPath = get_full_path(filename, false);
        
        if (!fs::exists(tempPath)) {
            std::cerr << "Error: file " << filename << " does not exist" << std::endl;
            return false;
        }
        
        fs::rename(tempPath, permPath);
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error moving message: " << e.what() << std::endl;
        return false;
    }
}


void messages_storage::create_directories() {
    fs::create_directories(pending_messages_path_);
    fs::create_directories(delivered_messages_path_);
}

std::string messages_storage::generate_filename(const std::string& destination_id, const std::string& prefix) {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    return prefix + "_" + destination_id + "_" + std::to_string(timestamp) + ".txt";
}

std::string messages_storage::get_full_path(const std::string& filename, bool pending) {
    return (pending ? pending_messages_path_ : delivered_messages_path_) + "/" + filename;
}

}