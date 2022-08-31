#pragma once
#define _CRT_SECURE_NO_DEPRECATE
#include "glew.h"
#include "freeglut.h"
#include <vector>
#include <string>


using namespace std;

unsigned char* LoadFileContent(const char* path, int& filesize);   //该函数来自于萌谷教程
//读取文件，返回文件内容，把文件大小赋值给filesize       


unsigned char* DecodeBMP(unsigned char* bmpFileData, int& width, int& height);
//传入bmp文件内容的内存块，图片宽,高.  需要解码bmp文件后才可以调用，返回一个unsigned char*指针。返回的是像素数据的起始地址

struct Float3
{
    float Data[3];
};

struct Float2      //点的纹理坐标数据类型
{
	float Data[2];   //u,v
};

struct Face {
    int vertex[3][3];
};

/*
GLuint CreateTexture2D(unsigned char* pixelData, int width, int height, GLenum type);
//返回一个标记，openGL中纹理对象的唯一标识符,之后将通过这个标识符，来使用初始化好了的纹理资源
GLuint CreateTexture2DFromBMP(const char* bmpPath);
//接受一个bmp路径，返回一个openGL里的纹理对象
*/

class objLoader {
public:

    objLoader(const char * objFileName);//构造函数

	void setTextureFromBmp(const char* texFileName);  //从obj文件创建纹理
   // objLoader(string filename);//构造函数
    void drawObj();//绘制obj函数
    void drawAABB();//绘制AABB包围盒函数
    double getVolume();//计算体积函数
    double getArea();//计算表面积函数
private:

    double countTriangleArea(Float3 a, Float3 b, Float3 c);//计算三角形面积
    double countPillarVolume(Float3 a, Float3 b, Float3 c);//计算每个三角形与其在ymin面上投影构成的几何体的体积
    void getAABB();//计算模型AABB包围盒时的最大点与最小点
    vector<Float3> mLocation;   //位置信息
    vector<Float3> mNormal;     //法线信息
	vector<Float2> mTexcoord;   //纹理坐标信息
    vector<Face> mFace;         //面信息
	GLuint mTexture;            //模型纹理	
    Float3 vMax;//模型中最大的xyz值
    Float3 vMin;//模型中最小的xyz值
    double area;//表面积
    double volume;//体积
};
