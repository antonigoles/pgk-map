#include <Engine/Support/HGT/HGT.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <bitset>
#include <thread>
#include <mutex>

#define BYTE(x) std::bitset<8>(x)

namespace Engine
{
    HGT::HGT() {
    };
    
    std::mutex hgtMutex;
    void HGT::interp_buffer_and_load(const std::vector<uint8_t>& buffer, const std::string& tile) {
        const unsigned int HGT_INT_WIDTH = 2;
        const unsigned int GRID_SIZE = 1201;
        float angularStep = 1.0f / ((float)GRID_SIZE);

        // parse tile
        float latitudeBase = std::stof(tile.substr(1, 2)) * (tile[0] == 'N' ? 1.0f : -1.0f);
        float longtitudeBase = std::stof(tile.substr(4, 3)) * (tile[3] == 'E' ? 1.0f : -1.0f);

        std::vector<HGTSample> localSamples;

        for (int i = 0; i<GRID_SIZE; i++) {
            for (int j = 0; j<GRID_SIZE; j++) {
                if (i % 2 != 0 || j % 2 != 0) continue;
                unsigned int offset = HGT_INT_WIDTH * GRID_SIZE * i + HGT_INT_WIDTH * j;
                unsigned int highByte = buffer[offset];
                unsigned int lowByte = buffer[offset+1];

                int16_t altitude = ((uint16_t)highByte << 8) | lowByte;

                localSamples.push_back(HGTSample{
                    .longitude = longtitudeBase + (1.0f - angularStep * ((float)i)),
                    .latitude = latitudeBase + angularStep * ((float)j),
                    .altitude = altitude
                });
            }
        }

        {
            std::lock_guard<std::mutex> lock(hgtMutex);
            this->sampleBuffers[tile] = std::move(localSamples);
        }

        std::cout << "Finished block " << tile << "\n";
    };

    void workerTask0(HGT* hgt, std::vector<uint8_t> buffer, std::string tile) {
        hgt->interp_buffer_and_load(buffer, tile);
    }

    HGT* HGT::fromDataSource(const std::string& path) {
        HGT* hgt = new HGT();
        std::vector<std::thread> threads;
        
        try {
            if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
                std::cerr << "Katalog nie istnieje: " << path << std::endl;
                return nullptr;
            }
            ssize_t file_count = 0;
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.is_regular_file() && entry.path().extension() == ".hgt") {
                    std::ifstream file(entry.path(), std::ios::binary | std::ios::ate);
                    auto fileSize = file.tellg();
                    file.seekg(0, std::ios::beg);
                    std::vector<uint8_t> buffer(fileSize);
                    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
                        throw std::runtime_error("Błąd odczytu: " + path);
                    }
                    // hgt->interp_buffer_and_load(buffer, entry.path().stem());
                    threads.emplace_back(workerTask0, hgt, std::move(buffer), entry.path().stem());
                    file_count++;
                    if (file_count > 1000) break;
                } 
            }

        } catch (const std::exception& e) {
            std::cerr << "Błąd ogólny: " << e.what() << std::endl;
        }

        for (auto& thread : threads) {
            if (thread.joinable()) thread.join();
        }
        return hgt;
    };

    std::mutex hgtTileMutex;
    void workerTask1(
        std::unordered_map<std::string, Engine::HGTTile *> &tiles, 
        const std::vector<Engine::HGTSample>& samples, 
        const std::string &tile
    ) {
        auto t = HGTTile::buildFrom(samples);
        {
            std::lock_guard<std::mutex> lock(hgtTileMutex);
            tiles[tile] = t;
        }
        std::cout << "Build: " << tile << "\n";
    } 

    void HGT::render_init(SceneContext *sceneContext) {
        // Prepare shaders
        this->shaderID = sceneContext->shaderRepository->shaderProgramFromDirectory("./assets/shaders/terrain-default", { 
            .isTransparent = false,
            .hasGeometryShader = false
        });

        std::vector<std::thread> threads;
        for (auto& buffer : this->sampleBuffers) {
            threads.emplace_back(workerTask1, std::ref(this->tiles), std::cref(buffer.second), buffer.first);
        }

        for (auto& thread : threads) {
            if (thread.joinable()) thread.join();
        }

        for (auto& tile : this->tiles) {
            tile.second->buildGpuBuffers();
        }
    };

    void HGT::render(RendererContext *context) {
        // bind shaders
        // ...
        context->shaderRepository->useShaderWithDataByID(this->shaderID, {}, {});
        context->shaderRepository->setUniformMat4("projection", context->projection);
        context->shaderRepository->setUniformMat4("view", context->view);
        context->shaderRepository->setUniformMat4("model", this->transform.getModelMatrix());
        context->shaderRepository->setUniformVec3("viewPos", context->cameraPosition);

        // bind and draw
        for (auto tile : this->tiles) {
            tile.second->bindAndRender();
        }
    };
};
