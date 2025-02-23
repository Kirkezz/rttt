#ifndef UTILS_HPP
#define UTILS_HPP

#include <RmlUi/Core.h>
#include <SDL3/SDL_iostream.h>

using namespace rml;

Span<const byte> LoadTTFFileToSpan(const String& filename) {
    SDL_IOStream* io = SDL_IOFromFile(filename.c_str(), "rb");
    if (!io) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_IOFromFile failed: %s", SDL_GetError());
        return Span<const byte>();
    }

    size_t data_size = 0;
    void* data = SDL_LoadFile_IO(io, &data_size, SDL_TRUE);
    if (!data) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_LoadFile_IO failed: %s", SDL_GetError());
        return Span<const byte>(); // Return empty span on failure
    }

    return Span<const byte>(static_cast<const byte*>(data), data_size);
}

#endif //UTILS_HPP
