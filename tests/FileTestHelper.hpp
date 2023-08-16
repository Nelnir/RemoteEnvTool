#include <vector>
#include <fstream>
#include <filesystem>

class FileTestHelper {
public:
    ~FileTestHelper() {
        // Destructor: Delete all created directories in reverse order
        for (auto it = createdFiles.rbegin(); it != createdFiles.rend(); ++it) {
            std::filesystem::remove(*it);
        }
    }

    void createFile(const std::string &path, const std::string &content = "") {
        // Extract the directory path from the provided path and create directories
        std::filesystem::path fsPath(path);
        if (fsPath.has_parent_path()) {
            createDirectories(fsPath.parent_path());
        }

        // Now, create or overwrite the file with the given content
        std::ofstream out(path, std::ios::out);
        if (out) {
            out << content;
            createdFiles.push_back(path);
            out.close();
        }
    }

    void createDirectory(const std::string &dirPath) {
        std::filesystem::path fsPath(dirPath);
        if (!std::filesystem::exists(fsPath)) {
            std::filesystem::create_directory(fsPath);
            createdFiles.push_back(dirPath);
        }
    }

    void updateFile(const std::string &path, const std::string &newContent) {
        std::ofstream out(path, std::ios::app);  // Appending mode
        if (out) {
            out << newContent;
            out.close();
        }
    }

    void deleteFile(const std::string &path) {
        std::filesystem::remove(path);
    }
    
    bool exists(const std::string& path){
        return std::filesystem::exists(path);
    }

private:
    void createDirectories(const std::filesystem::path &dirPath) {
        std::filesystem::path currentPath;
        for (const auto &part : dirPath) {
            currentPath /= part;
            if (!std::filesystem::exists(currentPath)) {
                std::filesystem::create_directory(currentPath);
                createdFiles.push_back(currentPath.string());
            }
        }
    }

    std::vector<std::string> createdFiles;
};