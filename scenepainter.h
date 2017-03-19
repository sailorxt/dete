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
    //��Ա��Ϣ����
    //������Աʵʱλ�ã�IDΪ��ǩ��ţ�topaintΪ����/���ؿ��أ�trueΪ���ƣ�falseΪ���أ�
    //styleΪ���Ʒ��
    void drawRealtimePos(unsigned int ID, Pos2d& pos, bool topaint=true, PosStyle style=PosStyle());
    void drawTrack(unsigned int ID, Track2d& track, bool topaint=true, TrackStyle style=TrackStyle());
    //��������
    void setRoomSize(double width, double depth, double height);
    void setSpecialArea(int ID, Pos2d* areaPos, bool topaint);
	//�����ӽǺ;���
	void setXRotation(int angle);
	void setYRotation(int angle);
	void setZRotation(int angle);
	void setEyePoint( Pos3d& point);
	void resize(int width, int height);

};

#endif // SCENEPAINTER

