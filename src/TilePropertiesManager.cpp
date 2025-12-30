#include "TileProperties.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

TilePropertiesManager& TilePropertiesManager::getInstance() {
    static TilePropertiesManager instance;
    return instance;
}

CollisionType TilePropertiesManager::stringToCollisionType(const std::string& str) const {
    if (str == "none") return CollisionType::NONE;
    if (str == "solid") return CollisionType::SOLID;
    if (str == "empty") return CollisionType::EMPTY;
    if (str == "platform") return CollisionType::PLATFORM;
    if (str == "half-block") return CollisionType::HALF_BLOCK;
    if (str == "slope-left") return CollisionType::SLOPE_LEFT;
    if (str == "slope-right") return CollisionType::SLOPE_RIGHT;
    if (str == "ladder") return CollisionType::LADDER;
    if (str == "spikes") return CollisionType::SPIKES;
    if (str == "water") return CollisionType::WATER;
    if (str == "ice") return CollisionType::ICE;
    if (str == "wall-left") return CollisionType::WALL_LEFT;
    if (str == "wall-right") return CollisionType::WALL_RIGHT;
    if (str == "ceiling") return CollisionType::CEILING;
    if (str == "portal") return CollisionType::PORTAL;
    if (str == "ground-corner-left") return CollisionType::GROUND_CORNER_LEFT;
    if (str == "ground-corner-right") return CollisionType::GROUND_CORNER_RIGHT;
    if (str == "ground-under-corner-left") return CollisionType::GROUND_UNDER_CORNER_LEFT;
    if (str == "ground-under-corner-right") return CollisionType::GROUND_UNDER_CORNER_RIGHT;
    if (str == "wall-corner-top-left") return CollisionType::WALL_CORNER_TOP_LEFT;
    if (str == "wall-corner-top-right") return CollisionType::WALL_CORNER_TOP_RIGHT;
    if (str == "wall-corner-bottom-left") return CollisionType::WALL_CORNER_BOTTOM_LEFT;
    if (str == "wall-corner-bottom-right") return CollisionType::WALL_CORNER_BOTTOM_RIGHT;

    return CollisionType::NONE;
}

// Simple JSON parser for our specific structure
bool TilePropertiesManager::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open tile properties file: " << filepath << std::endl;
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    // Remove whitespace and newlines for easier parsing
    content.erase(std::remove_if(content.begin(), content.end(), ::isspace), content.end());

    // Skip opening brace
    size_t pos = content.find('{');
    if (pos == std::string::npos) return false;
    pos++;

    // Parse each tile entry
    while (pos < content.length()) {
        // Skip whitespace
        while (pos < content.length() && std::isspace(content[pos])) pos++;

        // Check for closing brace
        if (content[pos] == '}') break;

        // Parse tile ID (quoted number)
        if (content[pos] != '"') break;
        pos++;

        size_t idEnd = content.find('"', pos);
        if (idEnd == std::string::npos) break;

        int tileId = std::stoi(content.substr(pos, idEnd - pos));
        pos = idEnd + 1;

        // Skip colon
        while (pos < content.length() && content[pos] != ':') pos++;
        pos++;

        // Skip opening brace of properties
        while (pos < content.length() && content[pos] != '{') pos++;
        pos++;

        TileProperties props;

        // Parse properties
        while (pos < content.length() && content[pos] != '}') {
            // Parse property name
            if (content[pos] != '"') break;
            pos++;

            size_t nameEnd = content.find('"', pos);
            if (nameEnd == std::string::npos) break;

            std::string propName = content.substr(pos, nameEnd - pos);
            pos = nameEnd + 1;

            // Skip colon
            while (pos < content.length() && content[pos] != ':') pos++;
            pos++;

            // Parse value based on property name
            if (propName == "desc" || propName == "notes") {
                // String value
                if (content[pos] != '"') break;
                pos++;

                size_t valueEnd = content.find('"', pos);
                if (valueEnd == std::string::npos) break;

                std::string value = content.substr(pos, valueEnd - pos);
                if (propName == "desc") {
                    props.description = value;
                } else {
                    props.notes = value;
                }
                pos = valueEnd + 1;
            } else if (propName == "collision") {
                // String value for collision type
                if (content[pos] != '"') break;
                pos++;

                size_t valueEnd = content.find('"', pos);
                if (valueEnd == std::string::npos) break;

                std::string value = content.substr(pos, valueEnd - pos);
                props.collisionType = stringToCollisionType(value);
                pos = valueEnd + 1;
            } else if (propName == "grassDepth") {
                // Numeric value
                size_t valueEnd = pos;
                while (valueEnd < content.length() &&
                       (std::isdigit(content[valueEnd]) || content[valueEnd] == '.')) {
                    valueEnd++;
                }
                props.grassDepth = std::stof(content.substr(pos, valueEnd - pos));
                pos = valueEnd;
            } else if (propName == "damage") {
                // Numeric value
                size_t valueEnd = pos;
                while (valueEnd < content.length() && std::isdigit(content[valueEnd])) {
                    valueEnd++;
                }
                props.damage = std::stoi(content.substr(pos, valueEnd - pos));
                pos = valueEnd;
            } else if (propName == "portalDestination") {
                // String value
                if (content[pos] != '"') break;
                pos++;

                size_t valueEnd = content.find('"', pos);
                if (valueEnd == std::string::npos) break;

                props.portalDestination = content.substr(pos, valueEnd - pos);
                pos = valueEnd + 1;
            }

            // Skip comma if present
            while (pos < content.length() && (content[pos] == ',' || std::isspace(content[pos]))) pos++;
        }

        // Store the properties
        m_properties[tileId] = props;

        // Skip closing brace and comma
        while (pos < content.length() && (content[pos] == '}' || content[pos] == ',' || std::isspace(content[pos]))) pos++;
    }

    std::cout << "Loaded " << m_properties.size() << " tile properties from " << filepath << std::endl;
    return true;
}

const TileProperties* TilePropertiesManager::getTileProperties(int tileId) const {
    auto it = m_properties.find(tileId);
    if (it != m_properties.end()) {
        return &it->second;
    }
    return nullptr;
}

bool TilePropertiesManager::isSolid(int tileId) const {
    const TileProperties* props = getTileProperties(tileId);
    if (!props) return false;

    // All collision types except NONE and EMPTY are considered solid in some way
    return props->collisionType != CollisionType::NONE &&
           props->collisionType != CollisionType::EMPTY;
}

bool TilePropertiesManager::isPlatform(int tileId) const {
    const TileProperties* props = getTileProperties(tileId);
    if (!props) return false;

    return props->collisionType == CollisionType::PLATFORM;
}

bool TilePropertiesManager::isSlope(int tileId) const {
    const TileProperties* props = getTileProperties(tileId);
    if (!props) return false;

    return props->collisionType == CollisionType::SLOPE_LEFT ||
           props->collisionType == CollisionType::SLOPE_RIGHT;
}

CollisionType TilePropertiesManager::getCollisionType(int tileId) const {
    const TileProperties* props = getTileProperties(tileId);
    if (!props) return CollisionType::NONE;

    return props->collisionType;
}

float TilePropertiesManager::getGrassDepth(int tileId) const {
    const TileProperties* props = getTileProperties(tileId);
    if (!props) return 0.0f;

    return props->grassDepth;
}
