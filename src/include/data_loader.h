#pragma once

#include <vector>
#include <filesystem>
#include <string>
#include <fstream>

class CData_Loader {
public:
    static const std::size_t s_do_not_chunk = 0;
    explicit CData_Loader(const std::filesystem::path& path, const std::size_t chunk_size = s_do_not_chunk);
    bool Has_Error() const;
    bool Has_Next_Chunk() const;
    const std::vector<std::wstring>& Get_Errors() const;
    std::size_t Load_Chunk(std::vector<double>& buffer);
private:
    std::ifstream m_input_stream;
    std::size_t m_chunk_size;
    std::vector<std::wstring> m_errors;
    std::size_t m_file_size = 0;
    std::vector<double> m_valid_backup;

    void Load_Valid_Backup();
};

std::vector<double> load_data(const std::filesystem::path& path);

std::string load_text_file(const std::filesystem::path& path);