#include <iostream>
#include <dlfcn.h>
#include <string>
#include "pthread.h"
#include <unistd.h>
#include <functional>

using std::cerr;
using std::cout;
using std::string;
using std::invalid_argument;
using std::runtime_error;
using std::function;

typedef int (*register_callback_t)(string, function<void(string)>);
typedef unsigned int (*start_daemon_t)(int);
typedef int (*stop_daemon_t)(void);

register_callback_t register_callback = NULL;
start_daemon_t start_daemon = NULL;
stop_daemon_t stop_daemon = NULL;

void reboot_callback(string payload) {
     cout << "Rebooting the device with message [" << payload << "]\n";
}

void* load_symbol(void *handle, const char *symbol_name) {
     void* sym = dlsym(handle, symbol_name);
     char *dlsym_error = dlerror();
     if (!dlsym_error) {
          return sym;
     }
     dlclose(handle);
     cerr << "Failed to open symbol " << dlsym_error << "\n";
     throw invalid_argument((string) dlsym_error);
}

void *start_daemon_thread(void *arg) {
     cout << "Starting daemon thread.\n";
     unsigned int interactions = start_daemon(50000);
     cout << "Stopped daemon after " << interactions << " interactions.\n";
     return NULL;
}

int main(int argc, char** argv) {
     if (argc < 2) {
        throw runtime_error("Missing lib_object argument.   ");
     }

     string command_msg = "reboot_device";
     string shared_object = argv[1];

     void *handle = dlopen(shared_object.c_str(), RTLD_LAZY);
     if (!handle) {
          cerr << "Cannot open library: " << dlerror() << '\n';
          return 1;
     }
     dlerror();

     register_callback = (register_callback_t) load_symbol(handle, "register_callback");
     start_daemon = (start_daemon_t) load_symbol(handle, "start_daemon");
     stop_daemon = (stop_daemon_t) load_symbol(handle, "stop_daemon");

     int callback_resp = register_callback(command_msg, &reboot_callback);

     pthread_t thread_id;
     int err = pthread_create(&thread_id, NULL, &start_daemon_thread, NULL);
     if (err) {
          cerr << "Failed to create thread " << err << "\n";
          throw invalid_argument("Failed to create thread");
     }

     cout << "Waiting to stop the daemon.\n";
     sleep(10);
     stop_daemon();

     err = pthread_join(thread_id, NULL);
     if (err) {
          cerr << "Failed to join thread " << err << "\n";
     }

     dlclose(handle);
}

