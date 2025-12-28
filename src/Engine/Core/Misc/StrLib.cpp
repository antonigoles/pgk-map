#include <Engine/Core/Misc/StrLib.hpp>
#include <deque>

namespace Engine {
	unsigned int count_char_in_string(const std::string& str, const char& c) {
        unsigned int ctr = 0;
        for (auto& e : str) 
            if (e == c) ctr++;
        return ctr;
    };

    void split_string(const std::string& str, const char& delimiter, std::vector<std::string>& out) {
        std::string parsed = "";
        for (int i = 0; i<str.size(); i++) {
            if (str[i] == delimiter) {
                if (parsed.size() > 0) {
                    out.push_back(parsed);
                    parsed = "";
                }
                continue;
            }
            parsed += str[i];
        }
        if (parsed.size() > 0) out.push_back(parsed);
    };

    bool isStringFloat(const std::string& str) {
        // very simple check
        for (auto& c : str) {
            if ((c - '0' < 0 || c - '0' > 9) && (c != '.')) return false;
        }
        return true;
    }

    std::string join_string(const std::vector<std::string>& str, const std::string& connector) {
        std::string result="";
        for (int i = 0; i<str.size(); i++) {
            result += str[i];
            if (i != str.size()-1) result += connector;
        }
        return result;
    };

    float parseInt(const std::vector<std::string>& tokens, int offset) {
        return std::stoi(tokens[offset]);
    };

    float parseFloat(const std::vector<std::string>& tokens, int offset) {
        return std::stof(tokens[offset]);
    };

    glm::vec3 parseVec3(const std::vector<std::string>& tokens, int offset) {
        return { 
            std::stof(tokens[offset]),
            std::stof(tokens[offset+1]),
            std::stof(tokens[offset+2])
        };
    };

    std::vector<float> parseVectorFlag(
        const std::vector<std::string>& tokens, 
        std::string flag, 
        std::vector<float> d_value
    ) {
        int found = -1;
        for (int i = 0; i<tokens.size(); i++) {
            if (tokens[i] == flag) {
                found = i;
                break;
            }
        }
        if (found == -1) return d_value;
        std::vector<float> solution;
        while (found+1 < tokens.size() && isStringFloat(tokens[found+1])) {
            found++;
            solution.push_back(std::stof(tokens[found]));
        }
        return solution;
    };

    std::string replace_variable(const std::string& str, const std::string& variable, const std::string& value) {
        std::string newString = "";
        bool isReadingBlock = false;
        std::string block = "";
        for (int i = 0; i<str.size(); i++) {
            if (!isReadingBlock && str[i] == '{') {
                isReadingBlock = true;
                continue;
            }
            if (isReadingBlock && str[i] == '{') {
                throw "Incorrect character at index " + std::to_string(i) + " : " + str;
            }
            if (isReadingBlock && str[i] == '}') {
                if (block == variable) {
                    newString += value; 
                } else {
                    newString += "{" + block + "}";
                }
                block = "";
                isReadingBlock = false;
                continue;
            }
            if (isReadingBlock) {
                block += str[i];
            } else {
                newString += str[i];
            }
        }
        return newString;
    };
}