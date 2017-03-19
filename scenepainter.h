#ifndef SCENEPAINTER
#define SCENEPAINTER
#include "posdefine.h"

struct PosStyle
{
	PosStyle()
	{
		r=255;
		g=255;
		b=255;
		size=5;
	}
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned int size;
};

struct TrackStyle
{
	TrackStyle()
	{
		r=255;
		g=255;
		b=255;
		width=5;
	}
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned int width;
};



class ScenePainter
{
public:
    //人员信息绘制
    //绘制人员实时位置，ID为标签编号，topaint为绘制/隐藏开关，true为绘制，false为隐藏，
    //style为绘制风格
    void drawRealtimePos(unsigned int ID, Pos2d& pos, bool topaint=true, PosStyle style=PosStyle());
    void drawTrack(unsigned int ID, Track2d& track, bool topaint=true, TrackStyle style=TrackStyle());
    //场景绘制
    void setRoomSize(double width, double depth, double height);
    void setSpecialArea(int ID, Pos2d* areaPos, bool topaint);
	//调整视角和距离
	void setXRotation(int angle);
	void setYRotation(int angle);
	void setZRotation(int angle);
	void setEyePoint( Pos3d& point);
	void resize(int width, int height);

};

#endif // SCENEPAINTER

