#ifndef SCREENSIZE_H_INCLUDED
#define SCREENSIZE_H_INCLUDED

#define MAX_MONITOR 16

extern int MonitorWidth[MAX_MONITOR];
extern int MonitorHeight[MAX_MONITOR];
extern int MonitorXPos[MAX_MONITOR];
extern int MonitorYPos[MAX_MONITOR];
extern int MonitorScaleFactor[MAX_MONITOR];
extern int MonitorSizeH[MAX_MONITOR];
extern int MonitorSizeV[MAX_MONITOR];
extern int nMonitor;
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
int GetScreensSize();
#ifdef __cplusplus
}
#endif // __cplusplus


#endif // SCREENSIZE_H_INCLUDED
