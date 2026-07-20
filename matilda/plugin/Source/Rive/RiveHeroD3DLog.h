#pragma once

// Release-safe diagnostics for the Windows Rive hero. Appends to
// %TEMP%\MatildaRiveD3D.log so failures in shipped builds can be inspected.
// Plain C++ only — RiveHeroD3DCore.cpp must stay free of JUCE headers.

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <mutex>
#include <string>

namespace matilda::rive {

inline void d3dLog(const std::string& message) {
    static std::mutex mutex;
    const std::lock_guard<std::mutex> lock(mutex);

    const char* temp = std::getenv("TEMP");
    if (temp == nullptr)
        temp = std::getenv("TMP");
    if (temp == nullptr)
        return;

    const std::string path = std::string(temp) + "\\MatildaRiveD3D.log";
    if (std::FILE* f = std::fopen(path.c_str(), "a")) {
        std::fprintf(f, "[%lld] %s\n", static_cast<long long>(std::time(nullptr)), message.c_str());
        std::fclose(f);
    }
}

inline void d3dLogHr(const char* what, long hr) {
    char buf[192];
    std::snprintf(buf, sizeof(buf), "%s hr=0x%08lX", what, static_cast<unsigned long>(hr));
    d3dLog(buf);
}

} // namespace matilda::rive
