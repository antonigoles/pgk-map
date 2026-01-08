#include <Engine/Support/HGT/HGT.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <bitset>
#include <thread>
#include <mutex>
#include <utility>
#include <format>
#include <Engine/Core/Scene/Camera.hpp>

#define BYTE(x) std::bitset<8>(x)

namespace Engine
{
    HGT::HGT() {};

    HGT* HGT::fromDataSource(const std::string& path) {
        HGT* hgt = new HGT();        
        try {
            if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
                std::cerr << "Katalog nie istnieje: " << path << std::endl;
                return nullptr;
            }
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.is_regular_file() && entry.path().extension() == ".hgt") {
                    hgt->tiles[entry.path().stem()] = HGTTile::buildFrom(entry.path(), hgt);
                } 
            }
        } catch (const std::exception& e) {
            std::cerr << "Błąd ogólny: " << e.what() << std::endl;
        }
        std::cout << "partially-loaded " << hgt->tiles.size() << " tiles into memory\n";
        return hgt;
    };

    std::mutex isRunningMutex;
    std::unordered_map<HGTTile*, bool> isRunning;
    std::unordered_map<int, bool> isThreadRunning;

    void loaderTask(HGTTile *tile, glm::vec3 position, int threadId) {
        tile->loadLODFromPlayerPosition(position);
        {
            std::lock_guard<std::mutex> lock(isRunningMutex);
            isThreadRunning[threadId] = false;
        }
    }

    void HGT::render_init(SceneContext *sceneContext) {
        // Prepare shaders
        this->shaderID = sceneContext->shaderRepository->shaderProgramFromDirectory("./assets/shaders/terrain-default", { 
            .isTransparent = false,
            .hasGeometryShader = false
        });
    };

    int32_t getThreadIdx(HGTTile *tile) {
        static int32_t last = 0;
        static std::unordered_map<HGTTile*, int32_t> ptr;
        if (ptr.contains(tile)) return ptr[tile];
        last = (last + 1) % 1;
        ptr[tile] = last;
        return last;
    }
    
    void scheduleLodLoading(HGTTile *tile, glm::vec3 position) {
        // std::cout << "Czy moge zeschedulowac wczytywanie?\n";
        int32_t idx = getThreadIdx(tile);
        std::lock_guard<std::mutex> lock(isRunningMutex);
        if (isThreadRunning.find(idx) != isThreadRunning.end() && isThreadRunning[idx]) {
            // std::cout << "Nie moge : (\n";
            return;
        }
        isThreadRunning[idx] = true;
        // std::cout << "tak, ladujemy do tile'a: " << tile << "\n";
        std::thread(loaderTask, tile, position, idx).detach();
    }

    glm::ivec2 HGT::get_tile_on_top() {
        auto p = glm::inverse(this->transform.getRotation()) * glm::vec3(0.0f, 1.0f, 0.0f);
        return {
            (int)glm::floor(glm::degrees(std::asin(p.y))),
            (int)glm::floor(glm::degrees(std::atan2(p.z, p.x)))
        };
    }; 

    std::string HGT::ivec2_to_tile_name(const glm::ivec2& vec) {
        char ns = vec.x > 0 ? 'N' : 'S';
        char ew = vec.y > 0 ? 'E' : 'W';
        return std::format(
            "{}{}{}{}{}{}", 
            ns, 
            glm::abs(vec.x) < 10 ? "0" : "",
            glm::abs(vec.x), 
            ew,
            glm::abs(vec.y) < 100 ? glm::abs(vec.y) < 10 ? "00" : "0" : "",
            glm::abs(vec.y)
        );
    }

    void HGT::render(RendererContext *context) {
        static int rendersLeftBeforeTileSort = 1000;
        // bind shaders
        // ...
        context->shaderRepository->useShaderWithDataByID(this->shaderID, {}, {});
        context->shaderRepository->setUniformMat4("projection", context->projection);
        context->shaderRepository->setUniformMat4("view", context->view);
        context->shaderRepository->setUniformMat4("model", this->transform.getModelMatrix());
        context->shaderRepository->setUniformVec3("viewPos", context->cameraPosition);
        
        // retrieve tile from position
        glm::ivec2 topTile = this->get_tile_on_top();
        std::vector<glm::ivec2> nbrs = {};

        // generate tiles to draw
        float distance = context->cameraPosition.y - 100000.0f;
        float factor = 10000.0f;

        int square_size_x = 1 + (int)(4.0f * distance / factor);
        int square_size_y = 1 + 2 * (int)(4.0f * distance / factor);


        for (int dy = square_size_y; dy >= -square_size_y; dy--) {
            for (int dx = -square_size_x; dx <= square_size_x; dx++) {
                nbrs.push_back(topTile + glm::ivec2(dx, dy));
            }
        }

        // bind and draw
        for (int i = 0; i<nbrs.size(); i++) {
            auto nbr = nbrs[i];
            auto tileName = ivec2_to_tile_name(nbr);
            if (!this->tiles.contains(tileName)) {
                this->tiles[tileName] = HGTTile::buildPlaneAt(nbr, this);
            };
            auto tile = this->tiles[tileName];

            scheduleLodLoading(tile, context->cameraPosition);
            tile->runPendingTransfers();
            tile->runGarbageCollector();
            float lod = (float)tile->getLod();
            tile->bindAndRender();
        }
    };
};
