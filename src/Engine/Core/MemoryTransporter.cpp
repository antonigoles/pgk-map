#include <Engine/Core/MemoryTransporter.hpp>
#include <iostream>

namespace Engine {
	MemoryTransporter::MemoryTransporter() {

	};

	void MemoryTransporter::registerLight(GLSL_LIGHT_BLOCK block) {
		this->lightBuffer.push_back(block);
	};

	void MemoryTransporter::registerGlobalBlock(GLSL_GLOBAL_BLOCK block) {
		this->globalBlock = block;
	};

	unsigned int MemoryTransporter::registerMaterial(const std::string& name, GLSL_MTL_block block) {
		this->materialBuffer.push_back({name, block});
		return this->materialBuffer.size()-1;
	};

	unsigned int MemoryTransporter::getMaterialBufferIndex(const std::string& name) {
		for (int i = 0; i<this->materialBuffer.size(); i++) {
			if (this->materialBuffer[i].first == name) return i;
		}
		std::cout << "Material not found" << "\n";
		std::exit(-1);
		return -1;
	};

	void MemoryTransporter::initAllSSBOS() {
		// init material SSBO
		std::vector<GLSL_MTL_block> normalziedBuffer;
		for (auto& [name, block] : this->materialBuffer) {
			normalziedBuffer.push_back(block);
		}
		glGenBuffers(1, &this->materialBufferSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->materialBufferSSBO);
        glBufferData(
            GL_SHADER_STORAGE_BUFFER, 
            sizeof(GLSL_MTL_block) * normalziedBuffer.size(), 
            normalziedBuffer.data(), 
            GL_STATIC_DRAW
        );

        // init global BLOCK
		glGenBuffers(1, &this->globalBlockBufferSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->globalBlockBufferSSBO);
        glBufferData(
            GL_SHADER_STORAGE_BUFFER, 
            sizeof(GLSL_GLOBAL_BLOCK), 
            &this->globalBlock, 
            GL_STATIC_DRAW
        );

        // init light BLOCK
		glGenBuffers(1, &this->lightBufferSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->lightBufferSSBO);
        glBufferData(
            GL_SHADER_STORAGE_BUFFER, 
            sizeof(GLSL_LIGHT_BLOCK) * this->lightBuffer.size(), 
            this->lightBuffer.data(), 
            GL_STATIC_DRAW
        );
	};

	void MemoryTransporter::bindStatic() {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, this->materialBufferSSBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, this->globalBlockBufferSSBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, this->lightBufferSSBO);
	};

	void MemoryTransporter::cleanUp() {
		this->lightBuffer.clear();
		this->materialBuffer.clear();
	};
}