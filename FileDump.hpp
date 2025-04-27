#ifndef CHOC_FILE_UTILS_HEADER_INCLUDED
#define CHOC_FILE_UTILS_HEADER_INCLUDED

#include <fstream>
#include <functional>
#include <cwctype>
#include <stdexcept>
#include <random>
#include <filesystem>
#include "UTF8.hpp"

namespace choc::file
{

/// A file handling error, thrown by some of the functions in this namespace.
struct Error  : public std::runtime_error
{
    Error (const std::string& error) : std::runtime_error (error) {}
};

/// Attempts to read the entire contents of the given file into memory,
/// throwing an Error exception if anything goes wrong. The lambda argument will be
/// called with the amount of space needed as a uint64_t parameter, and it must
/// return a pointer to a sufficiently large block of memory to write the data into
/// (or nullptr if it can't provide a suitable buffer, in which case nothing will be done).
/// Returns the number of bytes successfully read.
template <typename GetDestBufferFn>
size_t readFileContent (const std::string& filename, GetDestBufferFn&&);

/// Attempts to load the contents of the given filename into a string,
/// throwing an Error exception if anything goes wrong.
std::string loadFileAsString (const std::string& filename);

/// Attempts to create or overwrite the specified file with some new data.
/// This will attempt to create and parent folders needed for the file, and will
/// throw an Error exception if something goes wrong.
void replaceFileWithContent (const std::filesystem::path& file,
                             std::string_view newContent);

/// Attempts to create or overwrite the specified file with data from a stream.
/// This will attempt to create and parent folders needed for the file, and will
/// throw an Error exception if something goes wrong. If maxBytesToWrite is zero,
/// the stream will be read until it reaches eof.
void replaceFileWithContent (const std::filesystem::path& file,
                             std::istream& sourceStream,
                             size_t maxBytesToWrite = 0);

/// Sanitises a string to remove illegal chatacters and leave it suitable for use
/// as a filename. This is intended only for checking a filename, not a whole path,
/// so it will remove any slashes in the string.
std::string makeSafeFilename (std::string_view source, size_t maxLength = 80);


//==============================================================================
//        _        _           _  _
//     __| |  ___ | |_   __ _ (_)| | ___
//    / _` | / _ \| __| / _` || || |/ __|
//   | (_| ||  __/| |_ | (_| || || |\__ \ _  _  _
//    \__,_| \___| \__| \__,_||_||_||___/(_)(_)(_)
//
//   Code beyond this point is implementation detail...
//
//==============================================================================

template <typename GetDestBufferFn>
size_t readFileContent (const std::string& filename, GetDestBufferFn&& getBuffer)
{
    if (filename.empty())
        throw Error ("Illegal filename");

    try
    {
        std::ifstream stream;
        stream.exceptions (std::ofstream::failbit | std::ofstream::badbit);
        stream.open (filename, std::ios::binary | std::ios::ate);

        if (! stream.is_open())
        {
            if (! exists (std::filesystem::path (filename)))
                throw Error ("File does not exist: " + filename);

            throw Error ("Failed to open file: " + filename);
        }

        auto fileSize = stream.tellg();

        if (fileSize < 0)
            throw Error ("Failed to read from file: " + filename);

        if (fileSize == 0)
            return {};

        if (auto destBuffer = getBuffer (static_cast<uint64_t> (fileSize)))
        {
            stream.seekg (0);

            if (stream.read (static_cast<std::ifstream::char_type*> (destBuffer), static_cast<std::streamsize> (fileSize)))
                return static_cast<size_t> (fileSize);

            throw Error ("Failed to read from file: " + filename);
        }
    }
    catch (const std::ios_base::failure& e)
    {
        throw Error ("Failed to read from file: " + filename + ": " + e.what());
    }

    return 0;
}

inline std::string loadFileAsString (const std::string& filename)
{
    std::string result;

    readFileContent (filename, [&] (uint64_t size)
    {
        result.resize (static_cast<std::string::size_type> (size));
        return result.data();
    });

    return result;
}

template <typename WriteFn>
void createAndWriteToFile (const std::filesystem::path& path, WriteFn&& write)
{
    try
    {
        try
        {
            if (path.has_parent_path())
                if (auto parent = path.parent_path(); ! exists (parent))
                    create_directories (parent);
        }
        catch (const std::ios_base::failure&) {}

        std::ofstream stream;
        stream.exceptions (std::ofstream::failbit | std::ofstream::badbit);
        stream.open (path.string(), std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
        write (stream);
    }
    catch (const std::ios_base::failure& e)
    {
        throw Error ("Failed to write to file: " + path.string() + ": " + e.what());
    }
    catch (...)
    {
        throw Error ("Failed to write to file: " + path.string());
    }
}

inline size_t attemptToRead (std::istream& source, std::istream::char_type* buffer, size_t size)
{
    size_t numRead = 0;

    while (size != 0)
    {
        try
        {
            source.read (buffer, static_cast<std::streamsize> (size));
        }
        catch (...) {}

        if (auto numDone = static_cast<size_t> (source.gcount()))
        {
            numRead += numDone;

            if (numDone != size)
            {
                buffer += numDone;
                size -= numDone;
                continue;
            }
        }

        break;
    }

    return numRead;
}

inline void replaceFileWithContent (const std::filesystem::path& path, std::string_view newContent)
{
    createAndWriteToFile (path, [=] (std::ofstream& stream)
    {
        stream.write (newContent.data(), static_cast<std::streamsize> (newContent.size()));
    });
}

inline void replaceFileWithContent (const std::filesystem::path& path, std::istream& source, size_t maxBytes)
{
    createAndWriteToFile (path, [&] (std::ofstream& stream)
    {
        std::istream::char_type buffer[8192];

        for (;;)
        {
            auto numToRead = maxBytes != 0 ? std::min (maxBytes, sizeof(buffer))
                                           : sizeof(buffer);

            auto numRead = attemptToRead (source, buffer, numToRead);

            if (numRead == 0)
                return;

            stream.write (buffer, static_cast<std::streamsize> (numRead));
        }
    });
}

inline std::string makeSafeFilename (std::string_view source, size_t maxLength)
{
    std::string name;
    name.reserve (source.length());

    for (auto c : source)
        if (std::string_view (",;#@*^|?:<>\"\\/").find (c) == std::string_view::npos)
            name += c;

    if (name.length() >= maxLength)
    {
        auto stem = std::filesystem::path (name).stem().string();
        auto extension = std::filesystem::path (name).extension().string();
        return stem.substr (0, maxLength - std::min (maxLength - 2, extension.length())) + extension;
    }

    if (name.empty())
        return "_";

    return name;
}

} // namespace choc::file

#endif // CHOC_FILE_UTILS_HEADER_INCLUDED
