#include "Memory.h"
#include <thread>
#include <fstream>
#include <sys/uio.h>

int ff_pid = 0;
bool isNeckToHead = false;
bool isAimbot = false;

void FileListener() {
    while (true) {
        std::ifstream cmd_file("/data/local/tmp/aixilio_cmd.txt");
        if (cmd_file.is_open()) {
            std::string cmd;
            std::getline(cmd_file, cmd);
            cmd_file.close();
            if (cmd == "NECK_HEAD_ON") { isNeckToHead = true; }
            else if (cmd == "AIMBOT_ON") { isAimbot = true; }
            else if (cmd == "OFF") { isNeckToHead = false; isAimbot = false; }
        }
        usleep(500000);
    }
}

void HeadshotLogic() {
    int fd = open(("/proc/" + std::to_string(ff_pid) + "/mem").c_str(), O_RDWR);
    if (fd == -1) return;

    // ADDRESSES: This will scan libil2cpp.so for the headshot patch
    uintptr_t base = Memory::getBase(ff_pid, "libil2cpp.so");
    uintptr_t hitbox_offset = 0;
    
    // FILL WITH REAL PATTERN LATER
    // std::vector<int> hitboxPattern = {0xFF, 0x03, 0x00, 0xD1};
    // hitbox_offset = Memory::scan(ff_pid, base, base + 0x3000000, hitboxPattern);

    while (true) {
        if (isNeckToHead && hitbox_offset) {
            int head = 0; // 0 = Head
            pwrite64(fd, &head, sizeof(int), hitbox_offset);
        }
        usleep(2000);
    }
    close(fd);
}

int main() {
    LOGI("Aixilio Started. Waiting for Free Fire...");
    while (!(ff_pid = Memory::getPid("com.dts.freefireth"))) { sleep(1); }
    LOGI("Free Fire PID: %d", ff_pid);

    std::thread(FileListener).detach();
    std::thread(HeadshotLogic).detach();

    while (true) { sleep(10); }
    return 0;
}
