#include <gtk/gtk.h>
#include "ScreenSize.h"
#include "SplashScreen.h"
#include "Tarot_ui.h"
#include <math.h>

GtkWidget *SplashWindow;                  //  The window which will display the splash

GdkPixbuf *SplashPixbuf;
GdkPixbuf *SplashScaled;
GtkWidget *SplashImage;

int ShowSplash(const char *NameSplash, int Monitor, int Duration)
{
GError *Error = NULL;
double RefWidth;
double RefHeight;
double Scale = 1.0;     //  Default scaling : no scaling
int NewWidth, NewHeight;
int Win_Xpos, Win_Ypos;

    if ( Monitor >= nMonitor ) Monitor = 0;     //  If monitor index is illegal, default to 0

    //  Load image from resource
    SplashPixbuf = gdk_pixbuf_new_from_resource(NameSplash, &Error);
    if ( SplashPixbuf == NULL )
    {
        printf("Error %d, %s\n", Error->code, Error->message);
        return 1;       //  Error !
    }

    //  Get pixbuf dimension
    RefWidth = gdk_pixbuf_get_width(SplashPixbuf);
    RefHeight = gdk_pixbuf_get_height(SplashPixbuf);
    printf("Splash size %.0fx%.0f\n", RefWidth, RefHeight);
    //  Compute the scale factor, splash should not be greater than 1/2 screen

    if ( RefWidth > 0.5*MonitorWidth[Monitor] )
    {
        Scale = 0.5*MonitorWidth[Monitor] / RefWidth;
    }
    if ( RefHeight > 0.5*MonitorHeight[Monitor] )
    {
        Scale = fmin(0.5*MonitorHeight[Monitor] / RefHeight, Scale);
    }
    NewWidth = Scale * RefWidth;
    NewHeight = Scale * RefHeight;
    printf("Scaling : %.2f, New image size %dx%d\n", Scale, NewWidth, NewHeight);
    //  Apply scaling
    SplashScaled = gdk_pixbuf_scale_simple(SplashPixbuf, NewWidth, NewHeight, GDK_INTERP_BILINEAR);
    //  Build the window
    SplashWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    //  Remove window title and border
    gtk_container_set_border_width (GTK_CONTAINER (SplashWindow), 0);
    gtk_window_set_decorated(GTK_WINDOW (SplashWindow), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(SplashWindow), FALSE);
    gtk_widget_set_size_request (SplashWindow, NewWidth, NewHeight);
    Win_Xpos = (MonitorWidth[Monitor] - Scale*RefWidth)/2 + MonitorXPos[Monitor];
    Win_Ypos = (MonitorHeight[Monitor] - Scale*RefHeight)/2 + MonitorYPos[Monitor];

    gtk_window_move(GTK_WINDOW(SplashWindow), Win_Xpos, Win_Ypos);
    SplashImage = gtk_image_new_from_pixbuf(SplashScaled);
    gtk_container_add(GTK_CONTAINER(SplashWindow), SplashImage);
    gtk_widget_show_all (SplashWindow);
    g_timeout_add (Duration, close_screen, SplashWindow);
    g_object_unref(SplashPixbuf);       //  Free original pixbuf if reference count is 0
    return 0;
}

