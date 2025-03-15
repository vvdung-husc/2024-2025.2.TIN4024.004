#include <iostream>
#include <fstream>
#include <cpr/cpr.h>
#include <json/json.h>
#include <set>

const std::string BOT_TOKEN = "7958454010:AAGYkVfTHc-FgAg-YqOMgIo5CR-DRgvMB-I";
const std::string UPDATE_URL = "https://api.telegram.org/bot" + BOT_TOKEN + "/getUpdates";
const std::string SEND_MSG_URL = "https://api.telegram.org/bot" + BOT_TOKEN + "/sendMessage";
std::set<std::string> chat_ids; // Lưu danh sách chat_id

// Hàm gửi tin nhắn đến một user
void sendMessage(const std::string& chat_id, const std::string& message) {
    cpr::Response r = cpr::Post(
        cpr::Url{SEND_MSG_URL},
        cpr::Payload{{"chat_id", chat_id}, {"text", message}}
    );

    if (r.status_code == 200) {
        std::cout << "Đã gửi tin nhắn đến " << chat_id << "\n";
    } else {
        std::cout << "Lỗi khi gửi tin nhắn: " << r.text << "\n";
    }
}

// Hàm lấy tin nhắn mới và lưu chat_id
void getUpdates() {
    cpr::Response r = cpr::Get(cpr::Url{UPDATE_URL});
    if (r.status_code != 200) {
        std::cout << "Lỗi khi gọi API: " << r.text << "\n";
        return;
    }

    Json::Value jsonData;
    Json::CharReaderBuilder reader;
    std::string errs;
    std::istringstream ss(r.text);
    if (!Json::parseFromStream(reader, ss, &jsonData, &errs)) {
        std::cout << "Lỗi parse JSON: " << errs << "\n";
        return;
    }

    // Duyệt danh sách tin nhắn mới
    for (const auto& update : jsonData["result"]) {
        std::string chat_id = update["message"]["chat"]["id"].asString();
        std::string message = update["message"]["text"].asString();

        if (chat_ids.find(chat_id) == chat_ids.end()) {
            chat_ids.insert(chat_id);
            std::cout << "Đã phát hiện người dùng mới: " << chat_id << "\n";
            sendMessage(chat_id, "Chào mừng bạn đến với bot của tôi!");
        }
    }
}

int main() {
    while (true) {
        getUpdates();
        std::this_thread::sleep_for(std::chrono::seconds(5)); // Chạy mỗi 5 giây
    }
    return 0;
}