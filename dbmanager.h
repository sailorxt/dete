#ifndef DBMANAGER
#define DBMANAGER
#include "posdefine.h"

struct Personnel
{
    char name[20];//����
    int job;//ְλ
    unsigned int ID;//��Ӧ��tag ID
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
    //λ����Ϣ
    void newPos(unsigned int ID, Pos2d& pos);
    void newPos(unsigned int ID, Pos3d& pos);

    //��ǩΪID����Ա�����Ĺ켣����
    int trackCountOfPerson(unsigned int ID);
    //��ñ�ǩΪID����Ա�����ΪtrackIndex�Ĺ켣��������track
    //��óɹ�����true��������false
    bool trackOfPerson(unsigned int ID,int trackIndex,Track2d& track);
    //��Ա��Ϣ
    void addPersonnel(Personnel& person);
    Personnel& findPersonnel(unsigned int ID);
    Personnels& allPersonnel();
    //�������ְλΪjob����Ա����Ϣ
    Personnels& personnelWithJob(int job);
    //��Ϣ���͹���
    void newInformation(Information& info);
    //��õ�ǰ������Ϣ
    Informations& allInformation();
    //�������Ϊtype��������Ϣ
    Informations& informationOfType(int type);

};

#endif // DBMANAGER

