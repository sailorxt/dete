#ifndef DBMANAGER
#define DBMANAGER
#include "posdefine.h"

struct Personnel
{
    char name[20];//姓名
    int job;//职位
    unsigned int ID;//对应的tag ID
};

struct Information
{
    char text[512];
    int type;
};
enum InformationTypes
{
    seriousWarning=0,
    warning=1,
    notification=2

};

typedef std::vector<Personnel> Personnels;
typedef std::vector<Information> Informations;

class DbManager
{
public:
    //位置信息
    void newPos(unsigned int ID, Pos2d& pos);
    void newPos(unsigned int ID, Pos3d& pos);

    //标签为ID的人员产生的轨迹数量
    int trackCountOfPerson(unsigned int ID);
    //获得标签为ID的人员的序号为trackIndex的轨迹，保存至track
    //获得成功返回true，出错返回false
    bool trackOfPerson(unsigned int ID,int trackIndex,Track2d& track);
    //人员信息
    void addPersonnel(Personnel& person);
    Personnel& findPersonnel(unsigned int ID);
    Personnels& allPersonnel();
    //获得所有职位为job的人员的信息
    Personnels& personnelWithJob(int job);
    //信息推送管理
    void newInformation(Information& info);
    //获得当前所有信息
    Informations& allInformation();
    //获得类型为type的所有信息
    Informations& informationOfType(int type);

};

#endif // DBMANAGER

