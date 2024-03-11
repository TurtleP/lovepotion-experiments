namespace love
{
// Startup
#define E_UNEXPECTED_ALIGNMENT \
    "Cannot push love object to Lua: unexpected alignment\
    (pointer is %p but alignment should be %d)."

#define E_POINTER_TOO_LARGE "Cannot push love object to Lua: pointer value %p is too large!"

// General
#define E_OUT_OF_MEMORY "Out of memory."
// Filesystem
#define E_PHYSFS_NOT_INITIALIZED     "PHYSFS is not initialized."
#define E_DATA_NOT_WRITTEN           "Data could not be written."
#define E_COULD_NOT_OPEN_FILE        "Could not open file at path {}."
#define E_PHYSFS_COULD_NOT_OPEN_FILE "Could not open file {} {:s}"
#define E_FILE_NOT_OPEN_FOR_WRITING  "File not open for writing."
#define E_INVALID_READ_SIZE          "Invalid read size."
#define E_INVALID_WRITE_SIZE         "Invalid write size."
#define E_NO_FILE_IN_LOVE_DIRS       "\n\tno '%s' in LOVE game directories."
// Audio
#define E_AUDIO_NOT_INITIALIZED "Failed to initialize ndsp"
} // namespace love
