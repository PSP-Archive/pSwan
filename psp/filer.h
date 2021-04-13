#ifndef FILER_H
#define FILER_H

extern char LastPath[], FilerMsg[];

int getExtId(const char *szFilePath);

int searchFile(const char *path, const char *name);
int getFilePath(char *out);

// —LŒø‚ÈŠg’£Žq
enum {
	EXT_WS,
	EXT_WSC,
#if 0
	EXT_ZIP,
#endif
	EXT_UNKNOWN
};



#endif
