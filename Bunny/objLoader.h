#pragma once
#define _CRT_SECURE_NO_DEPRECATE
#include "glew.h"
#include "freeglut.h"
#include <vector>
#include <string>


using namespace std;

unsigned char* LoadFileContent(const char* path, int& filesize);   //�ú����������ȹȽ̳�
//��ȡ�ļ��������ļ����ݣ����ļ���С��ֵ��filesize       


unsigned char* DecodeBMP(unsigned char* bmpFileData, int& width, int& height);
//����bmp�ļ����ݵ��ڴ�飬ͼƬ��,��.  ��Ҫ����bmp�ļ���ſ��Ե��ã�����һ��unsigned char*ָ�롣���ص����������ݵ���ʼ��ַ

struct Float3
{
    float Data[3];
};

struct Float2      //�������������������
{
	float Data[2];   //u,v
};

struct Face {
    int vertex[3][3];
};

/*
GLuint CreateTexture2D(unsigned char* pixelData, int width, int height, GLenum type);
//����һ����ǣ�openGL����������Ψһ��ʶ��,֮��ͨ�������ʶ������ʹ�ó�ʼ�����˵�������Դ
GLuint CreateTexture2DFromBMP(const char* bmpPath);
//����һ��bmp·��������һ��openGL����������
*/

class objLoader {
public:

    objLoader(const char * objFileName);//���캯��

	void setTextureFromBmp(const char* texFileName);  //��obj�ļ���������
   // objLoader(string filename);//���캯��
    void drawObj();//����obj����
    void drawAABB();//����AABB��Χ�к���
    double getVolume();//�����������
    double getArea();//������������
private:

    double countTriangleArea(Float3 a, Float3 b, Float3 c);//�������������
    double countPillarVolume(Float3 a, Float3 b, Float3 c);//����ÿ��������������ymin����ͶӰ���ɵļ���������
    void getAABB();//����ģ��AABB��Χ��ʱ����������С��
    vector<Float3> mLocation;   //λ����Ϣ
    vector<Float3> mNormal;     //������Ϣ
	vector<Float2> mTexcoord;   //����������Ϣ
    vector<Face> mFace;         //����Ϣ
	GLuint mTexture;            //ģ������	
    Float3 vMax;//ģ��������xyzֵ
    Float3 vMin;//ģ������С��xyzֵ
    double area;//�����
    double volume;//���
};
