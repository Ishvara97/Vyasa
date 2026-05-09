#include "export_paths.h"

#include <filesystem>

namespace {
const std::string kExportRoot = "HymnExports";
const std::string kJsonFolder = "HymnExports/json";
const std::string kTabularFolder = "HymnExports/tabular";
const std::string kAnalysisFolder = "HymnExports/analysis";
const std::string kSandhiFolder = "HymnExports/sandhi";
const std::string kSimilarityFolder = "HymnExports/similarity";
const std::string kMatrixFolder = "HymnExports/matrices";
}

void ensureExportDirectories() {
    std::filesystem::create_directories(kExportRoot);
    std::filesystem::create_directories(kJsonFolder);
    std::filesystem::create_directories(kTabularFolder);
    std::filesystem::create_directories(kAnalysisFolder);
    std::filesystem::create_directories(kSandhiFolder);
    std::filesystem::create_directories(kSimilarityFolder);
    std::filesystem::create_directories(kMatrixFolder);
}

HymnExportPaths buildHymnExportPaths(const std::string& exportBaseName) {
    return {
        kJsonFolder + "/" + exportBaseName + ".json",
        kTabularFolder + "/" + exportBaseName + ".csv",
        kAnalysisFolder + "/" + exportBaseName + "_Analysis.csv",
        kSandhiFolder + "/" + exportBaseName + "_Sandhi.csv",
        kSimilarityFolder + "/" + exportBaseName + "_Similarity.csv"
    };
}

std::string buildMatrixExportPath(const std::string& exportBaseName, const std::string& analysisName) {
    return kMatrixFolder + "/" + exportBaseName + "_" + analysisName + ".csv";
}
