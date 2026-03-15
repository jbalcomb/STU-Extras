// Platform file dialog implementation.
// Desktop: Windows common dialogs (GetSaveFileNameA / GetOpenFileNameA).
// WASM: Emscripten EM_ASM blob download; load_file deferred to T036.
// Powered by Claude.

#include "platform/FileDialog.hpp"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include <windows.h>
#include <commdlg.h>
#endif

// --------------------------------------------------------------------------
// Pending file buffer for asynchronous browser file uploads.
// The JS FileReader callback writes into this buffer; the main loop polls
// has_pending_file() and consumes via take_pending_file().
// Powered by Claude.
// --------------------------------------------------------------------------
static std::vector<uint8_t> g_pending_file_data;
static bool g_pending_file_ready = false;

#ifdef __EMSCRIPTEN__
extern "C" {
// C callback invoked by JavaScript FileReader.onload.
// Copies |len| bytes from |ptr| into the pending buffer.
// Powered by Claude.
EMSCRIPTEN_KEEPALIVE
void mom_file_upload_callback(const uint8_t* ptr, int len) {
    g_pending_file_data.assign(ptr, ptr + len);
    g_pending_file_ready = true;
}
}
#endif

namespace mom::platform {

// --------------------------------------------------------------------------
// Helper: build a double-null-terminated filter string for OPENFILENAMEA.
// Format: "GAM Files\0*.GAM\0All Files\0*.*\0\0"
// std::string can hold embedded NULs when sized explicitly.
// Powered by Claude.
// --------------------------------------------------------------------------
#ifndef __EMSCRIPTEN__
static std::string build_filter_string(const std::string& desc,
                                       const std::string& ext)
{
    // "desc\0ext\0All Files\0*.*\0\0"
    std::string filter;
    filter.append(desc);
    filter.push_back('\0');
    filter.append(ext);
    filter.push_back('\0');
    filter.append("All Files");
    filter.push_back('\0');
    filter.append("*.*");
    filter.push_back('\0');
    // Final terminating NUL (OPENFILENAMEA requires double-null termination).
    filter.push_back('\0');
    return filter;
}
#endif

// --------------------------------------------------------------------------
// save_file — show a save dialog and write bytes to the chosen path.
// Powered by Claude.
// --------------------------------------------------------------------------
bool save_file(const std::string& default_name,
               const std::string& filter_desc,
               const std::string& filter_ext,
               const std::vector<uint8_t>& data)
{
#ifdef __EMSCRIPTEN__
    // On WASM, fall through to a blob download using the default name.
    download_blob(default_name, data);
    return true;
#else
    std::string filter = build_filter_string(filter_desc, filter_ext);

    char filepath[MAX_PATH];
    std::memset(filepath, 0, sizeof(filepath));

    // Seed the filename buffer with the suggested default name.
    if (!default_name.empty()) {
        std::strncpy(filepath, default_name.c_str(), MAX_PATH - 1);
    }

    OPENFILENAMEA ofn;
    std::memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize  = sizeof(ofn);
    ofn.hwndOwner    = nullptr;
    ofn.lpstrFilter  = filter.c_str();
    ofn.lpstrFile    = filepath;
    ofn.nMaxFile     = MAX_PATH;
    ofn.Flags        = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt  = nullptr;

    if (!GetSaveFileNameA(&ofn)) {
        return false; // User cancelled or an error occurred.
    }

    std::ofstream ofs(filepath, std::ios::binary);
    if (!ofs) {
        return false;
    }
    ofs.write(reinterpret_cast<const char*>(data.data()),
              static_cast<std::streamsize>(data.size()));
    return ofs.good();
#endif
}

// --------------------------------------------------------------------------
// load_file — show an open dialog and read the selected file into out_data.
// Powered by Claude.
// --------------------------------------------------------------------------
bool load_file(const std::string& filter_desc,
               const std::string& filter_ext,
               std::vector<uint8_t>& out_data)
{
#ifdef __EMSCRIPTEN__
    // Trigger browser file picker asynchronously.
    // Creates a temporary <input type="file"> element, attaches an onchange
    // handler that reads the file via FileReader, and calls the C callback
    // mom_file_upload_callback to deliver bytes. Returns false immediately;
    // caller must poll has_pending_file() on subsequent frames.
    // Powered by Claude.
    (void)filter_desc;
    (void)out_data;
    g_pending_file_ready = false;
    g_pending_file_data.clear();
    EM_ASM({
        var accept = UTF8ToString($0);
        var input = document.createElement("input");
        input.type = "file";
        input.accept = accept;
        input.style.display = "none";
        input.onchange = function(e) {
            var file = e.target.files[0];
            if (!file) return;
            var reader = new FileReader();
            reader.onload = function() {
                var data = new Uint8Array(reader.result);
                var buf = Module._malloc(data.length);
                Module.HEAPU8.set(data, buf);
                Module._mom_file_upload_callback(buf, data.length);
                Module._free(buf);
            };
            reader.readAsArrayBuffer(file);
            document.body.removeChild(input);
        };
        document.body.appendChild(input);
        input.click();
    }, filter_ext.c_str());
    return false;
#else
    std::string filter = build_filter_string(filter_desc, filter_ext);

    char filepath[MAX_PATH];
    std::memset(filepath, 0, sizeof(filepath));

    OPENFILENAMEA ofn;
    std::memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize  = sizeof(ofn);
    ofn.hwndOwner    = nullptr;
    ofn.lpstrFilter  = filter.c_str();
    ofn.lpstrFile    = filepath;
    ofn.nMaxFile     = MAX_PATH;
    ofn.Flags        = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (!GetOpenFileNameA(&ofn)) {
        return false; // User cancelled or an error occurred.
    }

    std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);
    if (!ifs) {
        return false;
    }

    std::streamsize size = ifs.tellg();
    if (size < 0) {
        return false;
    }
    ifs.seekg(0, std::ios::beg);

    out_data.resize(static_cast<size_t>(size));
    ifs.read(reinterpret_cast<char*>(out_data.data()), size);
    return ifs.good();
#endif
}

// --------------------------------------------------------------------------
// download_blob — trigger a file download in the browser, or write directly
// to disk on desktop (no dialog).
// Powered by Claude.
// --------------------------------------------------------------------------
void download_blob(const std::string& filename,
                   const std::vector<uint8_t>& data)
{
#ifdef __EMSCRIPTEN__
    // Create a JavaScript Blob from the raw bytes, build an object URL,
    // click an invisible <a> element to start the download, then revoke.
    EM_ASM({
        var bytes  = HEAPU8.subarray($0, $0 + $1);
        var blob   = new Blob([new Uint8Array(bytes)],
                              {type: "application/octet-stream"});
        var url    = URL.createObjectURL(blob);
        var a      = document.createElement("a");
        a.href     = url;
        a.download = UTF8ToString($2);
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
    }, data.data(), static_cast<int>(data.size()), filename.c_str());
#else
    // Desktop fallback: write directly to the given filename with no dialog.
    std::ofstream ofs(filename, std::ios::binary);
    if (ofs) {
        ofs.write(reinterpret_cast<const char*>(data.data()),
                  static_cast<std::streamsize>(data.size()));
    }
#endif
}

// --------------------------------------------------------------------------
// has_pending_file — check if the async browser file picker has delivered data.
// Powered by Claude.
// --------------------------------------------------------------------------
bool has_pending_file() {
    return g_pending_file_ready;
}

// --------------------------------------------------------------------------
// take_pending_file — consume the pending file buffer and clear the flag.
// Powered by Claude.
// --------------------------------------------------------------------------
std::vector<uint8_t> take_pending_file() {
    g_pending_file_ready = false;
    return std::move(g_pending_file_data);
}

} // namespace mom::platform
