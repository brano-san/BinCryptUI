#pragma once

#include <fstream>
#include <functional>
#include <filesystem>
#include <array>
#include <vector>
#include <Windows.h>

namespace crypt
{
class File;
class Cryptor;
class FileSaver;
class MaskedFileManager;

class File
{
public:
    File() = default;
    File(const std::string& path,
         std::ios_base::openmode mode = std::ios::in | std::ios::out);

public:
    std::string getPath();
    std::string getFileName();

    void open(const std::string& path,
              std::ios_base::openmode mode = std::ios::in | std::ios::out);
    void close();

    std::string read();
    void write(const std::string& str);

private:
    std::fstream _file;
    std::filesystem::path _path;
};

class FileSavingStrategy {
public:
    virtual ~FileSavingStrategy() = default;
    virtual void saveToFile(std::filesystem::path path, const std::string& data) = 0;
};

class OverwritingSavingStrategy : public FileSavingStrategy {
public:
    void saveToFile(std::filesystem::path path, const std::string& data) override;
};

class NewNameSavingStrategy : public FileSavingStrategy {
public:
    void saveToFile(std::filesystem::path path, const std::string& data) override;
};

class WritingToEndSavingStrategy : public FileSavingStrategy {
public:
    void saveToFile(std::filesystem::path path, const std::string& data) override;
};

class FileSaver {
public:
    FileSaver() = default;
    FileSaver(FileSavingStrategy* strategy);

public:
    void setStrategy(FileSavingStrategy* strategy);

    void saveToFile(std::filesystem::path path, const std::string& data) const;

private:
    FileSavingStrategy* _strategy;
};

class Cryptor
{
public:
    Cryptor() = default;
    Cryptor(FileSavingStrategy* strategy);

public:
    void setFileSaverStrategy(FileSavingStrategy* strategy);

    void cryptFile(File* file,
                   const std::array<std::byte, 8>& keyBytes,
                   const std::string& outputPath);

    void cryptFile(MaskedFileManager* files,
                   const std::array<std::byte, 8>& keyBytes,
                   const std::string& outputPath);

private:
    FileSaver _saver;
};

class MaskedFileManager
{
public:
    MaskedFileManager() = default;
    explicit MaskedFileManager(const std::string& path,
                               const std::string& mask);

    ~MaskedFileManager() = default;

public:
    size_t getFileCount() const;

    void closeFiles();
    void deleteFiles();

    void findFilesByMask(const std::string& path,
                         const std::string& mask);

private:
    friend void Cryptor::cryptFile(MaskedFileManager* files,
                                   const std::array<std::byte, 8>& keyBytes,
                                   const std::string& outputPath);

private:
    std::vector<File> _files;
};
}
