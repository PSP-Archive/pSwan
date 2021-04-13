#include <stdio.h>
#include <string.h>
#include "types.h"
#include "define.h"
#include "syscall.h"
#include "filer.h"

extern u32 new_pad;

struct dirent files[MAX_ENTRY];
int nfiles;

////////////////////////////////////////////////////////////////////////
// クイックソート
// AC add start
void SJISCopy(struct dirent *a, unsigned char *file)
{
	unsigned char ca;
	int i;

	for(i=0;i<=strlen(a->name);i++){
		ca = a->name[i];
		if (((0x81 <= ca)&&(ca <= 0x9f))
		|| ((0xe0 <= ca)&&(ca <= 0xef))){
			file[i++] = ca;
			file[i] = a->name[i];
		}
		else{
			if(ca>='a' && ca<='z') ca-=0x20;
			file[i] = ca;
		}
	}

}
int cmpFile(struct dirent *a, struct dirent *b)
{
    unsigned char file1[0x108];
    unsigned char file2[0x108];
	unsigned char ca, cb;
	int i, n, ret;

	if(a->type==b->type){
		SJISCopy(a, file1);
		SJISCopy(b, file2);
		n=strlen(file1);
		for(i=0; i<=n; i++){
			ca=file1[i]; cb=file2[i];
			ret = ca-cb;
			if(ret!=0) return ret;
		}
		return 0;
	}
	
	if(a->type & TYPE_DIR)	return -1;
	else					return 1;
}
// AC add end

/* AC del start
int cmpFile(struct dirent *a, struct dirent *b)
{
	char ca, cb;
	int i, n, ret;
	
	if(a->type==b->type){
		n=strlen(a->name);
		for(i=0; i<=n; i++){
			ca=a->name[i]; cb=b->name[i];
			if(ca>='a' && ca<='z') ca-=0x20;
			if(cb>='a' && cb<='z') cb-=0x20;
			
			ret = ca-cb;
			if(ret!=0) return ret;
		}
		return 0;
	}
	
	if(a->type & TYPE_DIR)	return -1;
	else					return 1;
}
AC del end */
void sort(struct dirent *a, int left, int right) {
	struct dirent tmp, pivot;
	int i, p;
	
	if (left < right) {
		pivot = a[left];
		p = left;
		for (i=left+1; i<=right; i++) {
			if (cmpFile(&a[i],&pivot)<0){
				p=p+1;
				tmp=a[p];
				a[p]=a[i];
				a[i]=tmp;
			}
		}
		a[left] = a[p];
		a[p] = pivot;
		sort(a, left, p-1);
		sort(a, p+1, right);
	}
}

// 拡張子管理用
const struct {
	char *szExt;
	int nExtId;
} stExtentions[] = {
 "ws",EXT_WS,
 "wsc",EXT_WSC,
#if 0
 "zip",EXT_ZIP,
#endif
 NULL, EXT_UNKNOWN
};

int getExtId(const char *szFilePath) {
	char *pszExt;
	int i;
	if((pszExt = strrchr(szFilePath, '.'))) {
		pszExt++;
		for (i = 0; stExtentions[i].nExtId != EXT_UNKNOWN; i++) {
			if (!stricmp(stExtentions[i].szExt,pszExt)) {
				return stExtentions[i].nExtId;
			}
		}
	}
	return EXT_UNKNOWN;
}



void getDir(const char *path) {
	int fd, b=0;
	char *p;
	
	nfiles = 0;
	
	if(strcmp(path,"ms0:/")){
		strcpy(files[nfiles].name,"..");
		files[nfiles].type = TYPE_DIR;
		nfiles++;
		b=1;
	}
	
	fd = sceIoDopen(path);
	while(nfiles<MAX_ENTRY){
		if(sceIoDread(fd, &files[nfiles])<=0) break;
		if(files[nfiles].name[0] == '.') continue;
		if(files[nfiles].type == TYPE_DIR){
			strcat(files[nfiles].name, "/");
			nfiles++;
			continue;
		}
		if(getExtId(files[nfiles].name) != EXT_UNKNOWN) nfiles++;
	}
	sceIoDclose(fd);
	if(b)
		sort(files+1, 0, nfiles-2);
	else
		sort(files, 0, nfiles-1);
}

char LastPath[MAX_PATH];
char FilerMsg[256];
SETTING setting;
int getFilePath(char *out)
{
	unsigned long color;
	static int sel=0;
	int top, rows=21, x, y, h, i, len, bMsg=0, up=0;
	char path[MAX_PATH], oldDir[MAX_NAME], *p;
	char msg1[128];

	top = sel-3;
	
	strcpy(path, LastPath);
#if 0
	if(FilerMsg[0])
		bMsg=1;
#else
	bMsg = 0;
#endif
	
	getDir(path);
	for(;;){
		readpad();
		if(new_pad)
			bMsg=0;
		if(new_pad & CTRL_CIRCLE){
			if(files[sel].type == TYPE_DIR){
				if(!strcmp(files[sel].name,"..")){
					up=1;
				}else{
					strcat(path,files[sel].name);
					getDir(path);
					sel=0;
				}
			}else{
				strcpy(out, path);
				strcat(out, files[sel].name);
				strcpy(LastPath,path);
				return 1;
			}
		}else if(new_pad & CTRL_CROSS){
			return 0;
		}else if(new_pad & CTRL_TRIANGLE){
			up=1;
		}else if(new_pad & CTRL_UP){
			sel--;
		}else if(new_pad & CTRL_DOWN){
			sel++;
		}else if(new_pad & CTRL_LEFT){
			sel-=10;
		}else if(new_pad & CTRL_RIGHT){
			sel+=10;
		}
		
		if(up){
			if(strcmp(path,"ms0:/")){
				p=strrchr(path,'/');
				*p=0;
				p=strrchr(path,'/');
				p++;
				strcpy(oldDir,p);
				strcat(oldDir,"/");
				*p=0;
				getDir(path);
				sel=0;
				for(i=0; i<nfiles; i++) {
					if(!strcmp(oldDir, files[i].name)) {
						sel=i;
						top=sel-3;
						break;
					}
				}
			}
			up=0;
		}
		
		if(top > nfiles-rows)	top=nfiles-rows;
		if(top < 0)				top=0;
		if(sel >= nfiles)		sel=nfiles-1;
		if(sel < 0)				sel=0;
		if(sel >= top+rows)		top=sel-rows+1;
		if(sel < top)			top=sel;
		
		if (bMsg) {
//			menu_frame(FilerMsg,"○：OK　×：CANCEL　△：UP");
			sprintf(msg1, "%c:OK %c:CANCEL %c:UP", 1, 2, 4);
			menu_frame(FilerMsg, msg1);
		} else {
//			menu_frame(path,"○：OK　×：CANCEL　△：UP");
			sprintf(msg1, "%c:OK %c:CANCEL %c:UP", 1, 2, 4);
			menu_frame(path, msg1);
		}

		// スクロールバー
		if(nfiles > rows){
			h = 219;
			pgDrawFrame(445,25,446,248,setting.color[1]);
			pgFillBox(448, h*top/nfiles + 27,
				460, h*(top+rows)/nfiles + 27,setting.color[1]);
		}
		
#if 0
		x=28; y=32;
#else
		x = 3; y = 4;
#endif
		for(i=0; i<rows; i++){
			if(top+i >= nfiles) break;
			if(top+i == sel) color = setting.color[3];
			else			 color = setting.color[2];
#if 0
			mh_print(x, y, files[top+i].name, color);
			y+=10;
#else
			pgPrint(x, y, color, files[top + i].name);
			y++;
#endif
		}
		
		pgScreenFlipV();
	}
}
