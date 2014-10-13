
#include "rmglwin.h"

class WinStat : public RMGLWin {
	
	private:
		int pixelX;
		int pixelY;
		int colR, colG, colB;
		float lumMax;
		const char* mappingMode;
		int frameNo;
		const char* channel;
		int rawPosX, rawPosY;
		float rawX, rawY, rawZ;
		bool bZoom;
				
	public:
		//WinStat( int posX, int posY, int width, int height);
		WinStat();
		~WinStat();
		
		void redraw(void);
		void setPixelData( int xx, int yy, int r, int g, int b);
		void setMaxFreq(float val);
		void setMapping(const char*);
		void setFrameNo(int no);
		void setChannel(const char* ch) ;
		void setRawData(int x, int y, float X, float Y, float Z);
		void setBZoom(bool bb);
};

