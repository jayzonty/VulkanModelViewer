#include "IO/FileIO.hpp"

#include <fstream>

namespace FileIO
{
    /**
     * @brief Read the specified file as binary.
     * @param[in] filePath File path
     * @param[out] outFileContents Vector where the file contents will be placed. If the vector is not empty, the contents will be overwritten.
     * @return Returns true if the file was successfully read. Returns false otherwise.
     */
    bool ReadFileAsBinary(const std::string& filePath, std::vector<char>& outFileContents)
    {
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);
        if (file.fail())
        {
            return false;
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        outFileContents.resize(fileSize);

        file.seekg(0);
        file.read(outFileContents.data(), fileSize);
        file.close();

        return true;
    }
}

