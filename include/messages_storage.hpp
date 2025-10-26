#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

class messages_storage {

public:
    messages_storage(const std::string& tempPath = "./temp_storage", 
                   const std::string& permanentPath = "./permanent_storage") 
        : pending_messages_path_(tempPath), delivered_messages_path_(permanentPath) {
        create_directories();
    }
    
    bool save_message(const std::string& message, bool is_pending = false) {
        try {
            std::string filename = generate_filename(is_pending ? "temp" : "perm");
            std::string full_path = get_full_path(filename, is_pending);
            
            std::ofstream file(full_path);
            if (!file.is_open()) {
                std::cerr << "Ошибка: не удалось создать файл " << full_path << std::endl;
                return false;
            }
            
            file << message;
            file.close();
            
            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "Ошибка при сохранении сообщения: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool delete_message(const std::string& filename, bool isTemporary = false) {
        try {
            std::string fullPath = get_full_path(filename, isTemporary);
            
            if (!fs::exists(fullPath)) {
                std::cerr << "Ошибка: файл " << fullPath << " не существует" << std::endl;
                return false;
            }
            
            if (fs::remove(fullPath)) {
                std::cout << "Файл " << filename << " успешно удален" << std::endl;
                return true;
            } else {
                std::cerr << "Ошибка: не удалось удалить файл " << filename << std::endl;
                return false;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Ошибка при удалении сообщения: " << e.what() << std::endl;
            return false;
        }
    }
    
    std::string read_message(const std::string& filename, bool isTemporary = false) {
        try {
            std::string fullPath = get_full_path(filename, isTemporary);
            
            if (!fs::exists(fullPath)) {
                std::cerr << "Ошибка: файл " << fullPath << " не существует" << std::endl;
                return "";
            }
            
            std::ifstream file(fullPath);
            if (!file.is_open()) {
                std::cerr << "Ошибка: не удалось открыть файл " << fullPath << std::endl;
                return "";
            }
            
            std::string content((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
            file.close();
            
            return content;
        }
        catch (const std::exception& e) {
            std::cerr << "Ошибка при чтении сообщения: " << e.what() << std::endl;
            return "";
        }
    }
    
    std::vector<std::string> list_messages(bool pending = true) {
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
            std::cerr << "Ошибка при получении списка сообщений: " << e.what() << std::endl;
        }
        
        return messages;
    }
    
    bool move_to_delivered(const std::string& filename) {
        try {
            std::string tempPath = get_full_path(filename, true);
            std::string permPath = get_full_path(filename, false);
            
            if (!fs::exists(tempPath)) {
                std::cerr << "Ошибка: файл " << filename << " не существует во временном хранилище" << std::endl;
                return false;
            }
            
            fs::rename(tempPath, permPath);
            std::cout << "Сообщение " << filename << " перемещено в постоянное хранилище" << std::endl;
            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "Ошибка при перемещении сообщения: " << e.what() << std::endl;
            return false;
        }
    }
    
private:
   
    void create_directories() {
        fs::create_directories(pending_messages_path_);
        fs::create_directories(delivered_messages_path_);
    }
    
    std::string generate_filename(const std::string& destination_id, const std::string& prefix = "msg") {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        return prefix + "_" + destination_id + "_" + std::to_string(timestamp) + ".txt";
    }
    
    std::string get_full_path(const std::string& filename, bool pending) {
        return (pending ? pending_messages_path_ : delivered_messages_path_) + "/" + filename;
    }

    std::string pending_messages_path_;
    std::string delivered_messages_path_;

};

