#pragma once
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <android/log.h>
#include <sys/uio.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "Aixilio", __VA_ARGS__)

class Memory {
public:
    static int getPid(const char* processName) {
        FILE* file = popen((std::string("pidof ") + processName).c_str(), "r");
        int pid = -1;
        if (file) { fscanf(file, "%d", &pid); pclose(file); }
        return pid;
    }

    static uintptr_t getBase(int pid, const char* libName) {
        char path[64]; sprintf(path, "/proc/%d/maps", pid);
        FILE* fp = fopen(path, "r");
        if (!fp) return 0;
        char line[512]; uintptr_t base = 0;
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, libName)) { sscanf(line, "%lx-%*lx", &base); break; }
        }
        fclose(fp);
        return base;
    }

    static uintptr_t scan(int pid, uintptr_t start, uintptr_t end, const std::vector<int>& pattern) {
        int fd = open(("/proc/" + std::to_string(pid) + "/mem").c_str(), O_RDWR);
        if (fd == -1) return 0;
        size_t size = end - start;
        std::vector<uint8_t> buffer(size);
        pread64(fd, buffer.data(), size, start);
        for (size_t i = 0; i < size - pattern.size(); i++) {
            bool found = true;
            for (size_t j = 0; j < pattern.size(); j++) {
                if (pattern[j] != -1 && buffer[i + j] != pattern[j]) { found = false; break; }
            }
            if (found) { close(fd); return start + i; }
        }
        close(fd); return 0;
    }
};
