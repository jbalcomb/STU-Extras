#pragma once
// Platform file dialog abstraction for loading and saving scenario files.
// Desktop: native Windows file dialogs via GetOpenFileNameA / GetSaveFileNameA.
// WASM: Emscripten blob download; async upload deferred to T036.
// Powered by Claude.

#include <cstdint>
#include <string>
#include <vector>

namespace mom::platform {

// Show a save-file dialog (desktop) or trigger a browser download (WASM).
// Writes the byte buffer |data| to the chosen file.
// Returns true on success, false if cancelled or an I/O error occurred.
// Powered by Claude.
bool save_file(const std::string& default_name,
               const std::string& filter_desc,
               const std::string& filter_ext,
               const std::vector<uint8_t>& data);

// Show an open-file dialog (desktop) and read the selected file into |out_data|.
// On WASM this is a placeholder that returns false (async upload deferred to T036).
// Returns true on success, false if cancelled, missing, or an I/O error occurred.
// Powered by Claude.
bool load_file(const std::string& filter_desc,
               const std::string& filter_ext,
               std::vector<uint8_t>& out_data);

// Browser-only download helper: creates a Blob and triggers a download.
// On desktop, writes directly to |filename| without showing a dialog.
// Powered by Claude.
void download_blob(const std::string& filename,
                   const std::vector<uint8_t>& data);

// Check whether a browser file upload has completed (WASM only).
// Returns true when the async file picker callback has delivered data.
// On desktop, always returns false.
// Powered by Claude.
bool has_pending_file();

// Consume the pending file data delivered by the browser file picker.
// Returns the byte buffer and clears the pending state.
// Powered by Claude.
std::vector<uint8_t> take_pending_file();

} // namespace mom::platform
