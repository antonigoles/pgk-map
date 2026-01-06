#include <Engine/Support/HGT/HGT.hpp>
#include <iostream>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <Engine/Core/Misc/StrLib.hpp>
#include <Engine/Core/OpenGL.hpp>
#include <thread>
#include <set>
#include <Engine/Core/Math/Math.hpp>

namespace Engine
{
    std::mutex currentMeshMutex;
    std::mutex indicesCountMutex;

    HGTTile::HGTTile(HGT* parent) {
        this->parent = parent;
        this->currentMesh = nullptr;
    };
    
    bool HGTTile::bindAndRender() {
        std::unique_lock<std::mutex> lock(currentMeshMutex);
        if (!this->currentMesh || this->currentMesh->LOD == HGTLOD::PLANE) {
            return false;
        }
        this->currentMesh->render();
        return true;
    };

    HGTLOD HGTTile::getLod() {
        std::unique_lock<std::mutex> lock(currentMeshMutex);
        return this->currentMesh ? this->currentMesh->LOD : HGTLOD::PLANE;
    };

    float LOD_MARGINS::PLANE = 1.0f;
    float LOD_MARGINS::VERY_LOW = 0.9f;
    float LOD_MARGINS::LOW = 0.6f;
    float LOD_MARGINS::MEDIUM = 0.7f;
    float LOD_MARGINS::HIGH = 0.6f;
    float LOD_MARGINS::VERY_HIGH = 0.5f;

    HGTLOD HGTTile::getLodFromPosition(glm::vec3 position) {
        if (this->isPermaPlane) return HGTLOD::PLANE;
        float distance = glm::length(position - glm::vec3(this->parent->transform.getModelMatrix() * glm::vec4(center, 1.0f)));
        float factor = glm::length(this->parent->transform.getScale());

        // we should fine tune this
        if (distance > factor * LOD_MARGINS::PLANE) return HGTLOD::PLANE;
        if (distance > factor * LOD_MARGINS::VERY_LOW) return HGTLOD::VERY_LOW;
        if (distance > factor * LOD_MARGINS::LOW) return HGTLOD::LOW;
        if (distance > factor * LOD_MARGINS::MEDIUM) return HGTLOD::MEDIUM;
        if (distance > factor * LOD_MARGINS::HIGH) return HGTLOD::HIGH;
        if (distance > factor * LOD_MARGINS::VERY_HIGH) return HGTLOD::VERY_HIGH;
        return HGTLOD::PERFECT;
    };

    void HGTTile::runGarbageCollector() {
        std::unique_lock<std::mutex> lock(currentMeshMutex);
        if (this->currentMesh && this->currentMesh->LOD == HGTLOD::PLANE) {
            // clean everything else
            this->meshQueue.clear();
            return;
        }

        if (this->meshQueue.size() >= 2) {
            int i = 0;
            std::erase_if(this->meshQueue, [](auto& x) {
                return x->LOD <= HGTLOD::MEDIUM;
            });
        }
    }

    void HGTTile::loadLODFromPlayerPositionInternal(glm::vec3 position) {
        auto LOD = this->getLodFromPosition(position);
        // std::cout << "Loaded with LOD: " << LOD << "\n";

        // TODO: Idea: maybe let's create some other way to load simple planes?
        
        std::vector<float> vertices_buffer;
        std::vector<uint32_t> indices_buffer;

        // 1. quick data load
        std::vector<uint8_t> buffer;
        if (!this->isPermaPlane) {
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            auto fileSize = file.tellg();
            file.seekg(0, std::ios::beg);
            if (fileSize > std::vector<uint8_t>().max_size()) {
                std::cout << "File too big?? " << fileSize << "\n";
                return;
            }
            buffer = std::vector<uint8_t>(fileSize);
            if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
                throw std::runtime_error("Błąd odczytu: " + path);
            }
        }

        buffer = {
            (uint8_t)0,(uint8_t)0,(uint8_t)0,(uint8_t)0,
            (uint8_t)0,(uint8_t)0,(uint8_t)0,(uint8_t)0
        };

        std::vector<std::string> out;
        split_string(path, '/', out);
        std::string tile_name = out.back().substr(0, 7);
        // std::cout << "Loading " << tile_name << "\n";

        // 2. turn buffer into a sample vector
        float latitudeBase = std::stof(tile_name.substr(1, 2)) * (tile_name[0] == 'N' ? 1.0f : -1.0f);
        float longtitudeBase = std::stof(tile_name.substr(4, 3)) * (tile_name[3] == 'E' ? 1.0f : -1.0f);

        uint32_t FILE_GRID_SIZE = this->isPermaPlane ? 2 : 1201;
        uint32_t EFFECTIVE_GRID_SIZE = this->isPermaPlane ? 2 : (1200/LOD) + 1;
        LOD = this->isPermaPlane ? HGTLOD::PERFECT : LOD;
        uint32_t HGT_INT_WIDTH = 2;

        for (int i = 0; i<FILE_GRID_SIZE; i+=LOD) {
            for (int j = 0; j<FILE_GRID_SIZE; j+=LOD) {
                unsigned int buffer_offset = HGT_INT_WIDTH * FILE_GRID_SIZE * i + HGT_INT_WIDTH * j;
                unsigned int highByte = buffer[buffer_offset];
                unsigned int lowByte = buffer[buffer_offset+1];

                int16_t altitude = (((uint16_t)highByte << 8) | lowByte);

                float latProgress = (float)i / (float)(FILE_GRID_SIZE - 1);
                float latitude = (latitudeBase + 1.0f) - (latProgress * 1.0f); 
                float lonProgress = (float)j / (float)(FILE_GRID_SIZE - 1);
                float longitude = longtitudeBase + (lonProgress * 1.0f);

                auto point = Math::angleToEarthPoint(latitude, altitude, longitude);

                vertices_buffer.insert(vertices_buffer.end(), {
                    point.x / 6371000.0f, 
                    point.y / 6371000.0f, 
                    point.z / 6371000.0f
                });

                // std::cout << point.x << " " << point.y << " " << point.z << ", ";
                if (i == 0 || j == 0) continue;
                // a - b
                // | \ |
                // c - d
                uint32_t offset_d = (vertices_buffer.size() / 3) - 1;
                uint32_t offset_a = offset_d - EFFECTIVE_GRID_SIZE - 1;
                uint32_t offset_b = offset_d - EFFECTIVE_GRID_SIZE;
                uint32_t offset_c = offset_d - 1;
                indices_buffer.insert(indices_buffer.end(), {
                    offset_a, offset_d, offset_c, offset_a, offset_b, offset_d
                });
            }
        }
        
        this->center = (glm::vec3(vertices_buffer[0], vertices_buffer[1], vertices_buffer[2]) 
            + glm::vec3(vertices_buffer[vertices_buffer.size()-3], vertices_buffer[vertices_buffer.size()-2], vertices_buffer[vertices_buffer.size()-1])) / 2.0f;

        auto mesh = HGTTileMesh::build(vertices_buffer, indices_buffer, LOD);
        {
            std::unique_lock<std::mutex> lock(currentMeshMutex);
            if (this->currentMesh != nullptr) this->meshQueue.push_back(std::move(this->currentMesh));
            this->currentMesh = std::move(mesh);
            this->currentMesh->pendingTransfer = true;
        }
    }

    void HGTTile::lodLoadTask(HGTTile *tile, glm::vec3 position) {
        tile->loadLODFromPlayerPositionInternal(position);
    }

    // this can be run from threads asynchronously
    void HGTTile::loadLODFromPlayerPosition(glm::vec3 position) {
        // check current mesh or get one from the queue
        auto LOD = this->getLodFromPosition(position);
        {
            std::unique_lock<std::mutex> lock(currentMeshMutex);
            if (this->currentMesh != nullptr && this->currentMesh->LOD == LOD) return;

            for (int i = 0; i<this->meshQueue.size(); i++) {
                if (this->meshQueue[i]->LOD == LOD) {
                    if (this->currentMesh != nullptr) this->meshQueue[i].swap(this->currentMesh);
                    else {
                        this->currentMesh = std::move(this->meshQueue[i]);
                        std::swap(this->meshQueue[i], this->meshQueue.back());
                        this->meshQueue.pop_back();
                        return;
                    }
                }
            }
        }

        auto lodJob = std::thread(lodLoadTask, this, position);
        if (lodJob.joinable()) lodJob.join();
    };

    HGTTile* HGTTile::buildFrom(const std::string& path, HGT* parent) {
        HGTTile *tile = new HGTTile(parent);
        tile->isPermaPlane = false;
        tile->path = path;
        tile->center = glm::vec3(0.0f, 0.0f, 0.0f);
        return tile;
    };

    HGTTile* HGTTile::buildPlaneAt(glm::ivec2 vec, HGT* parent) {
        HGTTile *tile = new HGTTile(parent);
        tile->isPermaPlane = true;
        tile->path = parent->ivec2_to_tile_name(vec);
        tile->center = glm::vec3(0.0f, 0.0f, 0.0f);
        return tile;
    };

    HGTTileMesh::HGTTileMesh() {
        this->pendingTransfer = false;
    };

    void HGTTileMesh::render() {
        glBindVertexArray(this->vao);
        {
            std::unique_lock<std::mutex> lock(indicesCountMutex);
            glDrawElements(GL_TRIANGLES, this->indices_count, GL_UNSIGNED_INT, (void*)0);
        }
        glBindVertexArray(0);
    };

    std::unique_ptr<HGTTileMesh> HGTTileMesh::build(
        const std::vector<float>& vertices_buffer, 
        const std::vector<uint32_t>& indices_buffer,
        HGTLOD LOD
    ) {
        std::unique_ptr<HGTTileMesh> mesh = std::make_unique<HGTTileMesh>();

        mesh->indices_buffer_cache = indices_buffer;
        mesh->vertices_buffer_cache = vertices_buffer;
        mesh->LOD = LOD;
        mesh->pendingTransfer = true;
        {
            std::unique_lock<std::mutex> lock(indicesCountMutex);
            mesh->indices_count = indices_buffer.size();
        }
        return mesh;
    };

    // this should be run synchronously
    void HGTTileMesh::transferBuffers() {
        if (!this->pendingTransfer) return;

        glGenVertexArrays(1, &this->vao);
        glGenBuffers(1, &this->vbo);
        glGenBuffers(1, &this->ebo);

        glBindVertexArray(this->vao);
        
        glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
        glBufferData(GL_ARRAY_BUFFER, this->vertices_buffer_cache.size() * sizeof(float), this->vertices_buffer_cache.data(), GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices_buffer_cache.size() * sizeof(uint32_t), this->indices_buffer_cache.data(), GL_STATIC_DRAW);
        
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        this->pendingTransfer = false;
    }

    HGTTileMesh::~HGTTileMesh() {
        if (this->vao) glDeleteVertexArrays(1, &this->vao);
        if (this->vbo) glDeleteBuffers(1, &this->vbo);
        if (this->ebo) glDeleteBuffers(1, &this->ebo);
    }

    void HGTTile::runPendingTransfers() {
        std::unique_lock<std::mutex> lock(currentMeshMutex);
        if (!this->currentMesh || !this->currentMesh->pendingTransfer) return;
        this->currentMesh->transferBuffers();
    };
};
