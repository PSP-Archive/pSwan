
	uint32		 startTime, endTime, totalFrames;
	unsigned int nNormalLast=0;
	int			 nNormalFrac=0; 
	int			 nTime=0,nCount=0; int i=0;
	int			surfacePitch;

	// 15 bits RGB555
	Format format(16,0x007c00,0x00003e0,0x0000001f);
	Console console;
	Surface *surface;
