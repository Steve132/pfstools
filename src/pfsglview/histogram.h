#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "rmglwin.h"

enum SelectedBar
{
	NONE,
	LEFT_BAR,
	RIGHT_BAR,
	WHOLE_SLIDER
};

class Histogram : public RMGLWin {

	private:
		int xPos, yPos;
		int width, height;
		int* frequencyValues;
		GLfloat* backgroundColor;
		float frequencyMax;
		float sliderPosMin, sliderPosMax; // luminance volues for start and end of slider
		float lumMin, lumMax;
		float logLumMin, logLumMax;
		
		SelectedBar selectionState;

	private:
		void drawBackground();
		void drawScale();
		void drawStatistic();
		void drawSlider();
		
	public:
		Histogram( int xPos, int yPos, int width, int height);
		~Histogram();
		
		void computeFrequency(const pfs::Array2D *image);
		void drawHistogram();
		float getHighFrequency() const;
		float getMaxFrequency() const;
		
		int getWidth( void);
		int getHeight( void);
		int getBackgroundWidth(void);
		int getBackgroundHeight(void);
		float getLumMin();
		float getLumMax();
		
		float pos2lum( float pos);
		float lum2pos( float lum);
		void computeLumRange( float& min, float& max);
		
		void processSliderSelection(int xCoord, int yCoord);
		int setSliderSelectionState(SelectedBar newState);
		SelectedBar getSliderSelectionState();
		void setSliderPosMin( float pos);
		float getSliderPosMin(void);
		void setSliderPosMax( float pos);
		float getSliderPosMax(void);
		void setSliderPosMinMax( float min, float max);	
		void resetFrequencyMax(void);
		
		void redraw(void);

};




#endif




 
