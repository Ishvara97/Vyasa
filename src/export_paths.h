#ifndef EXPORT_PATHS_H
#define EXPORT_PATHS_H

#include <string>

struct HymnExportPaths {
    std::string jsonPath;
    std::string csvPath;
    std::string analysisPath;
    std::string sandhiPath;
    std::string similarityPath;
};

void ensureExportDirectories();
HymnExportPaths buildHymnExportPaths(const std::string& exportBaseName);
std::string buildMatrixExportPath(const std::string& exportBaseName, const std::string& analysisName);

#endif
