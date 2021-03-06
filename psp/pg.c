// primitive graphics for Hello World sce
#include <stdio.h>
#include <syscall.h>
#include <define.h>
#include <types.h>
#include <pg.h>
#include "font.c"
#include "fontNaga10.c"

//variables
//char *pg_vramtop=(char *)0x04000000;
#define pg_vramtop ((char *)0x04000000)
long pg_screenmode;
long pg_showframe;
long pg_drawframe;
unsigned long pgc_csr_x[2], pgc_csr_y[2];
unsigned long pgc_fgcolor[2], pgc_bgcolor[2];
char pgc_fgdraw[2], pgc_bgdraw[2];
char pgc_mag[2];

char pg_mypath[MAX_PATH];
char pg_workdir[MAX_PATH];

void pgWaitVn(unsigned long count)
{
	for (; count>0; --count) {
		sceDisplayWaitVblankStart();
	}
}


void pgWaitV()
{
	sceDisplayWaitVblankStart();
}


char *pgGetVramAddr(unsigned long x,unsigned long y)
{
	return pg_vramtop+(pg_drawframe?FRAMESIZE:0)+x*PIXELSIZE*2+y*LINESIZE*2+0x40000000;
//	return pg_vramtop+(pg_drawframe?FRAMESIZE:0)+x*PIXELSIZE*2+y*LINESIZE*2;//+0x40000000;	//変わらないらしい
}


void pgInit()
{
	sceDisplaySetMode(0,SCREEN_WIDTH,SCREEN_HEIGHT);
	pgScreenFrame(0,0);
}


void pgPrint(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX_X && y<CMAX_Y) {
		pgPutChar(x*8,y*8,color,0,*str,1,0,1);
		str++;
		x++;
		if (x>=CMAX_X) {
			x=0;
			y++;
		}
	}
}

void pgPrint2(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX2_X && y<CMAX2_Y) {
		pgPutChar(x*16,y*16,color,0,*str,1,0,2);
		str++;
		x++;
		if (x>=CMAX2_X) {
			x=0;
			y++;
		}
	}
}


void pgPrint4(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX4_X && y<CMAX4_Y) {
		pgPutChar(x*32,y*32,color,0,*str,1,0,4);
		str++;
		x++;
		if (x>=CMAX4_X) {
			x=0;
			y++;
		}
	}
}

void pgDrawFrame(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=pgGetVramAddr(0,0);
	for(i=x1; i<=x2; i++){
		((unsigned short *)vptr0)[i*PIXELSIZE + y1*LINESIZE] = color;
		((unsigned short *)vptr0)[i*PIXELSIZE + y2*LINESIZE] = color;
	}
	for(i=y1; i<=y2; i++){
		((unsigned short *)vptr0)[x1*PIXELSIZE + i*LINESIZE] = color;
		((unsigned short *)vptr0)[x2*PIXELSIZE + i*LINESIZE] = color;
	}
}

void pgFillBox(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i, j;

	vptr0=pgGetVramAddr(0,0);
	for(i=y1; i<=y2; i++){
		for(j=x1; j<=x2; j++){
			((unsigned short *)vptr0)[j*PIXELSIZE + i*LINESIZE] = color;
		}
	}
}

void pgFillvram(unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=pgGetVramAddr(0,0);
	for (i=0; i<FRAMESIZE/2; i++) {
		*(unsigned short *)vptr0=color;
		vptr0+=PIXELSIZE*2;
	}
}

void pgBitBlt(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short *d)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	unsigned long xx,yy,mx,my;
	const unsigned short *dd;

	vptr0=pgGetVramAddr(x,y);
	for (yy=0; yy<h; yy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			dd=d;
			for (xx=0; xx<w; xx++) {
				for (mx=0; mx<mag; mx++) {
					*(unsigned short *)vptr=*dd;
					vptr+=PIXELSIZE*2;
				}
				dd++;
			}
			vptr0+=LINESIZE*2;
		}
		d+=w;
	}

}

//ちょい早いx1 - LCK
void pgBitBltN1h(unsigned long x,unsigned long y,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long yy;

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=0; yy<144; yy++) {
		__memcpy4a(v0,d,112);
		v0+=(LINESIZE/2-112);
	}
}

static void _rotate_backbuffer(unsigned short *backbuffer)
{
	static signed short temp[224*144];
	memcpy(temp,backbuffer,224*144*2);

	int line, column;
	for (line=0;line<144;line++)
		for (column=0;column<224;column++)
			backbuffer[line+((223-column)<<7)+((223-column)<<4)]=temp[column+(line<<7)+(line<<6)+(line<<5)];
}

void pgBitBltN1v(unsigned long x,unsigned long y,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long yy;

	_rotate_backbuffer((unsigned short *)d);

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=0; yy<224; yy++) {
		__memcpy4a(v0,d,72);
		v0+=(LINESIZE/2-72);
	}
}

//あんまり変わらないx1.5 -LCK
void pgBitBltN15(unsigned long x,unsigned long y,unsigned long *d)
{
	unsigned short *vptr0;		//pointer to vram
	unsigned short *vptr;		//pointer to vram
	unsigned long xx,yy;
	
	vptr0=(unsigned short *)pgGetVramAddr(x,y);
	for (yy=0; yy<72; yy++) {
		unsigned long *d0=d+(yy*2)*88;
		vptr=vptr0;
		for (xx=0; xx<80; xx++) {
			unsigned long dd1,dd2,dd3,dd4;
			unsigned long dw;
			dw=d0[0];
			dd1=((vptr[0]           =((dw)     & 0x739c))) ;
			dd2=((vptr[2]           =((dw>>16) & 0x739c))) ;
			dw=d0[88];
			dd3=((vptr[0+LINESIZE*2]=((dw)     & 0x739c))) ;
			dd4=((vptr[2+LINESIZE*2]=((dw>>16) & 0x739c))) ;

			vptr++;
			*vptr=(dd1+dd2) >> 1;
			vptr+=(LINESIZE-1);
			*vptr=(dd1+dd3) >> 1;
			vptr++;
			*vptr=(dd1+dd2+dd3+dd4) >> 2;
			vptr++;
			*vptr=(dd2+dd4) >> 1;
			vptr+=(LINESIZE-1);
			*vptr=(dd3+dd4) >> 1;
			vptr+=(2-LINESIZE*2);
			d0+=1;
		}
		vptr0+=LINESIZE*3;
	}
}

//よくわかんないx2 - LCK
void pgBitBltN2(unsigned long x,unsigned long y,unsigned long h,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long xx,yy;
	unsigned long dx,dl[2];

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=h; yy>0; --yy) {
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
//			dl[0]=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
//			dl[1]=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			cpy2x(dl,dx);
			v0[LINESIZE/2]=dl[0];
			v0[LINESIZE/2+1]=dl[1];
			*(v0++)=dl[0];
			*(v0++)=dl[1];
		}
		v0+=(LINESIZE-160);
		d+=8;
	}
}

//by z-rwt
void pgBitBltStScan(unsigned long x,unsigned long y,unsigned long h,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long xx,yy;
	unsigned long dx;

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=h; yy>0; --yy) {
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
//			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
//			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
//			*(v0++)=d0;
//			*(v0++)=d1;
			cpy2x(v0,dx);
			v0+=2;
		}
		v0+=(LINESIZE-160);
		d+=8;
	}
}

//by z-rwt
void pgBitBltSt2wotop(unsigned long x,unsigned long y,unsigned long h,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long xx,yy;
	unsigned long dx,dl[2];

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=0; yy<16; yy++){
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
//			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
//			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
//			*(v0++)=d0;
//			*(v0++)=d1;
			cpy2x(v0,dx);
			v0+=2;
		}
		v0+=(LINESIZE/2-160);
		d+=8;
	}
	for (; yy<h; yy++) {
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
#if 1
			dl[0]=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
			dl[1]=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
#else
			cpy2x(dl,dx);
#endif
			v0[LINESIZE/2]=dl[0];
			v0[LINESIZE/2+1]=dl[1];
			*(v0++)=dl[0];
			*(v0++)=dl[1];
		}
		v0+=(LINESIZE-160);
		d+=8;
	}
}

//by z-rwt
void pgBitBltSt2wobot(unsigned long x,unsigned long y,unsigned long h,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long xx,yy;
	unsigned long dx,dl[2];

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=0; yy<h-16; yy++){
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
//			dl[0]=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
//			dl[1]=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			cpy2x(dl,dx);
			v0[LINESIZE/2]=dl[0];
			v0[LINESIZE/2+1]=dl[1];
			*(v0++)=dl[0];
			*(v0++)=dl[1];
		}
		v0+=(LINESIZE-160);
		d+=8;
	}
	for (; yy<h; yy++) {
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
//			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
//			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
//			*(v0++)=d0;
//			*(v0++)=d1;
			cpy2x(v0,dx);
			v0+=2;
		}
		v0+=(LINESIZE/2-160);
		d+=8;
	}
}

//Parallel blend
static inline unsigned long PBlend(unsigned long c0, unsigned long c1)
{
	return (c0 & c1) + (((c0 ^ c1) & 0x7bde7bde) >> 1);
}

//2x Fit
void pgBitBltSt2Fix(unsigned long x,unsigned long y,unsigned long h,unsigned long mag,const unsigned short *d)
{
	unsigned long	*vptr0;		//pointer to vram
	unsigned long	*vptr;		//pointer to vram
	unsigned long	xx, yy;
	unsigned short	er, f, hf;
	unsigned long	*dl;

	f = hf = 0;
	er = SCREEN_HEIGHT;
	vptr0 = (unsigned long*)pgGetVramAddr(x, 0);
	for(yy = 0; yy < SCREEN_HEIGHT; yy++) {
		vptr = vptr0;
		dl = (unsigned long *)d;
		if(hf == 0) {
			for(xx = 80; xx > 0; xx--) {
				cpy2x(vptr, *dl++);
				vptr+=2;
			}
		} else {
			for(xx = 80; xx > 0; xx--) {
				cpy2x(vptr, PBlend(*(dl-88), *dl++));
				vptr+=2;
			}
			hf = 0;
		}
		vptr0 += LINESIZE/2;
		er += 15;
		if(er > SCREEN_HEIGHT - 3 && f == 0) {
			er -= SCREEN_HEIGHT - 2;
			f++;
			hf = 1;
		}
		f++;
		if(f > 1) {
			f -= 2;
			d += 176;
		}
	}
}

//Full
void pgBitBltStFull(unsigned long x,unsigned long y,unsigned long h,unsigned long mag,const unsigned short *d)
{
	unsigned long	*vptr0;		//pointer to vram
	unsigned long	xx, yy;
	unsigned short	er, f, hf;
	unsigned long	*dl;

	f = hf = 0;
	er = SCREEN_HEIGHT;
	vptr0 = (unsigned long*)pgGetVramAddr(0, 0);
	for(yy = SCREEN_HEIGHT; yy > 0 ; yy--) {
		dl = (unsigned long *)d;
		if(hf == 0) {
			for(xx = 80; xx > 0; xx--) {
				cpy3x(vptr0, *dl++);
				vptr0+=3;
			}
		} else {
			for(xx = 80; xx > 0; xx--) {
				cpy3x(vptr0, PBlend(*(dl-88), *dl++));
				vptr0+=3;
			}
			hf = 0;
		}
		vptr0 += (LINESIZE -160*3)/2;
		er += 15;
		if(er > SCREEN_HEIGHT - 3 && f == 0) {
			er -= SCREEN_HEIGHT - 2;
			f++;
			hf = 1;
		}
		f++;
		if(f > 1) {
			f -= 2;
			d += 176;
		}
	}
}

void pgPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	const unsigned char *cfont;		//pointer to font
	unsigned long cx,cy;
	unsigned long b;
	char mx,my;

	if (ch>255) return;
	cfont=font+ch*8;
	vptr0=pgGetVramAddr(x,y);
	for (cy=0; cy<8; cy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			b=0x80;
			for (cx=0; cx<8; cx++) {
				for (mx=0; mx<mag; mx++) {
					if ((*cfont&b)!=0) {
						if (drawfg) *(unsigned short *)vptr=color;
					} else {
						if (drawbg) *(unsigned short *)vptr=bgcolor;
					}
					vptr+=PIXELSIZE*2;
				}
				b=b>>1;
			}
			vptr0+=LINESIZE*2;
		}
		cfont++;
	}
}

void pgScreenFrame(long mode,long frame)
{
	pg_screenmode=mode;
	frame=(frame?1:0);
	pg_showframe=frame;
	if (mode==0) {
		//screen off
		pg_drawframe=frame;
		sceDisplaySetFrameBuf(0,0,0,1);
	} else if (mode==1) {
		//show/draw same
		pg_drawframe=frame;
		sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	} else if (mode==2) {
		//show/draw different
		pg_drawframe=(frame?0:1);
		sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	}
}


void pgScreenFlip()
{
	pg_showframe=(pg_showframe?0:1);
	pg_drawframe=(pg_drawframe?0:1);
	sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,0);
}


void pgScreenFlipV()
{
	pgWaitV();
	pgScreenFlip();
}

// by kwn
void Draw_Char_Hankaku(int x,int y,const unsigned char c,int col) {
	unsigned short *vr;
	unsigned char  *fnt;
	unsigned char  pt;
	unsigned char ch;
	int x1,y1;

	ch = c;

	// mapping
	if (ch<0x20)
		ch = 0;
	else if (ch<0x80)
		ch -= 0x20;
	else if (ch<0xa0)
		ch = 0;
	else
		ch -= 0x40;

	fnt = (unsigned char *)&hankaku_font10[ch*10];

	// draw
	vr = (unsigned short *)pgGetVramAddr(x,y);
	for(y1=0;y1<10;y1++) {
		pt = *fnt++;
		for(x1=0;x1<5;x1++) {
			if (pt & 1)
				*vr = col;
			vr++;
			pt = pt >> 1;
		}
		vr += LINESIZE-5;
	}
}

// by kwn
void Draw_Char_Zenkaku(int x,int y,const unsigned char u,unsigned char d,int col) {
	// ELISA100.FNTに存在しない文字
	const unsigned short font404[] = {
		0xA2AF, 11,
		0xA2C2, 8,
		0xA2D1, 11,
		0xA2EB, 7,
		0xA2FA, 4,
		0xA3A1, 15,
		0xA3BA, 7,
		0xA3DB, 6,
		0xA3FB, 4,
		0xA4F4, 11,
		0xA5F7, 8,
		0xA6B9, 8,
		0xA6D9, 38,
		0xA7C2, 15,
		0xA7F2, 13,
		0xA8C1, 720,
		0xCFD4, 43,
		0xF4A5, 1030,
		0,0
	};
	unsigned short *vr;
	unsigned short *fnt;
	unsigned short pt;
	int x1,y1;

	unsigned long n;
	unsigned short code;
	int i, j;

	// SJISコードの生成
	code = u;
	code = (code<<8) + d;

	// SJISからEUCに変換
	if(code >= 0xE000) code-=0x4000;
	code = ((((code>>8)&0xFF)-0x81)<<9) + (code&0x00FF);
	if((code & 0x00FF) >= 0x80) code--;
	if((code & 0x00FF) >= 0x9E) code+=0x62;
	else code-=0x40;
	code += 0x2121 + 0x8080;

	// EUCから恵梨沙フォントの番号を生成
	n = (((code>>8)&0xFF)-0xA1)*(0xFF-0xA1)
		+ (code&0xFF)-0xA1;
	j=0;
	while(font404[j]) {
		if(code >= font404[j]) {
			if(code <= font404[j]+font404[j+1]-1) {
				n = -1;
				break;
			} else {
				n-=font404[j+1];
			}
		}
		j+=2;
	}
	fnt = (unsigned short *)&zenkaku_font10[n*10];

	// draw
	vr = (unsigned short *)pgGetVramAddr(x,y);
	for(y1=0;y1<10;y1++) {
		pt = *fnt++;
		for(x1=0;x1<10;x1++) {
			if (pt & 1)
				*vr = col;
			vr++;
			pt = pt >> 1;
		}
		vr += LINESIZE-10;
	}
}

// by kwn
void mh_print(int x,int y,const unsigned char *str,int col) {
	unsigned char ch = 0,bef = 0;

	while(*str != 0) {
		ch = *str++;
		if (bef!=0) {
			Draw_Char_Zenkaku(x,y,bef,ch,col);
			x+=10;
			bef=0;
		} else {
			if (((ch>=0x80) && (ch<0xa0)) || (ch>=0xe0)) {
				bef = ch;
			} else {
				Draw_Char_Hankaku(x,y,ch,col);
				x+=5;
			}
		}
	}
}

u32 new_pad;
u32 old_pad;
u32 now_pad;
ctrl_data_t paddata;
extern SETTING setting;

void readpad(void)
{
	static int n=0;
	ctrl_data_t paddata;

	sceCtrlRead(&paddata, 1);
	// kmg
	// Analog pad state
	if (setting.analog2dpad) {
		if (paddata.analog[CTRL_ANALOG_Y] == 0xff) paddata.buttons=CTRL_DOWN;  // DOWN
		if (paddata.analog[CTRL_ANALOG_Y] == 0x00) paddata.buttons=CTRL_UP;    // UP
		if (paddata.analog[CTRL_ANALOG_X] == 0x00) paddata.buttons=CTRL_LEFT;  // LEFT
		if (paddata.analog[CTRL_ANALOG_X] == 0xff) paddata.buttons=CTRL_RIGHT; // RIGHT
	}
	now_pad = paddata.buttons;
	new_pad = now_pad & ~old_pad;
	if(old_pad==now_pad){
		n++;
		if(n>=25){
			new_pad=now_pad;
			n = 20;
		}
	}else{
		n=0;
		old_pad = now_pad;
	}
}

/******************************************************************************/


void pgcLocate(unsigned long x, unsigned long y)
{
	if (x>=CMAX_X) x=0;
	if (y>=CMAX_Y) y=0;
	pgc_csr_x[pg_drawframe?1:0]=x;
	pgc_csr_y[pg_drawframe?1:0]=y;
}


void pgcColor(unsigned long fg, unsigned long bg)
{
	pgc_fgcolor[pg_drawframe?1:0]=fg;
	pgc_bgcolor[pg_drawframe?1:0]=bg;
}


void pgcDraw(char drawfg, char drawbg)
{
	pgc_fgdraw[pg_drawframe?1:0]=drawfg;
	pgc_bgdraw[pg_drawframe?1:0]=drawbg;
}


void pgcSetmag(char mag)
{
	pgc_mag[pg_drawframe?1:0]=mag;
}

void pgcCls()
{
	pgFillvram(pgc_bgcolor[pg_drawframe]);
	pgcLocate(0,0);
}

void pgcPutchar_nocontrol(const char ch)
{
	pgPutChar(pgc_csr_x[pg_drawframe]*8, pgc_csr_y[pg_drawframe]*8, pgc_fgcolor[pg_drawframe], pgc_bgcolor[pg_drawframe], ch, pgc_fgdraw[pg_drawframe], pgc_bgdraw[pg_drawframe], pgc_mag[pg_drawframe]);
	pgc_csr_x[pg_drawframe]+=pgc_mag[pg_drawframe];
	if (pgc_csr_x[pg_drawframe]>CMAX_X-pgc_mag[pg_drawframe]) {
		pgc_csr_x[pg_drawframe]=0;
		pgc_csr_y[pg_drawframe]+=pgc_mag[pg_drawframe];
		if (pgc_csr_y[pg_drawframe]>CMAX_Y-pgc_mag[pg_drawframe]) {
			pgc_csr_y[pg_drawframe]=CMAX_Y-pgc_mag[pg_drawframe];
//			pgMoverect(0,pgc_mag[pg_drawframe]*8,SCREEN_WIDTH,SCREEN_HEIGHT-pgc_mag[pg_drawframe]*8,0,0);
		}
	}
}

void pgcPutchar(const char ch)
{
	if (ch==0x0d) {
		pgc_csr_x[pg_drawframe]=0;
		return;
	}
	if (ch==0x0a) {
		if ((++pgc_csr_y[pg_drawframe])>=CMAX_Y) {
			pgc_csr_y[pg_drawframe]=CMAX_Y-1;
//			pgMoverect(0,8,SCREEN_WIDTH,SCREEN_HEIGHT-8,0,0);
		}
		return;
	}
	pgcPutchar_nocontrol(ch);
}

void pgcPuthex2(const unsigned long s)
{
	char ch;
	ch=((s>>4)&0x0f);
	pgcPutchar((ch<10)?(ch+0x30):(ch+0x40-9));
	ch=(s&0x0f);
	pgcPutchar((ch<10)?(ch+0x30):(ch+0x40-9));
}


void pgcPuthex8(const unsigned long s)
{
	pgcPuthex2(s>>24);
	pgcPuthex2(s>>16);
	pgcPuthex2(s>>8);
	pgcPuthex2(s);
}


void pgScreenInit()
{
	sceDisplaySetMode(0,SCREEN_WIDTH,SCREEN_HEIGHT);
	pgScreenFrame(0,1);
	pgcLocate(0,0);
	pgcColor(0xffff,0x0000);
	pgcDraw(1,1);
	pgcSetmag(1);
	pgScreenFrame(0,0); //screen off
	pgcLocate(0,0);
	pgcColor(0xffff,0x0000);
	pgcDraw(1,1);
	pgcSetmag(1);
	pgScreenFrame(2,0); //show/draw different
}

void pgControlInit()
{
	sceCtrlInit(0);
	sceCtrlSetAnalogMode(1);
}

void pgScreenClear()
{
	pgFillvram(0);
	pgScreenFlipV();
	pgFillvram(0);
	pgScreenFlipV();
}

void pgMessage(char *msg, MESSAGE_LEVEL lvl, int wait)
{
	int color;

	switch (lvl) {
	case MSG_WARN :
		color = 0xff00;
		break;
	case MSG_ERROR :
		color = 0x00ff;
		break;
	case MSG_INFO :
		color = 0xffff;
		break;
	case MSG_DEBUG :
		color = 0x0ff0;
		break;
	default :
		color = 0x0000;
	}
//	pgFillBox(0, 263, strlen(msg) * 8, 271, 0);
	pgFillBox(0, 263, SCREEN_WIDTH, 271, 0);
	pgPrint(0, 33, color, msg);
	pgScreenFlip();
	if (wait > 0)
		pgWaitVn(wait);
}


void pgDebug(char *msg, int y)
{
	pgFillBox(0, 8*y, strlen(msg) * 8, 8*(y+1), 0);
	pgPrint(0, y, 0xffff, msg);
//	pgScreenFlip();
}








//----------------------------------------------------------------------------------------------------------------------
#if 1

int pgfOpen(const char *filename, unsigned long flag)
{
	return sceIoOpen(filename,flag,0777);
}

void pgfClose(int fd)
{
	sceIoClose(fd);
}

int pgfRead(int fd, void *data, int size)
{
	return sceIoRead(fd,data,size);
}

int pgfWrite(int fd, void *data, int size)
{
	return sceIoWrite(fd,data,size);
}

int pgfSeek(int fd, int offset, int whence)
{
	return sceIoLseek(fd,offset,whence);
}

#endif
//----------------------------------------------------------------------------------------------------------------------
