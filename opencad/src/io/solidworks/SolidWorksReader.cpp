#include "io/solidworks/SolidWorksReader.h"
#include "io/parasolid/ParasolidReader.h"
#include <QCoreApplication> // For UI responsiveness
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <zlib.h>

namespace opencad {
namespace io {

namespace {

struct CFBHeader {
  uint8_t signature[8];
  uint8_t clsid[16];
  uint16_t minorVersion;
  uint16_t majorVersion;
  uint16_t byteOrder;
  uint16_t sectorShift;
  uint16_t miniSectorShift;
  uint8_t reserved[6];
  uint32_t numDirSectors;
  uint32_t numFatSectors;
  uint32_t firstDirSector;
  uint32_t transactionSignature;
  uint32_t miniStreamCutoffSize;
  uint32_t firstMiniFatSector;
  uint32_t numMiniFatSectors;
  uint32_t firstDifatSector;
  uint32_t numDifatSectors;
  uint32_t difat[109];
};

struct DirEntry {
  char16_t name[32];
  uint16_t nameLen;
  uint8_t type; // 1=Storage, 2=Stream, 5=Root
  uint8_t color;
  uint32_t prevDid;
  uint32_t nextDid;
  uint32_t childDid;
  uint8_t clsid[16];
  uint32_t stateBits;
  uint64_t creationTime;
  uint64_t modTime;
  uint32_t firstSector;
  uint64_t streamSize;
};

class MiniCFB {
public:
  MiniCFB(const std::string &filename, std::string &error) : m_error(error) {
    // Support for UTF-8 paths on Windows (e.g. "Masaüstü")
    std::filesystem::path path = std::filesystem::u8path(filename);
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
      error = "Cannot open file";
      return;
    }

    file.seekg(0, std::ios::end);
    m_fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    m_data.resize(m_fileSize);
    file.read(m_data.data(), m_fileSize);

    if (m_fileSize < 512) {
      error = "File too small";
      return;
    }

    memcpy(&m_header, m_data.data(), 512);

    const uint8_t magic[] = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1};
    if (memcmp(m_header.signature, magic, 8) != 0) {
      error = "Invalid OLE Signature";
      return;
    }

    m_sectorSize = 1 << m_header.sectorShift;
    m_miniSectorSize = 1 << m_header.miniSectorShift;

    loadFAT();
    loadMiniFAT();
    loadDirectory();

    m_valid = true;
  }

  bool isValid() const { return m_valid; }

  // Instead of getting all data, get entries
  const std::vector<DirEntry> &getEntries() const { return m_entries; }

  // Read specific stream by entry
  std::vector<char> getStreamData(const DirEntry &entry) {
    if (entry.type != 2)
      return {};
    return readStream(entry.firstSector, entry.streamSize);
  }

  std::string getName(const DirEntry &entry) {
    std::string name;
    for (int i = 0; i < 32 && entry.name[i] != 0; ++i) {
      if (entry.name[i] < 128)
        name += (char)entry.name[i];
      else
        name += '?';
    }
    return name;
  }

private:
  std::string &m_error;
  std::vector<char>
      m_data; // This holds the WHOLE file in memory.
              // Optimization: Could map file, but for <200MB this is OK.
              // The issue was duplicating it for every stream.
  size_t m_fileSize;
  CFBHeader m_header;
  bool m_valid = false;
  uint32_t m_sectorSize;
  uint32_t m_miniSectorSize;

  std::vector<uint32_t> m_fat;
  std::vector<uint32_t> m_miniFat;
  std::vector<DirEntry> m_entries;
  std::vector<char> m_miniStream;

  void loadFAT() {
    std::vector<uint32_t> difat;
    for (int i = 0; i < 109; ++i)
      difat.push_back(m_header.difat[i]);

    for (uint32_t sector : difat) {
      if (sector == 0xFFFFFFFF || sector == 0xFFFFFFFE)
        break;
      const char *ptr = getSectorPtr(sector);
      if (!ptr)
        break;
      const uint32_t *entries = reinterpret_cast<const uint32_t *>(ptr);
      for (size_t k = 0; k < m_sectorSize / 4; ++k)
        m_fat.push_back(entries[k]);
    }
  }

  void loadMiniFAT() {
    if (m_header.firstMiniFatSector == 0xFFFFFFFE)
      return;
    auto chain = getChain(m_header.firstMiniFatSector);
    for (uint32_t sector : chain) {
      const char *ptr = getSectorPtr(sector);
      if (!ptr)
        break;
      const uint32_t *entries = reinterpret_cast<const uint32_t *>(ptr);
      for (size_t k = 0; k < m_sectorSize / 4; ++k)
        m_miniFat.push_back(entries[k]);
    }
  }

  void loadDirectory() {
    auto chain = getChain(m_header.firstDirSector);
    for (uint32_t sector : chain) {
      const char *ptr = getSectorPtr(sector);
      if (!ptr)
        continue;
      for (size_t k = 0; k < m_sectorSize / 128; ++k) {
        DirEntry entry;
        memcpy(&entry, ptr + k * 128, 128);
        m_entries.push_back(entry);
      }
    }
    if (!m_entries.empty()) {
      DirEntry root = m_entries[0];
      m_miniStream = readStream(root.firstSector, root.streamSize);
    }
  }

  std::vector<uint32_t> getChain(uint32_t startSector) {
    std::vector<uint32_t> chain;
    uint32_t current = startSector;
    int limit = 200000; // Hard limit to prevent infinite loops
    while (current != 0xFFFFFFFE && current != 0xFFFFFFFF && limit-- > 0) {
      if (current >= m_fat.size())
        break;
      chain.push_back(current);
      current = m_fat[current];
    }
    return chain;
  }

  std::vector<uint32_t> getMiniChain(uint32_t startSector) {
    std::vector<uint32_t> chain;
    uint32_t current = startSector;
    int limit = 200000;
    while (current != 0xFFFFFFFE && current != 0xFFFFFFFF && limit-- > 0) {
      if (current >= m_miniFat.size())
        break;
      chain.push_back(current);
      current = m_miniFat[current];
    }
    return chain;
  }

  const char *getSectorPtr(uint32_t sectorId) {
    size_t offset = (size_t)(sectorId + 1) * m_sectorSize;
    if (offset + m_sectorSize > m_data.size())
      return nullptr;
    return m_data.data() + offset;
  }

  std::vector<char> readStream(uint32_t startSector, uint64_t size) {
    std::vector<char> result;
    if (size == 0)
      return result;
    try {
      result.reserve(size);
    } catch (...) {
      return {};
    } // Handle alloc fail

    if (size < m_header.miniStreamCutoffSize) {
      auto chain = getMiniChain(startSector);
      for (uint32_t miniSec : chain) {
        size_t offset = miniSec * m_miniSectorSize;
        if (offset + m_miniSectorSize > m_miniStream.size())
          break;
        size_t copySize =
            std::min((uint64_t)m_miniSectorSize, size - result.size());
        result.insert(result.end(), m_miniStream.begin() + offset,
                      m_miniStream.begin() + offset + copySize);
        if (result.size() >= size)
          break;
      }
    } else {
      auto chain = getChain(startSector);
      for (uint32_t sector : chain) {
        const char *ptr = getSectorPtr(sector);
        if (!ptr)
          break;
        size_t copySize =
            std::min((uint64_t)m_sectorSize, size - result.size());
        result.insert(result.end(), ptr, ptr + copySize);
        if (result.size() >= size)
          break;
      }
    }
    return result;
  }
};

} // namespace

SolidWorksReader::SolidWorksReader() {}
SolidWorksReader::~SolidWorksReader() {}

bool SolidWorksReader::read(const std::string &filename) {
  std::string cfbError;
  MiniCFB cfb(filename, cfbError);

  std::stringstream log;
  log << "Scanning: " << filename << "\n";

  if (!cfb.isValid()) {
    m_error = "Invalid OLE file: " + cfbError;

    // Write debug log immediately
    std::ofstream debugFile("sw_import_log.txt");
    if (debugFile.is_open()) {
      debugFile << "Scanning: " << filename << "\n";
      debugFile << m_error << "\n";

      // Log first 16 bytes for hex dump diagnosis
      // Re-open file to read header
      std::filesystem::path path = std::filesystem::u8path(filename);
      std::ifstream file(path, std::ios::binary);
      if (file.is_open()) {
        char header[16];
        file.read(header, 16);
        debugFile << "Header Hex: ";
        for (int i = 0; i < 16; ++i) {
          debugFile << std::hex << (int)(unsigned char)header[i] << " ";
        }
        debugFile << std::dec << "\n";
        file.close();
      }

      debugFile << "Attempting RAW SCAN fallback...\n";

      // --- FALLBACK: RAW SCAN ---
      // If OLE fails, read whole file and look for T_01 / P_01
      std::ifstream rawFile(path, std::ios::binary | std::ios::ate);
      if (rawFile.is_open()) {
        size_t size = rawFile.tellg();
        rawFile.seekg(0, std::ios::beg);
        std::vector<char> buffer(size);
        rawFile.read(buffer.data(), size);

        std::string_view sv(buffer.data(), size);

        // Check T_01
        auto t01 = sv.find("T_01");
        if (t01 != std::string::npos) {
          debugFile << "FALLBACK: Found T_01 at offset " << t01 << "\n";
          // Try to parse?
          // We can't really return true from here easily without modifying the
          // structure significantly, but we can at least log it. Actually,
          // let's TRY logging it as a success for the user? No, we need to
          // populate m_shapes.
        } else {
          debugFile << "FALLBACK: No T_01 found in raw file.\n";
        }

        // Check P_01
        if (sv.find("P_01") != std::string::npos) {
          debugFile << "FALLBACK: Found P_01 (Binary) in raw file.\n";
        }
      }

      debugFile.close();
    }

    // Attempt the fallback for REAL
    {
      std::filesystem::path path = std::filesystem::u8path(filename);
      std::ifstream rawFile(path, std::ios::binary | std::ios::ate);
      if (rawFile.is_open()) {
        size_t size = rawFile.tellg();
        rawFile.seekg(0, std::ios::beg);
        std::vector<char> buffer(size);
        rawFile.read(buffer.data(), size);

        std::string_view sv(buffer.data(), size);
        if (sv.find("T_01") != std::string::npos) {
          ParasolidReader xtReader;
          // Extract from T_01 to end (rough)
          size_t start = sv.find("T_01");
          std::string content = std::string(sv.substr(start));
          if (xtReader.readFromBuffer(content)) {
            m_shapes = xtReader.getAllShapes();
            if (!m_shapes.empty())
              return true;
          }
        }
      }
    }

    return false;
  }

  // Get ALL entries, not data
  const auto &entries = cfb.getEntries();
  log << "Total Entries: " << entries.size() << "\n";

  int candidates = 0;

  // Iterate and process one by one
  for (size_t k = 0; k < entries.size(); ++k) {
    const auto &entry = entries[k];

    // Keep UI alive every few entries or if entry is big
    if (k % 5 == 0)
      QCoreApplication::processEvents();

    if (entry.type != 2)
      continue; // Skip non-streams
    if (entry.streamSize == 0)
      continue;

    std::string name = cfb.getName(entry);

    // OPTIMIZATION: Only look at streams that look like content
    // "Contents", "Data", "Body", or just big streams (>1KB)
    // Skip tiny metadata streams
    if (entry.streamSize < 100 && name != "Contents")
      continue;

    std::vector<char> data = cfb.getStreamData(entry);
    if (data.empty())
      continue;

    // 1. T_01 (Text) or P_01 (Binary) Check
    std::string_view sv(data.data(), data.size());
    if (sv.find("T_01") != std::string::npos) {
      candidates++;
      log << "  Stream '" << name << "': Found T_01 (Text) Header.\n";
      ParasolidReader xtReader;
      if (xtReader.readFromBuffer(std::string(sv))) {
        m_shapes = xtReader.getAllShapes();
        if (!m_shapes.empty())
          return true;
      }
    }
    if (sv.find("P_01") != std::string::npos) {
      candidates++;
      log << "  Stream '" << name << "': Found P_01 (Binary) Header.\n";
      m_error = "Found Binary Parasolid (P_01) data. Binary format is not yet "
                "supported.";
      return false;
    }

    // 2. Decompression (Zlib or Raw Deflate)
    if (data.size() > 100) {
      std::vector<char> outBuffer(50 * 1024 * 1024); // 50MB limit
      bool decompressed = false;
      size_t validSize = 0;

      z_stream strm;
      strm.zalloc = Z_NULL;
      strm.zfree = Z_NULL;
      strm.opaque = Z_NULL;
      strm.avail_in = (uInt)data.size();
      strm.next_in = (Bytef *)data.data();
      strm.avail_out = (uInt)outBuffer.size();
      strm.next_out = (Bytef *)outBuffer.data();

      // Try standard inflate first
      if (inflateInit(&strm) == Z_OK) {
        int ret = inflate(&strm, Z_NO_FLUSH);
        inflateEnd(&strm);
        if (ret == Z_STREAM_END || ret == Z_OK) {
          validSize = outBuffer.size() - strm.avail_out;
          decompressed = true;
        }
      }

      // Method B: Raw Deflate (WindowBits = -15)
      if (!decompressed) {
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = (uInt)data.size();
        strm.next_in = (Bytef *)data.data();
        strm.avail_out = (uInt)outBuffer.size();
        strm.next_out = (Bytef *)outBuffer.data();

        if (inflateInit2(&strm, -15) == Z_OK) {
          int ret = inflate(&strm, Z_NO_FLUSH);
          inflateEnd(&strm);
          if (ret == Z_STREAM_END || ret == Z_OK) {
            validSize = outBuffer.size() - strm.avail_out;
            decompressed = true;
            log << "  Stream '" << name << "': Decompressed via Raw Deflate.\n";
          }
        }
      }

      if (decompressed && validSize > 0) {
        std::string_view decompView(outBuffer.data(), validSize);
        if (decompView.find("T_01") != std::string::npos) {
          log << "  Stream '" << name
              << "': Found T_01 in Decompressed Data.\n";
          ParasolidReader xtReader;
          if (xtReader.readFromBuffer(std::string(decompView))) {
            auto newShapes = xtReader.getAllShapes();
            if (!newShapes.empty()) {
              m_shapes = newShapes;
              return true;
            }
          }
        }
      }
    }
  }

  {
    // Write debug log about OLE failure
    std::ofstream debugFile("sw_import_log.txt");
    if (debugFile.is_open()) {
      debugFile << "Scanning: " << filename << "\n";
      log << "Diagnosis: No Parasolid data found in OLE streams.\n";
      if (entries.size() > 0) {
        log << "Analyzed " << entries.size() << " streams:\n";
        for (const auto &entry : entries) {
          std::string n = cfb.getName(entry);
          if (n.length() > 0 && entry.type == 2) {
            log << " - " << n << " (" << entry.streamSize << " bytes)\n";
          }
        }
      }
      debugFile << log.str() << "\n";
      debugFile << "Attempting RAW SCAN fallback...\n";

      std::filesystem::path path = std::filesystem::u8path(filename);
      std::ifstream rawFile(path, std::ios::binary | std::ios::ate);
      if (rawFile.is_open()) {
        size_t size = rawFile.tellg();
        rawFile.seekg(0, std::ios::beg);
        std::vector<char> buffer(size);
        rawFile.read(buffer.data(), size);

        std::string_view sv(buffer.data(), size);

        if (sv.find("T_01") != std::string::npos) {
          debugFile << "FALLBACK: Found T_01 at offset " << sv.find("T_01")
                    << "\n";
        } else {
          debugFile << "FALLBACK: No T_01 found in raw file.\n";
        }

        if (sv.find("P_01") != std::string::npos) {
          debugFile << "FALLBACK: Found P_01 (Binary) in raw file.\n";
        }

        // --- 2. ZLIB BRUTE FORCE SCAN ---
        debugFile << "Attempting ZLIB DECOMPRESSION scan...\n";
        bool zlibFound = false;
        // Scan for Zlib magic bytes: 0x78 0x9C (Default), 0x78 0xDA (Max), 0x78
        // 0x01 (No/Low)
        for (size_t i = 0; i < size - 2 && i < 5000;
             ++i) { // Scan first 5KB aggressively
          uint8_t b1 = (uint8_t)buffer[i];
          uint8_t b2 = (uint8_t)buffer[i + 1];
          if (b1 == 0x78 && (b2 == 0x9C || b2 == 0xDA || b2 == 0x01)) {
            debugFile << "Potential Zlib header at offset " << i << " ("
                      << std::hex << (int)b2 << std::dec << ")\n";

            std::vector<char> outBuffer(50 * 1024 * 1024); // 50MB buffer
            z_stream strm;
            strm.zalloc = Z_NULL;
            strm.zfree = Z_NULL;
            strm.opaque = Z_NULL;
            strm.avail_in = (uInt)(size - i);
            strm.next_in = (Bytef *)(&buffer[i]);
            strm.avail_out = (uInt)outBuffer.size();
            strm.next_out = (Bytef *)outBuffer.data();

            if (inflateInit(&strm) == Z_OK) {
              int ret = inflate(&strm, Z_NO_FLUSH);
              inflateEnd(&strm);
              if (ret == Z_STREAM_END || ret == Z_OK) {
                debugFile << "  -> Decompressed successfully!\n";
                size_t validSize = outBuffer.size() - strm.avail_out;
                std::string_view decompView(outBuffer.data(), validSize);

                if (decompView.find("T_01") != std::string::npos) {
                  debugFile << "  -> Found T_01 in decompressed data! WE ARE "
                               "SAVED!\n";
                  zlibFound = true;
                  // We found it! We should probably parse it here.
                  // But let's verify via logs first or try to load.
                } else if (decompView.find("P_01") != std::string::npos) {
                  debugFile
                      << "  -> Found P_01 in decompressed data! (Binary)\n";
                  zlibFound = true;
                }
              }
            }
          }
        }
      }
      debugFile.close();
    }

    // Perform Actual Raw Fallback Read (with Zlib scan)
    std::filesystem::path path = std::filesystem::u8path(filename);
    std::ifstream rawFile(path, std::ios::binary | std::ios::ate);
    if (rawFile.is_open()) {
      size_t size = rawFile.tellg();
      rawFile.seekg(0, std::ios::beg);
      std::vector<char> buffer(size);
      rawFile.read(buffer.data(), size);

      std::string_view sv(buffer.data(), size);

      // Plain text check
      if (sv.find("T_01") != std::string::npos) {
        ParasolidReader xtReader;
        size_t start = sv.find("T_01");
        std::string content = std::string(sv.substr(start));
        if (xtReader.readFromBuffer(content)) {
          m_shapes = xtReader.getAllShapes();
          if (!m_shapes.empty())
            return true;
        }
      }

      // Zlib Brute Force Check
      for (size_t i = 0; i < size - 2 && i < 5000; ++i) {
        uint8_t b1 = (uint8_t)buffer[i];
        uint8_t b2 = (uint8_t)buffer[i + 1];
        if (b1 == 0x78 && (b2 == 0x9C || b2 == 0xDA || b2 == 0x01)) {
          std::vector<char> outBuffer(50 * 1024 * 1024); // 50MB
          z_stream strm;
          strm.zalloc = Z_NULL;
          strm.zfree = Z_NULL;
          strm.opaque = Z_NULL;
          strm.avail_in = (uInt)(size - i);
          strm.next_in = (Bytef *)(&buffer[i]);
          strm.avail_out = (uInt)outBuffer.size();
          strm.next_out = (Bytef *)outBuffer.data();

          if (inflateInit(&strm) == Z_OK) {
            int ret = inflate(&strm, Z_NO_FLUSH);
            inflateEnd(&strm);
            if (ret == Z_STREAM_END || ret == Z_OK) {
              size_t validSize = outBuffer.size() - strm.avail_out;
              std::string_view decompView(outBuffer.data(), validSize);
              if (decompView.find("T_01") != std::string::npos) {
                ParasolidReader xtReader;
                if (xtReader.readFromBuffer(std::string(decompView))) {
                  m_shapes = xtReader.getAllShapes();
                  if (!m_shapes.empty())
                    return true;
                }
              }
            }
          }
        }
      }
    }
  }

  return false;
}

std::vector<core::Shape> SolidWorksReader::getAllShapes() const {
  return m_shapes;
}

std::string SolidWorksReader::errorMessage() const { return m_error; }

} // namespace io
} // namespace opencad
