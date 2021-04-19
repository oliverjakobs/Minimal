#include "Minimal.h"

#include "toolbox/tb_str.h"

static uint64_t     _minimal_timer_frequency;
static uint64_t     _minimal_timer_offset;

static MinimalWindow* _minimal_current_context;

static uint64_t MinimalGetTimeValue()
{
    uint64_t value;
    QueryPerformanceCounter((LARGE_INTEGER*)&value);
    return value;
}

uint8_t MinimalInit()
{
    MinimalCreateKeyTable();

    /* init timer */
    if (!QueryPerformanceFrequency((LARGE_INTEGER*)&_minimal_timer_frequency))
    {
        MINIMAL_ERROR("High-resolution performance counter is not supported");
        return MINIMAL_FAIL;
    }

    _minimal_timer_offset = MinimalGetTimeValue();
    _minimal_current_context = NULL;

    return MINIMAL_OK;
}

double MinimalGetTime()
{
    return (double)(MinimalGetTimeValue() - _minimal_timer_offset) / _minimal_timer_frequency;
}

void MinimalMakeContextCurrent(MinimalWindow* context)	{ _minimal_current_context = context; }
MinimalWindow* MinimalGetCurrentContext()				{ return _minimal_current_context; }


/* --------------------------| Minimal App |----------------------------- */
static void MinimalGetGLVersion(const char* version_str, int* major, int* minor)
{
	const char* sep = ".";
	const char* major_str = version_str;
	const char* minor_str = version_str + strcspn(version_str, sep) + 1;

	if (major_str && major) *major = atoi(major_str);
	if (minor_str && minor) *minor = atoi(minor_str);
}

uint8_t MinimalLoad(MinimalApp* app, const char* title, int width, int height, char* gl_version)
{
	app->title = tb_strdup(title);
	if (!app->title)
	{
		MINIMAL_ERROR("[GLFW] Failed to allocate memory for title");
		return MINIMAL_FAIL;
	}

	app->debug = 0;
	app->vsync = 0;

	/* Minimal initialization */
	if (!MinimalInit())
	{
		MINIMAL_ERROR("[Minimal] Failed to initialize Minimal");
		return MINIMAL_FAIL;
	}

	MINIMAL_INFO("[Minimal] Initialized Minimal");

	int gl_major, gl_minor;
	MinimalGetGLVersion(gl_version, &gl_major, &gl_minor);

	MINIMAL_INFO("Version: %d.%d", gl_major, gl_minor);

	/* creating the window */
	if (!MinimalCreateWindow(&app->window, "Minimal", width, height))
	{
		MINIMAL_ERROR("[Minimal] Failed to create Minimal window");
		return MINIMAL_FAIL;
	}

	MinimalMakeContextCurrent(&app->window);
	MinimalTimerReset(&app->timer);

	return (app->on_load) ? app->on_load(app) : MINIMAL_OK;
}

void MinimalDestroy(MinimalApp* app)
{
	if (app->on_destroy) app->on_destroy(app);
	MinimalDestroyWindow(&app->window);
}

void MinimalRun(MinimalApp* app, void(*clear_buffer)())
{
	while (!MinimalShouldCloseWindow(&app->window))
	{
		MinimalTimerStart(&app->timer, MinimalGetTime());
		// InputUpdate(app->window);

		app->on_update(app, (float)app->timer.deltatime);

		if (clear_buffer) clear_buffer();

		app->on_render(app);

		if (app->debug) app->on_render_debug(app);

		MinimalPollEvent(&app->window);
		MinimalSwapBuffer(&app->window);

		MinimalTimerEnd(&app->timer, MinimalGetTime());
	}
}

void MinimalSetLoadCallback(MinimalApp* app, MinimalLoadCB cb) { app->on_load = cb; }
void MinimalSetDestroyCallback(MinimalApp* app, MinimalDestroyCB cb) { app->on_destroy = cb; }
void MinimalSetUpdateCallback(MinimalApp* app, MinimalUpdateCB cb) { app->on_update = cb; }
void MinimalSetRenderCallback(MinimalApp* app, MinimalRenderCB cb) { app->on_render = cb; }
void MinimalSetRenderDebugCallback(MinimalApp* app, MinimalRenderDebugCB cb) { app->on_render_debug = cb; }
void MinimalSetRenderGUICallback(MinimalApp* app, MinimalRenderGUICB cb) { app->on_render_gui = cb; }

/* --------------------------| Settings |-------------------------------- */
void MinimalClose(MinimalApp* app) { MinimalCloseWindow(&app->window); }
void MinimalEnableDebug(MinimalApp* app, int b) { app->debug = b; }
void MinimalEnableVsync(MinimalApp* app, int b) { /* glfwSwapInterval(b); */ app->vsync = b; }

void MinimalToggleDebug(MinimalApp* app) { MinimalEnableDebug(app, !app->debug); }
void MinimalToggleVsync(MinimalApp* app) { MinimalEnableVsync(app, !app->vsync); }

uint32_t MinimalGetFps(MinimalApp* app)	{ return app->timer.fps; }

/* --------------------------| Input |----------------------------------- */
uint8_t MinimalKeyPressed(uint32_t keycode)
{
	int8_t state = MinimalGetKeyState(_minimal_current_context, keycode);
	return state >= 0 && (state == MINIMAL_PRESS || state == MINIMAL_REPEAT);
}

uint8_t MinimalKeyReleased(uint32_t keycode)
{
	int8_t state = MinimalGetKeyState(_minimal_current_context, keycode);
	return state >= 0 && MinimalGetKeyState(_minimal_current_context, keycode) == MINIMAL_RELEASE;
}

uint8_t MinimalMouseButtonPressed(uint32_t button)
{
	int8_t state = MinimalGetMouseButtonState(_minimal_current_context, button);
	return state >= 0 && MINIMAL_PRESS;
}

uint8_t MinimalMouseButtonReleased(uint32_t button)
{
	int8_t state = MinimalGetMouseButtonState(_minimal_current_context, button);
	return state >= 0 && MINIMAL_RELEASE;
}

void MinimalGetCursorPos(float* x, float* y)
{
	MinimalWindow* window = MinimalGetCurrentContext();
	if (!window) return;

	POINT pos = { 0 };
	if (GetCursorPos(&pos)) ScreenToClient(window->handle, &pos);

	if (x) *x = (float)pos.x;
	if (y) *y = (float)pos.y;
}