#pragma once
#include <vector>
#include <string>

namespace Engine {
    class ProfilerSection {
    public:
        ProfilerSection(const std::string& name, ProfilerSection* belongsTo);
        
        std::string sectionName;
        double startTime;
        double endTime;
        ProfilerSection* belongsTo;
        int depth;
        std::vector<ProfilerSection*> subsections;
    };

    class GlobalProfiler {
    private:
        static int logFrequency;
        static ProfilerSection *lastUnclosedSection;
    public:
        GlobalProfiler() = delete;

        static void openNewSection(const std::string& name);
        static void closeLastSection();
        static void setFrequency(const int& logFrequency);
    };
}