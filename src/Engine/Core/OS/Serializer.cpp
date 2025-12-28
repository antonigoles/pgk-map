#include <Engine/Core/OS/Serializer.hpp>
#include <fstream>
#include <zlib.h>
#include <iostream>
#include <bitset>

namespace Engine {
	std::vector<unsigned char> compress_buffer(const std::vector<unsigned int>& input) {
	    const Bytef* src = reinterpret_cast<const Bytef*>(input.data());
	    uLong src_len = input.size() * sizeof(unsigned int);

	    uLongf dst_len = compressBound(src_len); // maksymalny rozmiar skompresowanego bufora
	    std::vector<unsigned char> compressed(dst_len);

	    int res = compress(compressed.data(), &dst_len, src, src_len);
	    if (res != Z_OK) {
	        throw std::runtime_error("compress failed");
	    }
	    compressed.resize(dst_len); // faktyczny rozmiar
	    return compressed;
	}

	std::vector<unsigned int> decompress_buffer(const std::vector<unsigned char>& compressed, size_t original_size) {
	    std::vector<unsigned int> decompressed(original_size / sizeof(unsigned int));

	    uLongf dst_len = original_size;
	    int res = uncompress(reinterpret_cast<Bytef*>(decompressed.data()), &dst_len,
	                         compressed.data(), compressed.size());
	    if (res != Z_OK) {
	        throw std::runtime_error("uncompress failed");
	    }
	    return decompressed;
	}

	void Serializer::writeMeshAsBinaryStreamOfFloats(
		const std::string& path, 
		Mesh *mesh
	) {
		// TODO: Create another format that utilizes vertex indexing, 
		// this is too slow and calculating normals is fast even for big models
		std::cout << "Serializing mesh...\n";
		std::vector<unsigned int> buffer = mesh->serialize_to_v1_format();
		unsigned int uncompressedSizeInBytes = 4 * buffer.size();
		
		std::cout << "Compressing " << uncompressedSizeInBytes << " bytes... \n";
		std::vector<unsigned char> compressed_buffer = compress_buffer(buffer);

		std::cout << "Writing " << compressed_buffer.size() << " bytes as binary...\n";
		std::ofstream out(path, std::ios::binary);
		

		unsigned char uncompressedSize_b0 = uncompressedSizeInBytes & 0b11111111;
		unsigned char uncompressedSize_b1 = (uncompressedSizeInBytes >> 8) & 0b11111111;
		unsigned char uncompressedSize_b2 = (uncompressedSizeInBytes >> 16) & 0b11111111;
		unsigned char uncompressedSize_b3 = (uncompressedSizeInBytes >> 24) & 0b11111111;
		// append size at the beginning
		out.write(reinterpret_cast<char*>(&(uncompressedSize_b3)), 1);
		out.write(reinterpret_cast<char*>(&(uncompressedSize_b2)), 1);
		out.write(reinterpret_cast<char*>(&(uncompressedSize_b1)), 1);
		out.write(reinterpret_cast<char*>(&(uncompressedSize_b0)), 1);

		for (int i = 0; i<compressed_buffer.size(); i++ ) {
			out.write(reinterpret_cast<char*>(&compressed_buffer[i]), 1);
		}
		out.close();
	};

	Mesh* Serializer::readMeshFromBinaryFile(const std::string& path) {
		std::ifstream in(path, std::ios::binary);
		std::vector<unsigned char> compressed_buffer;
		unsigned char c;
		unsigned int uncompressedSize = 0;
		int bytesRead = 0;
		while(in.read(reinterpret_cast<char*>(&c), 1)) {
			if (bytesRead >= sizeof(uncompressedSize)) {
				compressed_buffer.push_back(c);
			} else {
				uncompressedSize <<= 8;
				uncompressedSize |= c;
			}
			bytesRead++;
		};
		in.close();
		std::bitset<32> b_uncompressedSize(uncompressedSize);
		std::cout << "Uncompressed size read: " << uncompressedSize << " 0b(" << b_uncompressedSize << ")\n"; 
		std::vector<unsigned int> buffer = decompress_buffer(compressed_buffer, uncompressedSize);

		Mesh* mesh = new Mesh();
		std::cout << "Uncompressed bytes count: " << 4 * buffer.size() << "\n";

		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;

		int ptr = 0;
		try {
			unsigned int vertexBlockStart = 1;
			unsigned int vertexCount = buffer[ptr];
			std::cout << "Vertex count:" << vertexCount << "\n";
			ptr++;
			while (ptr < vertexBlockStart + 3 * vertexCount) {
				vertices.push_back({
					std::bit_cast<float>(buffer[ptr]), 
					std::bit_cast<float>(buffer[ptr+1]), 
					std::bit_cast<float>(buffer[ptr+2])
				});
				ptr += 3;
			}

			unsigned int normalBlockStart = ptr+1;
			unsigned int normalCount = buffer[ptr];
			ptr++;

			std::cout << "Normal count:" << normalCount << "\n";
			while (ptr < normalBlockStart + 3 * normalCount) {
				normals.push_back({
					std::bit_cast<float>(buffer[ptr]), 
					std::bit_cast<float>(buffer[ptr+1]), 
					std::bit_cast<float>(buffer[ptr+2])
				});
				ptr += 3;
			}

			unsigned int trigBlockStart = ptr+1;
			unsigned int trigCount = buffer[ptr];

			std::cout << "Trig count:" << trigCount << "\n";

			ptr++;
			while (ptr < trigBlockStart + 6 * trigCount) {
				int v1 = std::bit_cast<unsigned int>(buffer[ptr]);
				int n1 = std::bit_cast<unsigned int>(buffer[ptr+1]);
				
				int v2 = std::bit_cast<unsigned int>(buffer[ptr+2]);
				int n2 = std::bit_cast<unsigned int>(buffer[ptr+3]);
				
				int v3 = std::bit_cast<unsigned int>(buffer[ptr+4]);
				int n3 = std::bit_cast<unsigned int>(buffer[ptr+5]);

				glm::vec3 verts[3] = {
					vertices[v1], vertices[v2], vertices[v3]
				};
				glm::vec3 norms[3] = {
					normals[n1], normals[n2], normals[n3]
				};

				mesh->push_triangle_with_normals(verts, norms);
				ptr += 6;
			}
		} catch(std::exception& e) {
			std::cout << "Error at ptr: " << ptr << "\n";
		}

		return mesh;
	};
}