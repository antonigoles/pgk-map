#include <Engine/Dev/GlobalProfiler.hpp>
#include <Engine/Core/OpenGL.hpp>
#include <vector>
#include <string>
#include <iostream>

namespace Engine {
    int GlobalProfiler::logFrequency = 1;
    ProfilerSection* GlobalProfiler::lastUnclosedSection = nullptr;

    ProfilerSection::ProfilerSection(const std::string& name, ProfilerSection* belongsTo) {
        this->sectionName = name;
        this->belongsTo = belongsTo;
        this->startTime = glfwGetTime();
        this->endTime = this->startTime; 
        this->depth = 0;
    };

    void GlobalProfiler::openNewSection(const std::string& name) {
        auto section = new ProfilerSection(name, GlobalProfiler::lastUnclosedSection);
        
        if (GlobalProfiler::lastUnclosedSection != nullptr) 
            section->depth = GlobalProfiler::lastUnclosedSection->depth+1;
        
        GlobalProfiler::lastUnclosedSection = section;
    };
    
    void GlobalProfiler::closeLastSection() {
        static int printedBlocks = 0;

        auto section = GlobalProfiler::lastUnclosedSection;
        section->endTime = glfwGetTime();

        if (printedBlocks % GlobalProfiler::logFrequency == 0) {
            for (int i = 0; i<section->depth; i++) std::cout << "--";
            std::cout << " " << section->sectionName << ": ";
            std::cout << (section->endTime - section->startTime) * 1000.0 << " ms\n";
        }
        

        GlobalProfiler::lastUnclosedSection = section->belongsTo;
        if (GlobalProfiler::lastUnclosedSection == nullptr) printedBlocks++;
        delete section;
    };

    void GlobalProfiler::setFrequency(const int& logFrequency) {
        GlobalProfiler::logFrequency = logFrequency;
    };
}