#include "winpty_api.h"

typedef winpty_config_t *
(*pfn_winpty_config_new)(UINT64 agentFlags, winpty_error_ptr_t *err /*OPTIONAL*/);

typedef
void (*pfn_winpty_config_free)(winpty_config_t *cfg);

typedef
void
(*pfn_winpty_config_set_initial_size)(winpty_config_t *cfg, int cols, int rows);

typedef winpty_t *
(*pfn_winpty_open)(const winpty_config_t *cfg,
            winpty_error_ptr_t *err /*OPTIONAL*/);
typedef HANDLE (*pfn_winpty_agent_process)(winpty_t *wp);
typedef LPCWSTR (*pfn_winpty_conin_name)(winpty_t *wp);
typedef LPCWSTR (*pfn_winpty_conout_name)(winpty_t *wp);
typedef LPCWSTR (*pfn_winpty_conerr_name)(winpty_t *wp);

typedef winpty_spawn_config_t *
(*pfn_winpty_spawn_config_new)(UINT64 spawnFlags,
                        LPCWSTR appname /*OPTIONAL*/,
                        LPCWSTR cmdline /*OPTIONAL*/,
                        LPCWSTR cwd /*OPTIONAL*/,
                        LPCWSTR env /*OPTIONAL*/,
                        winpty_error_ptr_t *err /*OPTIONAL*/);

typedef void (*pfn_winpty_spawn_config_free)(winpty_spawn_config_t *cfg);

typedef BOOL
(*pfn_winpty_spawn)(winpty_t *wp,
             const winpty_spawn_config_t *cfg,
             HANDLE *process_handle /*OPTIONAL*/,
             HANDLE *thread_handle /*OPTIONAL*/,
             DWORD *create_process_error /*OPTIONAL*/,
             winpty_error_ptr_t *err /*OPTIONAL*/);
typedef BOOL
(*pfn_winpty_set_size)(winpty_t *wp, int cols, int rows,
                winpty_error_ptr_t *err /*OPTIONAL*/);

typedef void (*pfn_winpty_free)(winpty_t *wp);

class WinptyApi {
public:
    WinptyApi()
        : m_hWinPty{nullptr}
        , fn_winpty_config_new {}
        , fn_winpty_config_free {}
        , fn_winpty_config_set_initial_size {}
        , fn_winpty_open {}
        , fn_winpty_agent_process {}
        , fn_winpty_conin_name {}
        , fn_winpty_conout_name {}
        , fn_winpty_conerr_name {}
        , fn_winpty_spawn_config_new {}
        , fn_winpty_spawn_config_free {}
        , fn_winpty_spawn {}
        , fn_winpty_set_size {}
        , fn_winpty_free {}
    {
        m_hWinPty = LoadLibrary("winpty.dll");

        if (!m_hWinPty)
            return;

        fn_winpty_config_new = (pfn_winpty_config_new)GetProcAddress(m_hWinPty, "winpty_config_new");
        fn_winpty_config_free = (pfn_winpty_config_free)GetProcAddress(m_hWinPty, "winpty_config_free");
        fn_winpty_config_set_initial_size = (pfn_winpty_config_set_initial_size)GetProcAddress(m_hWinPty, "winpty_config_set_initial_size");
        fn_winpty_open = (pfn_winpty_open)GetProcAddress(m_hWinPty, "winpty_open");
        fn_winpty_agent_process = (pfn_winpty_agent_process)GetProcAddress(m_hWinPty, "winpty_agent_process");
        fn_winpty_conin_name = (pfn_winpty_conin_name)GetProcAddress(m_hWinPty, "winpty_conin_name");
        fn_winpty_conout_name = (pfn_winpty_conout_name)GetProcAddress(m_hWinPty, "winpty_conout_name");
        fn_winpty_conerr_name = (pfn_winpty_conerr_name)GetProcAddress(m_hWinPty, "winpty_conerr_name");
        fn_winpty_spawn_config_new = (pfn_winpty_spawn_config_new)GetProcAddress(m_hWinPty, "winpty_spawn_config_new");
        fn_winpty_spawn_config_free = (pfn_winpty_spawn_config_free)GetProcAddress(m_hWinPty, "winpty_spawn_config_free");
        fn_winpty_spawn = (pfn_winpty_spawn)GetProcAddress(m_hWinPty, "winpty_spawn");
        fn_winpty_set_size = (pfn_winpty_set_size)GetProcAddress(m_hWinPty, "winpty_set_size");
        fn_winpty_free = (pfn_winpty_free)GetProcAddress(m_hWinPty, "winpty_free");
    }

    ~WinptyApi() {
        if (m_hWinPty) {
            FreeLibrary(m_hWinPty);
        }
    }

    bool HasWinPtyApi() {
        return m_hWinPty
                && fn_winpty_config_new
                && fn_winpty_config_free
                && fn_winpty_config_set_initial_size
                && fn_winpty_open
                && fn_winpty_agent_process
                && fn_winpty_conin_name
                && fn_winpty_conout_name
                && fn_winpty_conerr_name
                && fn_winpty_spawn_config_new
                && fn_winpty_spawn_config_free
                && fn_winpty_spawn
                && fn_winpty_set_size
                && fn_winpty_free
                ;
    }
public:
    HMODULE m_hWinPty;
    pfn_winpty_config_new fn_winpty_config_new;
    pfn_winpty_config_free fn_winpty_config_free;
    pfn_winpty_config_set_initial_size fn_winpty_config_set_initial_size;
    pfn_winpty_open fn_winpty_open;
    pfn_winpty_agent_process fn_winpty_agent_process;
    pfn_winpty_conin_name fn_winpty_conin_name;
    pfn_winpty_conout_name fn_winpty_conout_name;
    pfn_winpty_conerr_name fn_winpty_conerr_name;
    pfn_winpty_spawn_config_new fn_winpty_spawn_config_new;
    pfn_winpty_spawn_config_free fn_winpty_spawn_config_free;
    pfn_winpty_spawn fn_winpty_spawn;
    pfn_winpty_set_size fn_winpty_set_size;
    pfn_winpty_free fn_winpty_free;
};

static
WinptyApi g_winptyApi;

bool HasWinPtyApi() {
    return g_winptyApi.HasWinPtyApi();
}
