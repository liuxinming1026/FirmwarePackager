#pragma once

#include <string>
#include "ProjectModel.h"

namespace core {

class ProjectSerializer {
public:
    Project load(const std::string& filePath) const;
    void save(const Project& project, const std::string& filePath) const;
};

} // namespace core

