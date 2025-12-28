#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <Engine/Core/Texture.hpp>
#include <Engine/Core/Rendering/Renderable.hpp>
#include <Engine/Core/Math/Transform.hpp>
#include <unordered_set>

namespace Engine
{
    class HGTSample {
    public:
        float longitude;
        float latitude;
        int16_t altitude;
    };

    enum HGTLOD {
        PLANE = 1200,
        VERY_LOW = 600,
        LOW = 300,
        MEDIUM = 150,
        HIGH = 12,
        VERY_HIGH = 4,
        PERFECT = 1
    };

    class HGTTileMesh {
    private:
        ssize_t indices_count;

        unsigned int vao;
        unsigned int vbo;
        unsigned int ebo;

        std::vector<float> vertices_buffer_cache;
        std::vector<uint32_t> indices_buffer_cache;
    public:
        HGTLOD LOD;    

        HGTTileMesh();
        ~HGTTileMesh();

        void render();

        static HGTTileMesh* build(
            const std::vector<float>& vertices_buffer, 
            const std::vector<uint32_t>& indices_buffer,
            HGTLOD LOD
        );
    };

    class HGTTile {
    private:
        std::vector<HGTTileMesh*> meshQueue;
        HGTTileMesh* currentMesh;

        std::string path;
        glm::vec3 center;

        HGTLOD lastLOD;

        HGTLOD getLodFromPosition(glm::vec3 position);

        void runGarbageCollector();
    public:
        HGTTile();

        void bindAndRender();

        void buildGpuBuffers();

        void fastGPUTransfer();

        void loadLODFromPlayerPosition(glm::vec3 position);

        // We start from loading a minimum version with least amount of vertices, then depending on distance
        // We'll gradually increase the resolution
        static HGTTile* buildFrom(const std::string& path);
    };

    class HGT : public Renderable {
    private:
        std::unordered_map<std::string, std::vector<HGTSample>> sampleBuffers;
        std::unordered_map<std::string, HGTTile*> tiles; // a tile is usually a single file (1x1 degrees^2)

        
        unsigned int shaderID;
    public:
        HGT();

        Texture* getTileTexture(const std::string& tile);

        void render_init(SceneContext *sceneContext) override;
        void render(RendererContext *context) override;

        void interp_buffer_and_load(const std::vector<uint8_t>& buffer, const std::string& tile);
        
        static HGT* fromDataSource(const std::string& path);

        Math::Transform transform;
    };
};
