#include <map>
#include <iostream>
#include <string>
#include <unistd.h>
#include <functional>
#include <sstream>

using std::cout;
using std::string;
using std::map;
using std::pair;
using std::function;

std::ostringstream ss;

bool daemon_running = false;
map<string, function<void(string)>> callback_map;

extern "C" int register_callback(string command_msg, function<void(string)>func) {
    cout << "register_callback for: " << command_msg << '\n';
    callback_map.insert(pair<string, function<void(string)>>(command_msg, func) );
    return 0;
}

extern "C" int stop_daemon() {
    cout << "stopping daemon \n";
    daemon_running = false;
    return 0;
}

extern "C" unsigned int start_daemon(unsigned int trigger_interation) {
    cout << "starting daemon for " << trigger_interation << " interactions\n";
    daemon_running = true;
    unsigned int int_count = 0;
    while (daemon_running) {
        if (int_count == trigger_interation) {
            cout << "reached interaction: " << int_count << "\n";
            cout << "--------------------------------\n";
            ss << "message_from_daemon...reboot_device at " << int_count;
            callback_map.find("reboot_device")->second(ss.str());
            cout << "--------------------------------\n";
        }
        int_count++;
        usleep(10);
    }
    return int_count;
}