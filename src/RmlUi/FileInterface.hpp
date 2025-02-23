#ifndef FILEINTERFACE_HPP
#define FILEINTERFACE_HPP

// a helper to handle different platforms correctly (especially android) written with help of Gemini

#include <RmlUi/Core/FileInterface.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_log.h>
#include <map>
#include <memory> // For std::unique_ptr

class FileInterface : public Rml::FileInterface {
  public:
    FileInterface() = default;
    ~FileInterface() override = default;

    Rml::FileHandle Open(const Rml::String& path) override {
        SDL_IOStream* io_stream = SDL_IOFromFile(path.c_str(), "rb");
        if (!io_stream) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open file '%s': %s", path.c_str(), SDL_GetError());
            return 0; // Return 0 as invalid FileHandle
        }
        FileHandle id = next_file_handle_id_++;
        file_map_[id] = io_stream; // Store in map
        return id;                 // Return integer ID as FileHandle
    }

    void Close(Rml::FileHandle file) override {
        auto it = file_map_.find(file);
        if (it != file_map_.end()) {
            SDL_IOStream* io_stream = it->second;
            if (io_stream) {
                if (!SDL_CloseIO(io_stream)) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to close file: %s", SDL_GetError()); }
            }
            file_map_.erase(it); // Remove from map
        }
    }

    size_t Read(void* buffer, size_t size, Rml::FileHandle file) override {
        auto it = file_map_.find(file);
        if (it != file_map_.end()) {
            SDL_IOStream* io_stream = it->second;
            return SDL_ReadIO(io_stream, buffer, size);
        }
        return 0; // Or handle error, maybe log
    }

    bool Seek(Rml::FileHandle file, long offset, int origin) override {
        auto it = file_map_.find(file);
        if (it != file_map_.end()) {
            SDL_IOStream* io_stream = it->second;
            SDL_IOWhence whence;
            switch (origin) {
            case SEEK_SET: whence = SDL_IO_SEEK_SET; break;
            case SEEK_CUR: whence = SDL_IO_SEEK_CUR; break;
            case SEEK_END: whence = SDL_IO_SEEK_END; break;
            default: SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid seek origin: %d", origin); return false;
            }
            Sint64 result = SDL_SeekIO(io_stream, offset, whence);
            if (result == -1) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Seek failed: %s", SDL_GetError());
                return false;
            }
            return true;
        }
        return false; // Or handle error, maybe log
    }

    size_t Tell(Rml::FileHandle file) override {
        auto it = file_map_.find(file);
        if (it != file_map_.end()) {
            SDL_IOStream* io_stream = it->second;
            Sint64 position = SDL_TellIO(io_stream);
            if (position == -1) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Tell failed: %s", SDL_GetError());
                return 0;
            }
            return static_cast<size_t>(position);
        }
        return 0; // Or handle error, maybe log
    }

  private:
    using FileHandle = size_t; // Define FileHandle explicitly as size_t
    std::map<FileHandle, SDL_IOStream*> file_map_;
    FileHandle next_file_handle_id_ = 1; // Start from 1, 0 is reserved for invalid
};

#endif //FILEINTERFACE_HPP