#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <Engine/Core/Texture.hpp>
#include <Engine/Core/Rendering/Renderable.hpp>
#include <Engine/Core/Math/Transform.hpp>
#include <set>
#include <memory>

namespace Engine
{
    enum HGTLOD {
        PLANE = 1200,
        VERY_LOW = 600,
        LOW = 300,
        MEDIUM = 150,
        HIGH = 12,
        VERY_HIGH = 4,
        PERFECT = 1
    };

    class LOD_MARGINS {
    public:
        static float PLANE;
        static float VERY_LOW;
        static float LOW;
        static float MEDIUM;
        static float HIGH;
        static float VERY_HIGH;
    };

    // TODO: Maaybe implement this?
    class HGTTileMesh;

    class HGTTileSubMesh {
    private:
        HGTTileMesh* parent;

        glm::vec3 position;

        ssize_t index;
    public:
        HGTTileSubMesh();
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

        bool pendingTransfer;

        HGTTileMesh();
        ~HGTTileMesh();

        void render();

        void transferBuffers();

        static std::unique_ptr<HGTTileMesh> build(
            const std::vector<float>& vertices_buffer, 
            const std::vector<uint32_t>& indices_buffer,
            HGTLOD LOD
        );
    };

    class HGT;

    class HGTTile {
    private:
        std::vector<std::unique_ptr<HGTTileMesh>> meshQueue;
        std::unique_ptr<HGTTileMesh> currentMesh;

        bool isPermaPlane;

        std::string path;

        HGTLOD lastLOD;

        HGT* parent;

        HGTLOD getLodFromPosition(glm::vec3 position);

        void loadLODFromPlayerPositionInternal(glm::vec3 position);
    public:
        glm::vec3 center;

        HGTTile(HGT* parent);

        HGTLOD getLod();

        bool bindAndRender();

        void loadLODFromPlayerPosition(glm::vec3 position);

        void runPendingTransfers();

        void runGarbageCollector();

        static HGTTile* buildFrom(const std::string& path, HGT* parent);

        static HGTTile* buildPlaneAt(glm::ivec2 vec, HGT* parent);

        static void lodLoadTask(HGTTile *tile, glm::vec3 position);
    };

    class HGT : public Renderable {
    private:
        unsigned int shaderID;

    public:
        std::unordered_map<std::string, HGTTile*> tiles; // a tile is usually a single file (1x1 degrees^2)
        std::vector<HGTTile*> tilesSorted;

        HGT();

        Texture* getTileTexture(const std::string& tile);

        void render_init(SceneContext *sceneContext) override;
        void render(RendererContext *context) override;

        void interp_buffer_and_load(const std::vector<uint8_t>& buffer, const std::string& tile);
        
        glm::ivec2 get_tile_on_top();

        std::string ivec2_to_tile_name(const glm::ivec2& vec);

        static HGT* fromDataSource(const std::string& path);

        Math::Transform transform;
    };
};
