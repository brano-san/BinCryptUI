#include "BinCryptor.h"
#include <iostream>
#include <Windows.h>
#include <cstddef>

// File class
crypt::File::File(const std::string& path, std::ios_base::openmode mode)
{
    this->open(path, mode);
}

std::string crypt::File::getPath()
{
    return _path.string();
}

std::string crypt::File::getFileName()
{
    return _path.filename().string();
}

void crypt::File::open(const std::string& path, std::ios_base::openmode mode)
{
    if (_file.is_open())
        _file.close();

    _path = path;

    _file.open(_path, mode);
}

void crypt::File::close()
{
    _file.close();
}

std::string crypt::File::read()
{
    if (!_file.is_open())
        return std::string();

    std::string data;
    std::string line;
    while (std::getline(_file, line)) {
        data += line;
    }

    return data;
}

void crypt::File::write(const std::string& str)
{
    _file.write(str.c_str(), str.size());
}

// Crypt functions
crypt::Cryptor::Cryptor(FileSavingStrategy* strategy)
    : _saver(FileSaver(strategy))
{	}

void crypt::Cryptor::setFileSaverStrategy(FileSavingStrategy* strategy)
{
    _saver.setStrategy(strategy);
}

void crypt::Cryptor::cryptFile(File* file,
                               const std::array<std::byte, 8>& keyBytes,
                               const std::string& outputPath)
{
    auto data = file->read();

    std::string result;
    result.resize(data.size());

    for (int i = 0; i < data.size(); ++i)
    {
        auto res = static_cast<std::byte>(data[i]) ^ keyBytes[i % 8];

        result[i] = static_cast<char>(res);
    }

    _saver.saveToFile(outputPath + file->getFileName(), result);
}

void crypt::Cryptor::cryptFile(MaskedFileManager* files,
                               const std::array<std::byte, 8>& keyBytes,
                               const std::string& outputPath)
{
    for (auto& file : files->_files)
    {
        cryptFile(&file, keyBytes, outputPath);
    }
}

// MaskedFileManager class
crypt::MaskedFileManager::MaskedFileManager(const std::string& path, const std::string& mask)
{
    findFilesByMask(path, mask);
}

size_t crypt::MaskedFileManager::getFileCount() const
{
    return _files.size();
}

void crypt::MaskedFileManager::closeFiles()
{
    for (auto& file : _files)
    {
        file.close();
    }
}

void crypt::MaskedFileManager::deleteFiles()
{
    for (auto& file : _files)
    {
        file.close();
        std::remove(file.getPath().c_str());
    }
    _files.clear();
}

void crypt::MaskedFileManager::findFilesByMask(const std::string& path, const std::string& mask)
{
    WIN32_FIND_DATAA fileData{};

    std::string fullPath = path + mask;

    HANDLE hFind = FindFirstFileA(fullPath.c_str(), &fileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        return;
    }
    else
    {
        closeFiles();
        _files.clear();

        do
        {
            _files.emplace_back(path + fileData.cFileName, std::ios::binary | std::ios::in);
        } while (FindNextFileA(hFind, &fileData));

        FindClose(hFind);
    }
}

// Save strategy
crypt::FileSaver::FileSaver(FileSavingStrategy* strategy)
    : _strategy(strategy)
{	}

void crypt::FileSaver::setStrategy(FileSavingStrategy* strategy)
{
    _strategy = strategy;
}

void crypt::FileSaver::saveToFile(std::filesystem::path path,
                                  const std::string& data) const
{
    _strategy->saveToFile(path, data);
}

void crypt::OverwritingSavingStrategy::saveToFile(std::filesystem::path path, const std::string& data)
{
    std::string outputPath = path.parent_path().string() +
                             "/" + path.filename().string();

    File resultFile(outputPath, std::ios::out | std::ios::binary);
    resultFile.write(data);
}

void crypt::NewNameSavingStrategy::saveToFile(std::filesystem::path path, const std::string& data)
{
    unsigned int count = 1;
    std::string newFilePath = path.string();

    while (std::filesystem::exists(newFilePath))
    {
        newFilePath = path.parent_path().string() + "/" +
                      path.stem().string() + " (" +
                      std::to_string(count) + ")" +
                      path.extension().string();
        ++count;
    }

    File resultFile(newFilePath, std::ios::out | std::ios::binary);
    resultFile.write(data);
}

void crypt::WritingToEndSavingStrategy::saveToFile(std::filesystem::path path, const std::string& data)
{
    std::string outputPath = path.parent_path().string() +
                             "/" + path.filename().string();

    File resultFile(outputPath, std::ios::out | std::ios::app);
    resultFile.write(data);
}
