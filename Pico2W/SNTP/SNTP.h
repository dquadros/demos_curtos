#ifndef _SNTP_H

#define _SNTP_H

bool SNTPinit (char *server, int fuso);
void SNTPupdate (void);
time_t SNTPtime (void);
bool SNTPvalid (void);

#endif