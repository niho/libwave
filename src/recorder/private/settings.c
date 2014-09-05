#include <string.h>
#include "settings.h"

void drSettings_setDefaults(drSettings* settings)
{
    memset(settings, 0, sizeof(drSettings));
    settings->controlEventFIFOCapacity = 50;
    settings->notificationFIFOCapacity = 50;
    settings->recordFIFOCapacity = 250;
}