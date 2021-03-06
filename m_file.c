#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "m_file.h"
#include "m_sha1.h"

#define M_META_ROOT ".m"
#define M_META_CAS "cas"
#define M_META_BRANCHES "branches"

#ifdef _MSC_VER
#   define WIN32_LEAN_AND_MEAN
#   include <direct.h>
#   include <Windows.h>

#   define MAX_PATH 260
#   define chdir _chdir
#   define getcwd _getcwd
#   define mkdir _mkdir
#   define snprintf _snprintf
#   define unlink _unlink
#   define M_PATH_SEPARATOR "\\"
#   define M_PATH_SEPARATOR_CHAR '\\'
#else
#   include <sys/stat.h>
#   include <unistd.h>
#   include <limits.h>

#   define MAX_PATH PATH_MAX
#   define M_PATH_SEPARATOR "/"
#   define M_PATH_SEPARATOR_CHAR '/'
#endif

#define HASH_STRING_BUFFER_SIZE ((2 * sizeof(struct m_sha1_hash_t)) + 1)

struct m_cas_write_handle_t
{
    FILE *fp;
    struct m_sha1_hash_context_t hash_context;
    char filename[MAX_PATH];
};

struct m_cas_read_handle_t
{
#ifdef _MSC_VER
    HANDLE file_handle;
    HANDLE file_mapping;
    void *ptr;
#else
#endif
};

static int m_cas_initialized;
static int m_have_meta_root;
static char m_cwd[MAX_PATH];
static char m_meta_root[MAX_PATH];
static char m_cas_root[MAX_PATH];

static int
m_mkdir(const char *directory)
{
#ifdef _MSC_VER
    return mkdir(directory);
#else
    /*
     * In POSIX environments we default the repository directories to have the
     * same permissions as the parent directory.
     */

    char path[MAX_PATH];
    struct stat stat_buf;
    mode_t mode;

    snprintf(path, MAX_PATH, "%s/..", directory);
    stat(path, &stat_buf);
    mode = stat_buf.st_mode & 0x1f;

    return mkdir(directory, mode);
#endif
}

int
m_make_meta_dir(void)
{
    if (m_mkdir(M_META_ROOT) != 0)
    {
        return -1;
    }

    if (m_mkdir(M_META_ROOT M_PATH_SEPARATOR M_META_CAS) != 0)
    {
        return -1;
    }

    return 0;
}

static void
m_hash_to_string(char *buffer, size_t buffer_size, struct m_sha1_hash_t hash)
{
    size_t i;
    unsigned char *ptr;
    size_t idx;
    char hex_chars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

    M_VERIFY(buffer_size >= HASH_STRING_BUFFER_SIZE, "Path length too long.");
    ptr = (unsigned char *)&hash;
    idx = 0;

    for (i = 0; i < sizeof(struct m_sha1_hash_t); ++i)
    {
        unsigned char low_idx = ptr[i] & 0xf;
        unsigned char high_idx = ptr[i] >> 4;

        char low_char = hex_chars[low_idx];
        char high_char = hex_chars[high_idx];

        buffer[idx++] = high_char;
        buffer[idx++] = low_char;
    }

    buffer[idx] = 0;
}

static int
m_exists(const char *filename)
{
    FILE *fp = fopen(filename, "rb");

    if (fp != NULL)
    {
        fclose(fp);
        return 1;
    }

    return 0;
}

static int
m_directory_exists(const char *directory)
{
#ifdef _MSC_VER
    DWORD attributes = GetFileAttributesA(directory);

    if ((attributes != INVALID_FILE_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        return 1;
    }

    return 0;
#else
    struct stat stat_buf;

    return stat(directory, &stat_buf) == 0 && S_ISDIR(stat_buf.st_mode);
#endif
}

static const char *
m_find_meta_root(void)
{
    char *rv;
    char dir[MAX_PATH];
    char prev_dir[MAX_PATH];
    char file[MAX_PATH];

    if (m_have_meta_root)
    {
        return m_meta_root;
    }

    rv = getcwd(m_cwd, MAX_PATH);
    assert(rv != NULL);
    strlcpy(dir, m_cwd, MAX_PATH);

    for (;;)
    {
        snprintf(file, MAX_PATH, "%s" M_PATH_SEPARATOR M_META_ROOT, dir);

        if (m_directory_exists(file))
        {
            strlcpy(m_meta_root, file, MAX_PATH);
            chdir(m_cwd);
            m_have_meta_root = 1;

            return m_meta_root;
        }

        strlcpy(prev_dir, dir, MAX_PATH);
        chdir("..");
        rv = getcwd(dir, MAX_PATH);
        assert(rv != NULL);

        M_VERIFY(!m_streq(dir, prev_dir), "Unable to find valid repository root in directory hierarchy.");
    }
}

static const char *
m_cas_initialize(void)
{
    const char *meta_root;
    char cas_root[MAX_PATH];

    if (!m_cas_initialized)
    {
        meta_root = m_find_meta_root();

        snprintf(cas_root, MAX_PATH, "%s" M_PATH_SEPARATOR M_META_CAS, meta_root);

        if (!m_directory_exists(cas_root))
        {
            m_mkdir(cas_root);
        }

        strlcpy(m_cas_root, cas_root, MAX_PATH);
        m_cas_initialized = 1;
    }

    return m_cas_root;
}

struct m_cas_write_handle_t *
m_cas_write_open(void)
{
    struct m_cas_write_handle_t *handle;
    const char *cas_root;
    int i;

    cas_root = m_cas_initialize();

    i = 0;
    handle = calloc(1, sizeof(struct m_cas_write_handle_t));

    do
    {
        snprintf(handle->filename, MAX_PATH, "%s" M_PATH_SEPARATOR ".cas.tmp.%d", cas_root, i++);
    } while (m_exists(handle->filename));

    handle->fp = fopen(handle->filename, "wb");
    assert(handle->fp != NULL);

    m_sha1_hash_init(&handle->hash_context);

    return handle;
}

void
m_cas_write(struct m_cas_write_handle_t *handle, const void *data, size_t length)
{
    assert(handle != NULL);
    assert(handle->fp != NULL);

    fwrite(data, 1, length, handle->fp);
    m_sha1_hash_update(&handle->hash_context, data, length);
}

static void
m_get_hive_path(char *path, size_t size, struct m_sha1_hash_t hash)
{
    int pos;
    
    pos = (int)strlcpy(path, m_cas_root, size);
    path[pos++] = M_PATH_SEPARATOR_CHAR;
    m_hash_to_string(path + pos, size - pos, hash);
}

struct m_sha1_hash_t
m_cas_write_close(struct m_cas_write_handle_t *handle)
{
    char path[MAX_PATH];
    struct m_sha1_hash_t hash;

    fclose(handle->fp);
    hash = m_sha1_hash_finalize(&handle->hash_context);

    m_get_hive_path(path, MAX_PATH, hash);

    if (!m_exists(path))
    {
#ifdef _MSC_VER
        MoveFileA(handle->filename, path);
#else
        rename(handle->filename, path);
#endif
    }
    else
    {
        unlink(handle->filename);
    }

    free(handle);
    return hash;
}

struct m_cas_read_handle_t *
m_cas_read_open(struct m_sha1_hash_t hash, void **data, size_t *bytes)
{
#ifdef _MSC_VER
    char path[MAX_PATH];
    HANDLE file_handle;
    LARGE_INTEGER file_size;
    HANDLE file_mapping;
    void *ptr;
    struct m_cas_read_handle_t *handle;

    m_get_hive_path(path, MAX_PATH, hash);
    file_handle = CreateFile(
            path, 
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

    if (file_handle == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    if (!GetFileSizeEx(file_handle, &file_size))
    {
        CloseHandle(file_handle);
        return NULL;
    }

    file_mapping = CreateFileMapping(
            file_handle,
            NULL,
            PAGE_READONLY,
            (DWORD)file_size.HighPart,
            file_size.LowPart,
            NULL);

    if (file_mapping == NULL)
    {
        CloseHandle(file_handle);
        return NULL;
    }

    ptr = MapViewOfFile(
            file_mapping,
            FILE_MAP_READ,
            0,
            0,
            0);

    if (ptr == NULL)
    {
        CloseHandle(file_mapping);
        CloseHandle(file_handle);
        return NULL;
    }

    handle = calloc(1, sizeof(struct m_cas_read_handle_t));
    handle->file_handle = file_handle;
    handle->file_mapping = file_mapping;
    handle->ptr = ptr;
    *data = ptr;
    *bytes = (size_t)file_size.QuadPart;

    return handle;

#else
#   error TBD
#endif
}

void
m_cas_read_close(struct m_cas_read_handle_t *handle)
{
#ifdef _MSC_VER
    UnmapViewOfFile(handle->ptr);
    CloseHandle(handle->file_mapping);
    CloseHandle(handle->file_handle);
    free(handle);
#else
#   error TBD
#endif
}

int
m_write(const char *filename, const void *data, size_t bytes)
{
    char file[MAX_PATH];
    FILE *fp;
    size_t bytes_written;

    snprintf(file, sizeof file, "%s" M_PATH_SEPARATOR "%s", m_meta_root, filename);
    fp = fopen(file, "wb");

    if (!fp)
    {
        return -1;
    }

    bytes_written = fwrite(data, 1, bytes, fp);

    if (bytes_written != bytes)
    {
        return -1;
    }

    return 0;
}

int
m_read(const char *filename, void **data, size_t *bytes)
{
    char file[MAX_PATH];
    FILE *fp;
    size_t size;
    void *buf;

    snprintf(file, sizeof file, "%s" M_PATH_SEPARATOR "%s", m_meta_root, filename);
    fp = fopen(file, "rb");

    if (!fp)
    {
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    size = (size_t)ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buf = calloc(1, size);
    fread(buf, 1, size, fp);

    *data = buf;
    *bytes = size;

    return 0;
}
