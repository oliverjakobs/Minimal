#include "application.h"

void minimalGetVersion(int* major, int* minor, int* rev)
{
    if (major != NULL) *major = MINIMAL_VERSION_MAJOR;
    if (minor != NULL) *minor = MINIMAL_VERSION_MINOR;
    if (rev != NULL)   *rev = MINIMAL_VERSION_REVISION;
}

#define MINIMAL_CONCAT_VERSION(m, n, r) #m "." #n "." #r
#define MINIMAL_MAKE_VERSION_STR(m, n, r) MINIMAL_CONCAT_VERSION(m, n, r)

const char* minimalGetVersionString()
{
    return MINIMAL_MAKE_VERSION_STR(MINIMAL_VERSION_MAJOR, MINIMAL_VERSION_MINOR, MINIMAL_VERSION_REVISION);
}

int minimalInit()
{
    if (!minimalPlatformInit())
    {
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void minimalTerminate()
{
    minimalPlatformTerminate();
}

/* --------------------------| minimal app |----------------------------- */
static void minimalGetGLVersion(const char* version_str, int* major, int* minor)
{
    const char* sep = ".";
    const char* major_str = version_str;
    const char* minor_str = version_str + strcspn(version_str, sep) + 1;

    if (major_str && major) *major = atoi(major_str);
    if (minor_str && minor) *minor = atoi(minor_str);
}

int minimalLoad(MinimalApp* app, const char* title, uint32_t w, uint32_t h, const char* gl)
{
    app->debug = 0;
    app->vsync = 1;

    /* minimal initialization */
    if (!minimalInit())
    {
        MINIMAL_ERROR("[App] Failed to initialize Minimal");
        return MINIMAL_FAIL;
    }

    int gl_major, gl_minor;
    minimalGetGLVersion(gl, &gl_major, &gl_minor);

    MinimalWndConfig wnd_config = {
        .gl_major = gl_major,
        .gl_minor = gl_minor,
        .profile = MINIMAL_CONTEXT_CORE_PROFILE,
        .flags = 0
    };

#ifdef _DEBUG
    wnd_config.flags |= MINIMAL_CONTEXT_DEBUG_BIT;
#endif // _DEBUG

    /* creating the window */
    app->window = minimalCreateWindow(title, w, h, &wnd_config);
    if (!app->window)
    {
        MINIMAL_ERROR("[App] Failed to create Minimal window");
        return MINIMAL_FAIL;
    }

    minimalSetApp(app->window, app);
    minimalMakeContextCurrent(app->window);
    minimalTimerReset(&app->timer);

    return (app->on_load) ? app->on_load(app, w, h) : MINIMAL_OK;
}

void minimalDestroy(MinimalApp* app)
{
    if (app->on_destroy) app->on_destroy(app);
    minimalDestroyWindow(app->window);
}

void minimalRun(MinimalApp* app)
{
    while (!minimalShouldClose(app->window))
    {
        minimalTimerStart(&app->timer, minimalGetTime());
        minimalUpdateInput(app->window);

        app->on_update(app, (float)app->timer.deltatime);

        minimalPollEvent(app->window);
        minimalSwapBuffers(app->window);

        minimalTimerEnd(&app->timer, minimalGetTime());
    }
}

void minimalClose(MinimalApp* app) { minimalCloseWindow(app->window); }

/* --------------------------| settings |-------------------------------- */
void minimalSetTitle(MinimalApp* app, const char* title) { minimalSetWindowTitle(app->window, title); }
void minimalEnableDebug(MinimalApp* app, uint8_t b)  { app->debug = b; }
void minimalEnableVsync(MinimalApp* app, uint8_t b)  { minimalSwapInterval(b); app->vsync = b; }

void minimalToggleDebug(MinimalApp* app) { minimalEnableDebug(app, !app->debug); }
void minimalToggleVsync(MinimalApp* app) { minimalEnableVsync(app, !app->vsync); }