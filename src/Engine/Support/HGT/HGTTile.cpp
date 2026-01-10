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
        this->isPermaPlane = false;
    };
    
    bool HGTTile::bindAndRender() {
        std::unique_lock<std::mutex> lock(currentMeshMutex);
        if (!this->currentMesh) {
            return false;
        }
        this->currentMesh->render();
        return true;
    };

    int HGTTile::getLod() {
        std::unique_lock<std::mutex> lock(currentMeshMutex);
        return this->currentMesh ? this->currentMesh->LOD : HGTLOD::PLANE;
    };

    float LOD_MARGINS::PLANE = 10.6f;
    float LOD_MARGINS::VERY_LOW = 5.5f;
    float LOD_MARGINS::LOW = 2.4f;
    float LOD_MARGINS::MEDIUM = 1.3f;
    float LOD_MARGINS::HIGH = 0.2f;
    float LOD_MARGINS::VERY_HIGH = 0.1f;

    // diviros of 1200
    std::vector<int> divisorsOf1200 = {
        1, 2, 3, 4, 5, 6, 8, 10, 12, 15, 16, 20, 24, 25, 30, 40, 48, 50, 60, 75, 80, 100, 120, 150, 200,
    };

    int HGTTile::getLodFromPosition(glm::vec3 position) {
        if (this->isPermaPlane) return HGTLOD::PLANE;
        float distance = position.y - 100000.0f;
        float factor = 10000.0f;

        
        // if (distance > factor * LOD_MARGINS::PLANE) return HGTLOD::PLANE;
        // if (distance > factor * LOD_MARGINS::VERY_LOW) return HGTLOD::VERY_LOW;
        // if (distance > factor * LOD_MARGINS::LOW) return HGTLOD::LOW;
        // if (distance > factor * LOD_MARGINS::MEDIUM) return HGTLOD::MEDIUM;
        // if (distance > factor * LOD_MARGINS::HIGH) return HGTLOD::HIGH;
        // if (distance > factor * LOD_MARGINS::VERY_HIGH) return HGTLOD::VERY_HIGH;
        // return HGTLOD::VERY_LOW;

        // continoous LOD
        int max = divisorsOf1200.size() - 1;
        int LOD = std::min(max, std::max(0, (int)(12.0f * distance / factor)));

        return divisorsOf1200[LOD];
    };

    void HGTTile::runGarbageCollector() {
        std::unique_lock<std::mutex> lock1(currentMeshMutex);
        std::unique_lock<std::mutex> lock2(indicesCountMutex);
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
        uint32_t EFFECTIVE_GRID_SIZE = this->isPermaPlane ? 2 : (FILE_GRID_SIZE-1) / LOD + 1;
        LOD = this->isPermaPlane ? HGTLOD::PERFECT : LOD;
        uint32_t HGT_INT_WIDTH = 2;

        double sumSoFar = 0.0;

        assert((FILE_GRID_SIZE-1) % LOD == 0);

        std::vector<int> holesToFill;
        float properCount = 0;

        // 1. first read bytes
        std::vector<uint16_t> altitudes;
        for (int i = 0; i<FILE_GRID_SIZE; i+=LOD) {
            for (int j = 0; j<FILE_GRID_SIZE; j+=LOD) {
                unsigned int buffer_offset = HGT_INT_WIDTH * FILE_GRID_SIZE * i + HGT_INT_WIDTH * j;
                unsigned int highByte = buffer[buffer_offset];
                unsigned int lowByte = buffer[buffer_offset+1];
                int16_t altitude = (((uint16_t)highByte << 8) | lowByte);
                altitudes.push_back(altitude);
            }
        }

        std::vector<uint16_t> smoothValues;
        for (int i = 0; i<FILE_GRID_SIZE; i+=1) {
            for (int j = 0; j<FILE_GRID_SIZE; j+=1) {
                smoothValues.push_back(altitudes[FILE_GRID_SIZE * i + j]);
            } 
        }

        for (int i = 0; i<EFFECTIVE_GRID_SIZE; i+=1) {
            for (int j = 0; j<EFFECTIVE_GRID_SIZE; j+=1) {
                int16_t altitude = smoothValues[EFFECTIVE_GRID_SIZE * i + j];

                float latProgress = (float)i / (float)(EFFECTIVE_GRID_SIZE - 1);
                float latitude = (latitudeBase + 1.0f) - (latProgress * 1.0f); 
                float lonProgress = (float)j / (float)(EFFECTIVE_GRID_SIZE - 1);
                float longitude = longtitudeBase + (lonProgress * 1.0f);

                auto point = Math::angleToEarthPoint(latitude, altitude, longitude);

                vertices_buffer.insert(vertices_buffer.end(), {
                    point.x / 6371000.0f, 
                    point.y / 6371000.0f, 
                    point.z / 6371000.0f
                });

                if (altitude > -500) {
                    sumSoFar += glm::length(point) / 6371000.0f;
                }

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
        int LOD
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
