#include "MinimalUtil.h"

/* --------------------------| Input |----------------------------------- */
uint8_t MinimalKeycodeValid(uint32_t keycode)
{
    return keycode >= MINIMAL_KEY_FIRST && keycode <= MINIMAL_KEY_LAST;
}

uint8_t MinimalMouseButtonValid(uint32_t buttoncode)
{
    return buttoncode >= MINIMAL_MOUSE_BUTTON_1 && buttoncode <= MINIMAL_MOUSE_BUTTON_LAST;
}

/* --------------------------| Logging |--------------------------------- */
#define MINIMAL_LOG_BLACK       "\x1b[30m"
#define MINIMAL_LOG_RED         "\x1b[31m"
#define MINIMAL_LOG_GREEN       "\x1b[32m"
#define MINIMAL_LOG_YELLOW      "\x1b[33m"
#define MINIMAL_LOG_BLUE        "\x1b[34m"
#define MINIMAL_LOG_MAGENTA     "\x1b[35m"
#define MINIMAL_LOG_CYAN        "\x1b[36m"
#define MINIMAL_LOG_WHITE       "\x1b[37m"

#define MINIMAL_LOG_BG_BLACK    "\x1b[40m"
#define MINIMAL_LOG_BG_RED      "\x1b[41m"
#define MINIMAL_LOG_BG_GREEN    "\x1b[42m"
#define MINIMAL_LOG_BG_YELLOW   "\x1b[43m"
#define MINIMAL_LOG_BG_BLUE     "\x1b[44m"
#define MINIMAL_LOG_BG_MAGENTA  "\x1b[45m"
#define MINIMAL_LOG_BG_CYAN     "\x1b[46m"
#define MINIMAL_LOG_BG_WHITE    "\x1b[47m"

#define MINIMAL_LOG_RESET       "\x1b[0m" /* No color */

static const char* MinimalLoggerGetLevelStr(MinimalLogLevel level)
{
    switch (level)
    {
    case MINIMAL_LOG_TRACE:     return MINIMAL_LOG_WHITE "[TRACE]" MINIMAL_LOG_RESET " ";
    case MINIMAL_LOG_INFO:      return MINIMAL_LOG_GREEN "[INFO]" MINIMAL_LOG_RESET " ";
    case MINIMAL_LOG_WARN:      return MINIMAL_LOG_YELLOW "[WARN]" MINIMAL_LOG_RESET " ";
    case MINIMAL_LOG_ERROR:     return MINIMAL_LOG_RED "[ERROR]" MINIMAL_LOG_RESET " ";
    case MINIMAL_LOG_CRITICAL:  return MINIMAL_LOG_WHITE MINIMAL_LOG_BG_RED "[CRITICAL]" MINIMAL_LOG_RESET " ";
    default: return "";
    }
}

void MinimalLoggerPrint(FILE* const stream, MinimalLogLevel level, const char* fmt, ...)
{
    fprintf(stream, MinimalLoggerGetLevelStr(level));

    va_list arg;
    va_start(arg, fmt);
    vfprintf(stream, fmt, arg);
    va_end(arg);

    fprintf(stream, "\n");
}

/* --------------------------| Timer |----------------------------------- */
void MinimalTimerReset(MinimalTimer* timer)
{
    timer->seconds = 0.0;
    timer->frames = 0;
    timer->fps = 0;

    timer->deltatime = 0.0;
    timer->lastframe = 0.0;
}

void MinimalTimerStart(MinimalTimer* timer, double seconds)
{
    timer->deltatime = seconds - timer->lastframe;
    timer->lastframe = seconds;
}

void MinimalTimerEnd(MinimalTimer* timer, double seconds)
{
    timer->frames++;
    if ((seconds - timer->seconds) > 1.0)
    {
        timer->seconds += 1.0;
        timer->fps = timer->frames;
        timer->frames = 0;
    }
}