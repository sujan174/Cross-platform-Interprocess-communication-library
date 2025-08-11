#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include "cross_ipc.hpp"
#include "shared_memory.hpp"


namespace simple_json {
    class value {
    public:
        enum class type { null, object, array, string, number, boolean };
        
        value() : type_(type::null) {}
        value(const std::vector<value>& arr) : type_(type::array), array_value_(arr) {}
        value(const std::vector<std::pair<std::string, value>>& obj) : type_(type::object), object_value_(obj) {}
        value(const std::string& s) : type_(type::string), string_value_(s) {}
        value(int n) : type_(type::number), number_value_(n) {}
        value(bool b) : type_(type::boolean), boolean_value_(b) {}
        
        type get_type() const { return type_; }
        bool is_object() const { return type_ == type::object; }
        bool is_array() const { return type_ == type::array; }
        
        value& operator[](const std::string& key) {
            for (auto& pair : object_value_) {
                if (pair.first == key) return pair.second;
            }
            object_value_.push_back({key, value()});
            return object_value_.back().second;
        }
        
        value& operator[](size_t index) {
            if (index >= array_value_.size()) {
                array_value_.resize(index + 1);
            }
            return array_value_[index];
        }
        
        bool contains(const std::string& key) const {
            for (const auto& pair : object_value_) {
                if (pair.first == key) return true;
            }
            return false;
        }
        
        size_t size() const { return array_value_.size(); }
        
        void push_back(const value& val) {
            if (type_ != type::array) {
                type_ = type::array;
                array_value_.clear();
            }
            array_value_.push_back(val);
        }
        
        std::string dump() const {
            switch (type_) {
                case type::null: return "null";
                case type::boolean: return boolean_value_ ? "true" : "false";
                case type::number: return std::to_string(number_value_);
                case type::string: return "\"" + string_value_ + "\"";
                case type::array: {
                    std::string result = "[";
                    for (size_t i = 0; i < array_value_.size(); ++i) {
                        if (i > 0) result += ",";
                        result += array_value_[i].dump();
                    }
                    result += "]";
                    return result;
                }
                case type::object: {
                    std::string result = "{";
                    for (size_t i = 0; i < object_value_.size(); ++i) {
                        if (i > 0) result += ",";
                        result += "\"" + object_value_[i].first + "\":" + object_value_[i].second.dump();
                    }
                    result += "}";
                    return result;
                }
            }
            return "null";
        }
        
        static value parse(const std::string& json) {
            size_t pos = 0;
            return parse_value(json, pos);
        }
        
    private:
        type type_;
        std::vector<std::pair<std::string, value>> object_value_;
        std::vector<value> array_value_;
        std::string string_value_;
        int number_value_;
        bool boolean_value_;
        
        static value parse_value(const std::string& json, size_t& pos) {
            
            while (pos < json.size() && std::isspace(json[pos])) pos++;
            
            if (pos >= json.size()) return value();
            
            char c = json[pos];
            if (c == '{') {
                return parse_object(json, pos);
            } else if (c == '[') {
                return parse_array(json, pos);
            } else if (c == '"') {
                return parse_string(json, pos);
            } else if (std::isdigit(c) || c == '-') {
                return parse_number(json, pos);
            } else if (c == 't' || c == 'f') {
                return parse_boolean(json, pos);
            } else if (c == 'n') {
                
                if (pos + 4 <= json.size() && json.substr(pos, 4) == "null") {
                    pos += 4;
                    return value();
                }
            }
            
            // Default to null for invalid input
            return value();
        }
        
        static value parse_object(const std::string& json, size_t& pos) {
            std::vector<std::pair<std::string, value>> obj;
            pos++; 
            
            
            while (pos < json.size() && std::isspace(json[pos])) pos++;
            
            if (pos < json.size() && json[pos] == '}') {
                pos++;
                return value(obj);
            }
            
            while (pos < json.size()) {
                // Parse key
                if (json[pos] != '"') break;
                std::string key = parse_string(json, pos).string_value_;
                
                
                while (pos < json.size() && (std::isspace(json[pos]) || json[pos] == ':')) pos++;
                
                
                value val = parse_value(json, pos);
                obj.push_back({key, val});
                
                
                while (pos < json.size() && (std::isspace(json[pos]) || json[pos] == ',')) pos++;
                
                if (pos < json.size() && json[pos] == '}') {
                    pos++;
                    break;
                }
            }
            
            return value(obj);
        }
        
        static value parse_array(const std::string& json, size_t& pos) {
            std::vector<value> arr;
            pos++; 
            
            
            while (pos < json.size() && std::isspace(json[pos])) pos++;
            
            if (pos < json.size() && json[pos] == ']') {
                pos++;
                return value(arr);
            }
            
            while (pos < json.size()) {
                
                value val = parse_value(json, pos);
                arr.push_back(val);
                
                
                while (pos < json.size() && (std::isspace(json[pos]) || json[pos] == ',')) pos++;
                
                if (pos < json.size() && json[pos] == ']') {
                    pos++;
                    break;
                }
            }
            
            return value(arr);
        }
        
        static value parse_string(const std::string& json, size_t& pos) {
            std::string str;
            pos++; 
            
            while (pos < json.size() && json[pos] != '"') {
                str += json[pos++];
            }
            
            if (pos < json.size()) pos++; 
            
            return value(str);
        }
        
        static value parse_number(const std::string& json, size_t& pos) {
            std::string num_str;
            
            while (pos < json.size() && (std::isdigit(json[pos]) || json[pos] == '-' || json[pos] == '.')) {
                num_str += json[pos++];
            }
            
            return value(std::stoi(num_str));
        }
        
        static value parse_boolean(const std::string& json, size_t& pos) {
            if (pos + 4 <= json.size() && json.substr(pos, 4) == "true") {
                pos += 4;
                return value(true);
            } else if (pos + 5 <= json.size() && json.substr(pos, 5) == "false") {
                pos += 5;
                return value(false);
            }
            
            return value(false);
        }
    };
}

// JSON structure for the shared data with predefined values
const simple_json::value DEFAULT_JSON = []() {
    simple_json::value json;
    
    
    simple_json::value counters;
    
    // Queue 1
    simple_json::value queue1;
    queue1.push_back(42);
    queue1.push_back(87);
    queue1.push_back(123);
    
    // Queue 2
    simple_json::value queue2;
    queue2.push_back(15);
    queue2.push_back(33);
    queue2.push_back(55);
    queue2.push_back(77);
    
    // Queue 3
    simple_json::value queue3;
    queue3.push_back(101);
    queue3.push_back(202);
    
    // Queue 4
    simple_json::value queue4;
    queue4.push_back(64);
    queue4.push_back(128);
    queue4.push_back(192);
    queue4.push_back(255);
    
    
    simple_json::value queue5;
    queue5.push_back(7);
    queue5.push_back(14);
    queue5.push_back(21);
    queue5.push_back(28);
    queue5.push_back(35);
    
    
    counters.push_back(queue1);
    counters.push_back(queue2);
    counters.push_back(queue3);
    counters.push_back(queue4);
    counters.push_back(queue5);
    
    // Add counters to json
    json["counters"] = counters;
    json["max_counter"] = 20;
    
    return json;
}();

// Constants
const double POLL_INTERVAL = 0.0185;  // seconds between increments
const int MAX_QUEUE_SIZE = 20;       // maximum items per queue
const unsigned long LOCK_TIMEOUT_MS = 2000;  

int main() {
    try {
        
        cross_ipc::SetDLLPath("cross-ipc.dll");
        
        std::cout << "Counter Incrementer (Producer)" << std::endl;
        std::cout << "This program adds random numbers to random queues in shared memory." << std::endl;
        
        
        cross_ipc::SharedMemory shm("CounterSync", 4096, true);
        
        if (!shm.Setup()) {
            std::cerr << "Failed to set up shared memory" << std::endl;
            return 1;
        }
        
        
        simple_json::value data;
        try {
            std::string data_str = shm.Read();
            if (!data_str.empty()) {
                try {
                    data = simple_json::value::parse(data_str);
                    // Verify data is an object
                    if (!data.is_object()) {
                        std::cout << "Invalid data format (not an object): " << data_str << std::endl;
                        data = DEFAULT_JSON;
                        std::cout << "Initializing with predefined values." << std::endl;
                    }
                } catch (...) {
                    std::cout << "Invalid JSON data: " << data_str << std::endl;
                    data = DEFAULT_JSON;
                    std::cout << "Initializing with predefined values." << std::endl;
                }
            } else {
                data = DEFAULT_JSON;
                std::cout << "Initializing with predefined values." << std::endl;
            }
            
            
            if (!shm.WriteWithLock(data.dump(), LOCK_TIMEOUT_MS)) {
                std::cerr << "Failed to acquire lock for initial write" << std::endl;
                return 1;
            }
        } catch (const std::exception& e) {
            std::cout << "Error initializing data: " << e.what() << std::endl;
            data = DEFAULT_JSON;
            std::cout << "Initializing with predefined values." << std::endl;
            
            
            if (!shm.WriteWithLock(data.dump(), LOCK_TIMEOUT_MS)) {
                std::cerr << "Failed to acquire lock for initial write" << std::endl;
                return 1;
            }
        }
        
        // Print initial state of queues
        std::cout << "\nInitial queue state:" << std::endl;
        for (size_t i = 0; i < data["counters"].size(); ++i) {
            std::cout << "Queue " << (i+1) << ": ";
            for (size_t j = 0; j < data["counters"][i].size(); ++j) {
                std::cout << data["counters"][i][j].dump() << " ";
            }
            std::cout << "(" << data["counters"][i].size() << " items)" << std::endl;
        }
        
        std::cout << "\nConnected to shared memory. Current data structure initialized." << std::endl;
        std::cout << "\n=================================================" << std::endl;
        std::cout << "PRODUCER RUNNING" << std::endl;
        std::cout << "Adding items every " << POLL_INTERVAL << " seconds (max per queue: " << MAX_QUEUE_SIZE << ")" << std::endl;
        std::cout << "Press Ctrl+C to stop" << std::endl;
        std::cout << "=================================================\n" << std::endl;
        
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> queue_dist(0, 4);  
        std::uniform_int_distribution<> value_dist(0, 256);
        
        
        while (true) {
            try {
                
                std::string data_str = shm.Read();
                if (data_str.empty()) {
                    data_str = "{}";
                }
                
                try {
                    data = simple_json::value::parse(data_str);
                    
                    if (!data.is_object()) {
                        std::cout << "Invalid data format (not an object): " << data_str << std::endl;
                        data = DEFAULT_JSON;
                    }
                } catch (...) {
                    std::cout << "Invalid JSON data: " << data_str << std::endl;
                    data = DEFAULT_JSON;
                }
                
                
                if (!data.contains("counters") || !data["counters"].is_array()) {
                    std::cout << "Invalid data structure, reinitializing..." << std::endl;
                    data = DEFAULT_JSON;
                }
                
                // Choose a random queue
                int queue_index = queue_dist(gen);
                
                // Make sure the selected queue is an array
                if (!data["counters"][queue_index].is_array()) {
                    data["counters"][queue_index] = simple_json::value(std::vector<simple_json::value>());
                }
                
                auto& selected_queue = data["counters"][queue_index];
                
                
                if (selected_queue.size() < MAX_QUEUE_SIZE) {
                    
                    int new_value = value_dist(gen);
                    
                    // Add the number to the queue
                    selected_queue.push_back(new_value);
                    
                    
                    if (shm.WriteWithLock(data.dump(), LOCK_TIMEOUT_MS)) {
                        std::cout << "Added " << new_value << " to queue " << (queue_index+1) 
                                  << ". Queue now has " << selected_queue.size() << " items." << std::endl;
                    } else {
                        auto now = std::chrono::system_clock::now();
                        auto time = std::chrono::system_clock::to_time_t(now);
                        char time_str[9];
                        std::strftime(time_str, sizeof(time_str), "%H:%M:%S", std::localtime(&time));
                        std::cout << "[LOCK DETECTED] Failed to acquire lock, add operation skipped - " 
                                  << time_str << std::endl;
                    }
                } else {
                    // Queue is full, silently skip
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                
                data = DEFAULT_JSON;
                if (shm.WriteWithLock(data.dump(), LOCK_TIMEOUT_MS)) {
                    std::cout << "Reset data structure after error" << std::endl;
                } else {
                    auto now = std::chrono::system_clock::now();
                    auto time = std::chrono::system_clock::to_time_t(now);
                    char time_str[9];
                    std::strftime(time_str, sizeof(time_str), "%H:%M:%S", std::localtime(&time));
                    std::cout << "[LOCK DETECTED] Failed to acquire lock for error recovery - " 
                              << time_str << std::endl;
                }
            }
            
            
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(POLL_INTERVAL * 1000)));
        }
    }
    catch (const cross_ipc::CrossIPCError& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}