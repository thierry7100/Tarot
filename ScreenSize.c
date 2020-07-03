#include <gtk/gtk.h>
#include "ScreenSize.h"

int MonitorWidth[MAX_MONITOR];
int MonitorHeight[MAX_MONITOR];
int MonitorScaleFactor[MAX_MONITOR];
int MonitorSizeH[MAX_MONITOR];
int MonitorSizeV[MAX_MONITOR];
int MonitorXPos[MAX_MONITOR];
int MonitorYPos[MAX_MONITOR];
int nMonitor;

int GetScreensSize()
{
GdkDisplay *MyDisplay;
GdkMonitor *MyMonitor;
GdkRectangle ScreenRect;
int ScaleFactor;
int m;

    MyDisplay = gdk_display_get_default();
    if ( MyDisplay != NULL )
    {
        nMonitor = gdk_display_get_n_monitors(MyDisplay);
        if ( nMonitor > 0 )
        {
            for ( m = 0; m < nMonitor; m++)
            {
                MyMonitor = gdk_display_get_monitor(MyDisplay, m);
                if ( MyMonitor != NULL )
                {
                    gdk_monitor_get_geometry(MyMonitor, &ScreenRect);
                    ScaleFactor = gdk_monitor_get_scale_factor(MyMonitor);
                    MonitorScaleFactor[m] = ScaleFactor;
                    MonitorWidth[m] = ScreenRect.width;
                    MonitorHeight[m] = ScreenRect.height;
                    if ( m > 0 )
                    {
                        MonitorXPos[m] = MonitorWidth[m-1];
                        MonitorYPos[m] = 0;
                    }
                    else
                    {
                        MonitorXPos[m] = 0;
                        MonitorYPos[m] = 0;
                    }
                    MonitorSizeH[m] = gdk_monitor_get_width_mm(MyMonitor);
                    MonitorSizeV[m] = gdk_monitor_get_height_mm(MyMonitor);
                    printf("Monitor %d: %dx%d, scale factor %d\n", m, MonitorWidth[m], MonitorHeight[m], MonitorScaleFactor[m]);
                    printf("Size in mm : %dx%d\n", MonitorSizeH[m], MonitorSizeV[m]);
                    printf("Window origin : %d,%d\n", MonitorXPos[m], MonitorYPos[m]);
                }
            }
            return 1;
        }
    }
    return -1;
}
