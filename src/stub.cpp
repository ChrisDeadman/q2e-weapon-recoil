// Minimal stub to verify the cross-compilation toolchain produces a valid DLL.
// Replace with SDK sources once the submodule is integrated.

#include <cstdint>

struct game_import_t;
struct game_export_t;

extern "C" __declspec(dllexport)
game_export_t *GetGameAPI(game_import_t * /*import*/) {
    return nullptr;
}
