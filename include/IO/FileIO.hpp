#pragma once

#include <string>
#include <vector>

namespace FileIO
{
    /**
     * @brief Read the specified file as binary.
     * @param[in] filePath File path
     * @param[out] outFileContents Vector where the file contents will be placed
     * @return Returns true if the file was successfully read. Returns false otherwise.
     */
    extern bool ReadFileAsBinary(const std::string& filePath, std::vector<char>& outFileContents);
}

