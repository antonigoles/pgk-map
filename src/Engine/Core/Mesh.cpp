#include <Engine/Core/Mesh.hpp>
#include <Engine/Core/OpenGL.hpp>
#include <iostream>
#include <unordered_set>

namespace Engine {
	static glm::vec3 zeroVec = {0.0f, 0.0f, 0.0f};
	Mesh::Mesh() {
        this->shouldInsertAsSmooth=false;
    };

    void Mesh::enableSmoothTriangleInsertion() {
        this->shouldInsertAsSmooth=true;
    };

    void Mesh::disableSmoothTriangleInsertion() {
        this->shouldInsertAsSmooth=false;
    };

    const std::vector<MeshDrawStep>& Mesh::getDrawingSteps() {
        return this->drawingSteps;
    }

    void Mesh::setMaterial(Material *material) {
        this->material = material;
    };

    Material* Mesh::getMaterial() {
        return this->material;
    };

    void Mesh::addDrawingStep(MeshDrawStep step) {
        this->drawingSteps.push_back(step);
    };

    void Mesh::push_full_info_triangle(glm::vec3 vertices[3], glm::vec3 normals[3], glm::vec2 texCoords[3]) {
        this->push_full_info_vertex(vertices[0], normals[0], texCoords[0]);
        this->push_full_info_vertex(vertices[1], normals[1], texCoords[1]);
        this->push_full_info_vertex(vertices[2], normals[2], texCoords[2]);
    };

    void Mesh::push_full_info_vertex(glm::vec3 vertex, glm::vec3 normal, glm::vec2 texCoords) {
        this->isSmoothShaded.push_back(shouldInsertAsSmooth);
        this->data.insert(
            this->data.end(), 
            {vertex.x, vertex.y, vertex.z, normal.x, normal.y, normal.z, texCoords.x, texCoords.y}
        );
    };

	void Mesh::push_triangle_with_normals(glm::vec3 vertices[3], glm::vec3 normals[3]) {
		push_vertex_with_normal(vertices[0], normals[0]);
		push_vertex_with_normal(vertices[1], normals[1]);
		push_vertex_with_normal(vertices[2], normals[2]);
	};

	void Mesh::push_triangle(glm::vec3 vertices[3]) {
		glm::vec3 normals[3] = {zeroVec, zeroVec, zeroVec};
		this->push_triangle_with_normals(vertices, normals);
	};
	
	void Mesh::push_vertex_with_normal(glm::vec3 vertex, glm::vec3 normal) {
        this->push_full_info_vertex(vertex, normal, {0.0f, 0.0f});
	};

	void Mesh::rotate(glm::quat rot) {
        // TODO: Implement
        // remeber to make sure normals are being correctly calculated
    };

    void Mesh::transform_vertices(std::function<void(float*, float*, float*)> transformer) {
        for (int i = 0; i<this->size(); i+=8) {
            transformer(&this->data[i], &this->data[i+1], &this->data[i+2]);
        }
    }

    void Mesh::foreach_triangle(std::function<void(const glm::vec3[3], const glm::vec3[3], const glm::vec2[3])> callback) {
        for (int i = 0; i<this->size(); i+=24) {
            glm::vec3 vec[3] = {
                glm::vec3{this->data[i], this->data[i+1], this->data[i+2]},
                glm::vec3{this->data[i+8], this->data[i+9], this->data[i+10]},
                glm::vec3{this->data[i+16], this->data[i+17], this->data[i+18]}
            }; 
            glm::vec3 norm[3] = {
                glm::vec3{this->data[i+3], this->data[i+4], this->data[i+5]},
                glm::vec3{this->data[i+11], this->data[i+12], this->data[i+13]},
                glm::vec3{this->data[i+19], this->data[i+20], this->data[i+21]}
            }; 
            glm::vec2 tm[3] = {
                glm::vec2{this->data[i+6], this->data[i+7]},
                glm::vec2{this->data[i+14], this->data[i+15]},
                glm::vec2{this->data[i+22], this->data[i+23]}
            }; 

            callback(vec, norm, tm);
        }
    }
	
	void Mesh::push_vertex(glm::vec3 vertex) {
		this->push_vertex_with_normal(vertex, zeroVec);
	};

    // TODO: Rename and remake this into: calculate smooth shading
	void Mesh::calculateSmoothNormals() {
        return;
		std::unordered_map<glm::vec3, glm::vec3> normals;
		for (int i = 0; i<this->size(); i+=24) {       
            glm::vec3 v1{this->data[i], this->data[i+1], this->data[i+2]};
            glm::vec3 n1{this->data[i+3], this->data[i+4], this->data[i+5]};
            
            glm::vec3 v2{this->data[i+8], this->data[i+9], this->data[i+10]};
            glm::vec3 n2{this->data[i+11], this->data[i+12], this->data[i+13]};
            
            glm::vec3 v3{this->data[i+16], this->data[i+17], this->data[i+18]};
            glm::vec3 n3{this->data[i+19], this->data[i+20], this->data[i+21]};

            glm::vec3 normal = glm::cross(v2 - v1, v3 - v1);
            normal = glm::normalize(normal);
            
            if (!normals.contains(v1)) normals[v1] = glm::vec3{0,0,0};
            if (!normals.contains(v2)) normals[v2] = glm::vec3{0,0,0};
            if (!normals.contains(v3)) normals[v3] = glm::vec3{0,0,0};

            normals[v1] += n1;
            normals[v2] += n2;
            normals[v3] += n3;
        }
        for (int i = 0; i<this->size(); i+=8) {       
            glm::vec3 vec{this->data[i], this->data[i+1], this->data[i+2]};

            if (this->isSmoothShaded[i / 8]) {
                glm::vec3 norm = glm::normalize(normals[vec]);
            	this->data[i+3]=norm.x;
            	this->data[i+4]=norm.y;
            	this->data[i+5]=norm.z;
            }
        }
	};

	std::pair<glm::vec3, glm::vec3> Mesh::getCuboid(float padding) {
		float maxX = std::numeric_limits<float>::min();
        float maxY = std::numeric_limits<float>::min();
        float maxZ = std::numeric_limits<float>::min();

        float minX = std::numeric_limits<float>::max();
        float minY = std::numeric_limits<float>::max();
        float minZ = std::numeric_limits<float>::max();
        
        for (int i = 0; i<this->data.size(); i+=8) {
            float x = this->data[i];
            float y = this->data[i+1];
            float z = this->data[i+2];
            
            maxX = std::max(x, maxX);
            maxY = std::max(y, maxY);
            maxZ = std::max(z, maxZ);

            minX = std::min(x, minX);
            minY = std::min(y, minY);
            minZ = std::min(z, minZ);
        }

        return {{minX - padding, minY - padding, minZ - padding}, {maxX + padding, maxY + padding, maxZ + padding}};
	}

	unsigned int Mesh::size() {
		return this->data.size();
	};

	float* Mesh::get_data() {
		return this->data.data();
	};

    // @Deprecated this will not work!
    std::vector<unsigned int> Mesh::serialize_to_v1_format() {
        // First map normals and vertices to indexes to reduce repetitions
        int normalId = 0;
        int vertexId = 0;
        std::vector<glm::vec3> vertexVector;
        std::vector<glm::vec3> normalVector;
        std::unordered_map<glm::vec3, int> vertexMap;
        std::unordered_map<glm::vec3, int> normalMap;

        std::vector<int> triangle;
        std::vector<int> triangles; 

        std::cout << "Builing triangle data...\n";
        for (int i = 0; i<this->data.size(); i+=6) {
            glm::vec3 vert = { this->data[i], this->data[i+1], this->data[i+2] };
            glm::vec3 norm = { this->data[i+3], this->data[i+4], this->data[i+5] };

            if (!vertexMap.contains(vert)) {
                vertexMap[vert] = vertexId++;
                vertexVector.push_back(vert);
            }
            if (!normalMap.contains(norm)) {
                normalMap[norm] = normalId++;
                normalVector.push_back(norm);
            }
            int n_id = normalMap[norm];
            int v_id = vertexMap[vert];

            triangle.insert(triangle.end(), {v_id, n_id});
            if (triangle.size() == 6) {
                for (auto& e : triangle ) triangles.push_back(e);
                triangle = {};
            }
        }

        std::cout << "Packing triangle data...\n";
        std::vector<unsigned int> totalBuffer;
        totalBuffer.push_back(std::bit_cast<unsigned int>(vertexId));
        for (int i = 0; i<vertexId; i++) {
            totalBuffer.insert(totalBuffer.end(), {
                std::bit_cast<unsigned int>(vertexVector[i].x),
                std::bit_cast<unsigned int>(vertexVector[i].y),
                std::bit_cast<unsigned int>(vertexVector[i].z),
            });
        }

        totalBuffer.push_back(std::bit_cast<unsigned int>(normalId));
        for (int i = 0; i<normalId; i++) {
            totalBuffer.insert(totalBuffer.end(), {
                std::bit_cast<unsigned int>(normalVector[i].x),
                std::bit_cast<unsigned int>(normalVector[i].y),
                std::bit_cast<unsigned int>(normalVector[i].z),
            });
        }

        totalBuffer.push_back(triangles.size()/6);
        for (int i = 0; i<triangles.size(); i++) {
            totalBuffer.push_back(triangles[i]);
        }

        return totalBuffer;
    };

    void Mesh::sanityCheck() {
        // 1. check for repeating triangles
        int repeatinTriangleCount = 0;
        std::unordered_map<glm::mat3, bool> s;
        for (int i = 0; i<this->data.size(); i+=24) {
            glm::vec3 v1 = {this->data[i], this->data[i+1], this->data[i+2]};
            glm::vec3 v2 = {this->data[i+8], this->data[i+9], this->data[i+10]};
            glm::vec3 v3 = {this->data[i+16], this->data[i+17], this->data[i+18]};

            glm::mat3 m[6] = {
                {v1, v2, v3},
                {v1, v3, v2},
                {v2, v1, v3},
                {v2, v3, v2},
                {v3, v1, v2},
                {v3, v2, v1}
            };

            for (int j = 0; j<6; j++) {
                if (s.contains(m[j])) {
                    repeatinTriangleCount++;
                    // std::cout << "Mesh sanity check failed: repeating triangles\n";

                    // * autoclean
                    // not effective tho
                    //
                    // for (int k = 0; k<24; k++) {
                    //     std::swap(this->data[k], this->data[this->data.size()-24 + k]);
                    // }
                    // for (int k = 0; k<24; k++) this->data.pop_back();
                }
                s[m[j]] = true;
            }
        }
        std::cout << repeatinTriangleCount << " repeated triangles\n";
    }
};