#include <Engine/Core/Material.hpp>
#include "Engine/Core/TextureRepository.hpp"
#include "glm/fwd.hpp"
#include <Engine/Core/MeshRepository.hpp>
#include <cstdio>
#include <exception>
#include <stdexcept>
#include <unordered_map>
#include <Engine/Core/Misc/Types.hpp>
#include <Engine/Core/Scene/GameObject.hpp>
#include <string>
#include <fstream>
#include <iostream>
#include <Engine/Core/OpenGL.hpp>
#include <Engine/Core/Misc/StrLib.hpp>

namespace Engine 
{
    MeshRepository::MeshRepository() {};
 
    EngineID MeshRepository::getNextMeshIndex() {
        return this->meshCounter++;
    };

    EngineID MeshRepository::loadMeshFromOBJFileWithPreprocessing(
        const std::string &path, 
        glm::quat rotation, 
        TextureRepository *textureRepository
    ) {
        std::ifstream file(path);
        if (!file.is_open())
            throw std::runtime_error(std::string("Could not open mesh file: ") + path);

        std::string line;

        std::vector<glm::vec3> vectors;
        std::vector<glm::vec3> normals = {{0.0f, 0.0f, 0.0f}};
        std::vector<glm::vec2> textureCoords = {{0.0f, 0.0f}};
        Material* material = Material::createBase(textureRepository);
        Mesh *mesh = new Mesh();
        std::vector<MeshDrawStep> drawingSteps = {MeshDrawStep{
            .which = "default",
            .offset = 0,
            .amountToDraw = 0
        }};

        int materialTriangleCount = 0;
        int triangleCount = 0;

        bool shouldCalculateNormals = false;

        int notImplementedCount = 0;

        std::vector<std::string> pathParts;
        split_string(path, '/', pathParts);
        pathParts.pop_back();
        std::string rootPath = join_string(pathParts, "/");


        while (getline (file, line)) {
            try {
                std::vector<std::string> tokens = {};
                // cleanup the line
                // std::cout << line << "\n";
                line.erase(std::remove(line.begin(), line.end(), '\n'), line.cend());
                line.erase(std::remove(line.begin(), line.end(), '\r'), line.cend());
                split_string(line, ' ', tokens);
                
                if (tokens.size() <= 0) continue;

                if (tokens[0] == "mtllib") {
                    std::cout << "Recursively loading " << rootPath + "/" + tokens[1] << "\n";
                    material->mergeWith(Material::loadMTLFile(rootPath + "/" + tokens[1], textureRepository));
                    continue;
                }

                if (tokens[0] == "usemtl") {
                    std::vector<std::string> name_split;
                    for (int i = 1; i<tokens.size(); i++) name_split.push_back(tokens[i]);
                    std::string name = join_string(name_split, " ");
                    if (drawingSteps.back().which == name) continue;

                    drawingSteps.back().amountToDraw = materialTriangleCount;
                    drawingSteps.push_back(MeshDrawStep{
                        .which = name,
                        .offset = triangleCount,
                        .amountToDraw = 0
                    });
                    materialTriangleCount = 0;
                    continue;
                }

                if (tokens[0] == "v") {
                    // parse vector
                    // assume w is allways 1.0
                    vectors.push_back({
                        std::stof(tokens[1]), 
                        std::stof(tokens[2]), 
                        std::stof(tokens[3])
                    });
                }

                if (tokens[0] == "vn") {
                    // parse normal
                    normals.push_back({
                        std::stof(tokens[1]), 
                        std::stof(tokens[2]), 
                        std::stof(tokens[3])
                    });
                }

                if (tokens[0] == "vt") {
                    // parse normal
                    textureCoords.push_back({
                        std::stof(tokens[1]), 
                        std::stof(tokens[2]), 
                    });
                }

                if (tokens[0] == "vp") {
                    // whatever this is - ignore this for now
                    notImplementedCount++;
                }

                if (tokens[0] == "l") {
                    notImplementedCount++;
                }

                if (tokens[0] == "s") {
                    notImplementedCount++;
                    if (tokens[1] == "1" || tokens[1] == "on") {
                        mesh->enableSmoothTriangleInsertion();
                    } else {
                        mesh->disableSmoothTriangleInsertion();
                    }
                }

                if (tokens[0] == "o") {
                    notImplementedCount++;
                }

                if (tokens[0] == "f") {
                    // parse face
                    std::vector<glm::ivec3> res = {};
                    for (int t = 1; t<tokens.size(); t++) {
                        std::vector<std::string> parts = {};
                        split_string(tokens[t], '/', parts);
                        if (parts.size() == 1) {
                            // only faces
                            shouldCalculateNormals=true;
                            int v = std::stoi(parts[0])-1;
                            res.push_back({v, 0, 0});
                        }
                        if (parts.size() == 2) {
                            // vertex/texture
                            shouldCalculateNormals=true;
                            int v = std::stoi(parts[0])-1;
                            int tc = std::stoi(parts[1]);
                            res.push_back({v, 0, tc});
                        }
                        if (parts.size() >= 3) {
                            // vertex/texture/normal
                            int v = std::stoi(parts[0])-1;
                            int tc = std::stoi(parts[1]);
                            int u = std::stoi(parts[2]);
                            res.push_back({v, u, tc});
                        }
                    }

                    if (res.size() == 3) {
                        triangleCount+=3;
                        materialTriangleCount+=3;
                        mesh->push_full_info_vertex(vectors[res[0].x], normals[res[0].y], textureCoords[res[0].z]);
                        mesh->push_full_info_vertex(vectors[res[1].x], normals[res[1].y], textureCoords[res[1].z]);
                        mesh->push_full_info_vertex(vectors[res[2].x], normals[res[2].y], textureCoords[res[2].z]);
                    }

                    if (res.size() == 4) {
                        triangleCount+=6;
                        materialTriangleCount+=6;
                        mesh->push_full_info_vertex(vectors[res[0].x], normals[res[0].y], textureCoords[res[0].z]);
                        mesh->push_full_info_vertex(vectors[res[1].x], normals[res[1].y], textureCoords[res[1].z]);
                        mesh->push_full_info_vertex(vectors[res[2].x], normals[res[2].y], textureCoords[res[2].z]);

                        mesh->push_full_info_vertex(vectors[res[0].x], normals[res[0].y], textureCoords[res[0].z]);
                        mesh->push_full_info_vertex(vectors[res[2].x], normals[res[2].y], textureCoords[res[2].z]);
                        mesh->push_full_info_vertex(vectors[res[3].x], normals[res[3].y], textureCoords[res[3].z]);
                    }
                }
            } catch(std::invalid_argument &exception) {
                std::cerr << "Error while parsing: \n" << line << "\n - " << "(" << exception.what() << ")\n";
                std::exit(-1);
            }
        }
        file.close();
        std::cout << "File loading finished\n";
        std::cout << "loaded " << mesh->size() << " floats " << "\n";
        // if (shouldCalculateNormals) return this->loadMesh(mesh);
        mesh->setMaterial(material);
        drawingSteps.back().amountToDraw = materialTriangleCount;

        std::cout << "Drawing steps for: " << path << "\n";
        for (auto& drawingStep : drawingSteps) {
            if (drawingStep.amountToDraw == 0) continue;
            mesh->addDrawingStep(drawingStep);
            std::cout << "- " << drawingStep.offset << "|" <<  drawingStep.amountToDraw << "|" << drawingStep.which << "\n";
        }
        mesh->sanityCheck();



        return this->loadMesh(mesh);
    };

    EngineID MeshRepository::loadMeshFromOBJFile(const std::string &path, TextureRepository *textureRepository) {
        return this->loadMeshFromOBJFileWithPreprocessing(path, {0.0f, 1.0f, 0.0f, 0.0f}, textureRepository);
    };

    EngineID MeshRepository::loadMeshFromFileWithPreprocessing(const std::string &path, glm::quat rotation) {
        std::ifstream file(path);
        if (!file.is_open())
            throw std::runtime_error(std::string("Could not open mesh file: ") + path);

        std::string line;

        std::vector<glm::vec3> vectors;
        Mesh *mesh = new Mesh();

        while (getline (file, line)) {
            if (line.size() < 0) continue;
            if (line[0] == 'v') {
                float x, y, z;
                std::vector<std::string> ns;
                split_string(line, ' ', ns);
                try {
                    x = std::stof(ns[0]);
                    y = std::stof(ns[1]);
                    z = std::stof(ns[2]);
                }
                catch(const std::exception& e) {
                    std::cerr << "Error while reading Mesh:" << e.what() << '\n';
                    return -1;
                }
                vectors.push_back({x,y,z});
            }

            if (line[0] == 't') {
                int x, y, z;
                std::vector<std::string> ns;
                split_string(line, ' ', ns);
                try {
                    x = std::stoi(ns[0]);
                    y = std::stoi(ns[1]);
                    z = std::stoi(ns[2]);
                }
                catch(const std::exception& e) {
                    std::cerr << "Error while reading Mesh:" << e.what() << '\n';
                    return -1;
                }
                glm::vec3 triangle[3] = {vectors[x], vectors[y], vectors[z]};
                mesh->push_triangle(triangle);
            }
        }
        
        file.close();
        // TODO: Make this actually do something (rotate is not implemented)
        mesh->rotate(rotation);
        return this->loadMesh(mesh);
    };  

    EngineID MeshRepository::loadMeshFromFile(const std::string &path) {
        return this->loadMeshFromFileWithPreprocessing(path, {0.0f, 1.0f, 0.0f, 0.0f});
    };
    
    EngineID MeshRepository::loadMesh(Mesh *mesh) {
        EngineID engineID = this->getNextMeshIndex();
        mesh->calculateSmoothNormals();
        this->meshMap[engineID] = mesh;

        // Build VAO
        EngineID meshVAO, meshVBO;
        glGenVertexArrays(1, &meshVAO);
        glGenBuffers(1, &meshVBO);

        glBindVertexArray(meshVAO);
        glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh->size(), mesh->get_data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);  
        glBindVertexArray(0);
        
        this->VAOMap[engineID] = meshVAO;
        return engineID;
    };
    
    void MeshRepository::assignMesh(GameObject* gameObject, EngineID meshId) {
        this->objectMap[gameObject] = meshId;
        this->meshToGameObjects[meshId].push_back(gameObject);
    };

    unsigned int MeshRepository::getMeshVAO(EngineID meshId) {
        return this->VAOMap[meshId];
    }; 

    size_t MeshRepository::getMeshSize(EngineID meshId) {
        return this->meshMap[meshId]->size() / 8;
    };

    EngineID MeshRepository::getMeshIDByGameObject(GameObject *gameObject) {
        return this->objectMap[gameObject];
    }

    Mesh* MeshRepository::getMeshByID(EngineID engineID) {
        return this->meshMap[engineID];
    };

    std::vector<Mesh*> MeshRepository::getAllMeshes() {
        std::vector<Mesh*> result;
        for (auto& [id, mesh] : this->meshMap) result.push_back(mesh);
        return result;
    };
};

