#include <Engine/Support/HGT/HGT.hpp>
#include <iostream>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <Engine/Core/Misc/StrLib.hpp>
#include <Engine/Core/OpenGL.hpp>

namespace Engine
{
    glm::vec3 angleToEarthPoint(float latitude, float altitude, float longitude) {
        const float earth_radius = 6371000.0f; // meters
        float radius = altitude + earth_radius;
        float latRad = glm::radians(latitude);
        float lonRad = glm::radians(longitude);
        float x = radius * cos(latRad) * cos(lonRad);
        float y = radius * sin(latRad);
        float z = radius * cos(latRad) * sin(lonRad);
        return glm::vec3(x, y, z);
    };

    HGTTile::HGTTile() {};
    
    void HGTTile::bindAndRender() {
        if (!this->currentMesh) {
            std::cout << "Trying to render empty tile!\n";
            return;
        }
        this->currentMesh->render();
    };

    HGTLOD HGTTile::getLodFromPosition(glm::vec3 position) {
        float distance = glm::length(position - center);
        // we should fine tune this
        if (distance > 1.0f) return HGTLOD::PLANE;
        if (distance > 0.9f) return HGTLOD::VERY_LOW;
        if (distance > 0.8f) return HGTLOD::LOW;
        if (distance > 0.7f) return HGTLOD::MEDIUM;
        if (distance > 0.6f) return HGTLOD::HIGH;
        if (distance > 0.5f) return HGTLOD::VERY_HIGH;
        return HGTLOD::PERFECT;
    };

    void HGTTile::runGarbageCollector() {
        if (this->currentMesh->LOD == HGTLOD::PLANE) {
            // clean everything else
            this->meshQueue.clear();
            return;
        }

        if (this->meshQueue.size() >= 2) {
            int i = 0;
            while (i < this->meshQueue.size()) {
                if (this->meshQueue[i]->LOD <= HGTLOD::MEDIUM) {
                    std::swap( this->meshQueue[i], this->meshQueue.back() );
                    this->meshQueue.pop_back();
                } else {
                    i++;
                }
            }
        }
    }

    void HGTTile::loadLODFromPlayerPosition(glm::vec3 position) {
        // check current mesh or get one from the queue
        auto LOD = this->getLodFromPosition(position);
        if (this->currentMesh->LOD == LOD) return;
        for (auto& mesh : this->meshQueue) {
            if (mesh->LOD == LOD) {
                this->meshQueue.push_back(this->currentMesh);
                this->currentMesh = mesh;
                return;
            }
        }
        
        std::vector<float> vertices_buffer;
        std::vector<uint32_t> indices_buffer;

        // 1. quick data load
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        auto fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<uint8_t> buffer(fileSize);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
            throw std::runtime_error("Błąd odczytu: " + path);
        }

        std::vector<std::string> out;
        split_string(path, '/', out);
        std::string tile_name = out.back().substr(0, 7);
        std::cout << "Loading " << tile_name << "\n";

        // 2. turn buffer into a sample vector
        float latitudeBase = std::stof(tile_name.substr(1, 2)) * (tile_name[0] == 'N' ? 1.0f : -1.0f);
        float longtitudeBase = std::stof(tile_name.substr(4, 3)) * (tile_name[3] == 'E' ? 1.0f : -1.0f);

        uint32_t FILE_GRID_SIZE = 1201;
        uint32_t EFFECTIVE_GRID_SIZE = (1200/LOD) + 1;
        uint32_t HGT_INT_WIDTH = 2;

        float angularStep = 1.0f / ((float)FILE_GRID_SIZE);

        for (int i = 0; i<FILE_GRID_SIZE; i+=LOD) {
            for (int j = 0; j<FILE_GRID_SIZE; j+=LOD) {
                unsigned int buffer_offset = HGT_INT_WIDTH * FILE_GRID_SIZE * i + HGT_INT_WIDTH * j;
                unsigned int highByte = buffer[buffer_offset];
                unsigned int lowByte = buffer[buffer_offset+1];

                float altitude = (float)(((uint16_t)highByte << 8) | lowByte);
                float longitude = longtitudeBase + (1.0f - angularStep * ((float)i));
                float latitude = latitudeBase + angularStep * ((float)j);

                auto point = angleToEarthPoint(latitude, altitude, longitude);

                vertices_buffer.insert(vertices_buffer.end(), {
                    point.x / 6371000.0f, 
                    point.y / 6371000.0f, 
                    point.z / 6371000.0f
                });
                if (i == 0 || j == 0) continue;
                // a - b
                // | \ |
                // c - d
                uint32_t offset_d = vertices_buffer.size() / 3;
                uint32_t offset_a = offset_d - EFFECTIVE_GRID_SIZE - 1;
                uint32_t offset_b = offset_d - EFFECTIVE_GRID_SIZE;
                uint32_t offset_c = offset_d - 1;
                indices_buffer.insert(indices_buffer.end(), {
                    offset_a, offset_c, offset_d, offset_b, offset_a, offset_d
                });
            }
        }
        
        auto centerIndex = (vertices_buffer.size() / 3)/2;
        this->center = glm::vec3(vertices_buffer[centerIndex], vertices_buffer[centerIndex+1], vertices_buffer[centerIndex+2]);
        auto mesh = HGTTileMesh::build(vertices_buffer, indices_buffer, LOD);
        this->meshQueue.push_back(this->currentMesh);
        this->currentMesh = mesh;
        this->runGarbageCollector();
    };

    HGTTile* HGTTile::buildFrom(const std::string& path) {
        HGTTile *tile = new HGTTile();
        tile->path = path;
        tile->center = glm::vec3(0.0f, 0.0f, 0.0f);
        return tile;
    };

    HGTTileMesh::HGTTileMesh() {};

    void HGTTileMesh::render() {
        glBindVertexArray(this->vao);
        glDrawElements(GL_TRIANGLES, this->indices_count, GL_UNSIGNED_INT, (void*)0);
        glBindVertexArray(0);
    };

    HGTTileMesh* HGTTileMesh::build(
        const std::vector<float>& vertices_buffer, 
        const std::vector<uint32_t>& indices_buffer,
        HGTLOD LOD
    ) {
        HGTTileMesh* mesh = new HGTTileMesh();

        mesh->indices_buffer_cache = indices_buffer;
        mesh->vertices_buffer_cache = vertices_buffer;
        mesh->LOD = LOD;

        glGenVertexArrays(1, &mesh->vao);
        glGenBuffers(1, &mesh->vbo);
        glGenBuffers(1, &mesh->ebo);

        glBindVertexArray(mesh->vao);
        
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices_buffer.size() * sizeof(float), vertices_buffer.data(), GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_buffer.size() * sizeof(uint32_t), indices_buffer.data(), GL_STATIC_DRAW);
        
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    };

    HGTTileMesh::~HGTTileMesh() {
        if (this->vao) glDeleteVertexArrays(1, &this->vao);
        if (this->vbo) glDeleteBuffers(1, &this->vbo);
        if (this->ebo) glDeleteBuffers(1, &this->ebo);
    }
};
