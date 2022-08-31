
#include "stb_image.h"
#include <iostream>
#include "objLoader.h"
#include <fstream>

#include <sstream>

using namespace std;

objLoader::objLoader(const char * objFileName)
{

    int nFileSize = 0;
    unsigned char* fileContent = LoadFileContent(objFileName, nFileSize);    //��ȡ�ļ�����
    if (fileContent == nullptr)     //�ļ�Ϊ�� 
    {
        return;
    }
    stringstream ssFileContent((char*)fileContent);   //����ȡ�ļ�����
    string temp;       //�����޹���Ϣ
    char szoneLine[256];        //��ȡһ�е�����
    while (!ssFileContent.eof())
    {
        memset(szoneLine, 0, 256);        //  ÿ��ѭ����ʼ������szoneLine
        ssFileContent.getline(szoneLine, 256);      //����ȡһ��
        if (strlen(szoneLine) > 0)       //���в�Ϊ��
        {
            if (szoneLine[0] == 'v')     //v��ͷ������
            {
                stringstream ssOneLine(szoneLine);        //���ݴ洢������ ���㸳ֵ
                if (szoneLine[1] == 't')       //������Ϣ
                {
                    ssOneLine >> temp;     //���ܱ�ʶ�� vt
                    Float2 tempTexcoord;
                    ssOneLine >> tempTexcoord.Data[0] >> tempTexcoord.Data[1];   //���ݴ�����ʱ������
                    mTexcoord.push_back(tempTexcoord);         //��������

                }
                if (szoneLine[1] == 'n')            //������Ϣ
                {
                    ssOneLine >> temp;      //���ձ�ʶ��vn
                    Float3 tempNormal;
                    ssOneLine >> tempNormal.Data[0] >> tempNormal.Data[1] >> tempNormal.Data[2];
                    mNormal.push_back(tempNormal);
                }
                else                          //���λ����Ϣ
                {
                    ssOneLine >> temp;
                    Float3 tempLocation;
                    ssOneLine >> tempLocation.Data[0] >> tempLocation.Data[1] >> tempLocation.Data[2];
                    mLocation.push_back(tempLocation);
                }
            }
            else if (szoneLine[0] == 'f')          //����Ϣ
            {
                stringstream ssOneLine(szoneLine);     //����ȡһ������
                ssOneLine >> temp; //���ձ�ʶ��f
                //    f��Ϣ    exp�� f 1/1/1 2/2/2 3/3/3      λ������/��������/��������   ������Ƭ �����㹹��һ����
                string vertexStr;   //������������
                Face tempFace;
                for (int i = 0; i < 3; ++i)         //ÿ����������
                {
                    ssOneLine >> vertexStr;           //�����ж�ȡ���������Ϣ
                    size_t pos = vertexStr.find_first_of('/');       //�ҵ���һ��/��λ��      //���ҵ����λ����Ϣ
                    string locIndexStr = vertexStr.substr(0, pos);       //��ֵ���λ����Ϣ
                    size_t pos2 = vertexStr.find_first_of('/', pos + 1);   //�ҵ��ڶ���/   ���ҵ��������������Ϣ
                    string texIndexSrt = vertexStr.substr(pos + 1, pos2 - 1 - pos);       //��ֵ�������������Ϣ
                    string norIndexSrt = vertexStr.substr(pos2 + 1, vertexStr.length() - 1 - pos2);   //��ֵ��ķ�����Ϣ
                    tempFace.vertex[i][0] = atoi(locIndexStr.c_str());        //��������Ϣ�� srtingת��Ϊ int     //λ��������Ϣ��ֵ
                    tempFace.vertex[i][1] = atoi(texIndexSrt.c_str());         //��������������Ϣ��ֵ
                    tempFace.vertex[i][2] = atoi(norIndexSrt.c_str());         //������Ϣ��ֵ
                }
                mFace.push_back(tempFace);
            }
        }   //end ��0��
    }  //end while
    delete fileContent;

    getAABB();//����AABB��������С��
}

/*

GLuint CreateTexture2DFromBMP(const char* bmpPath) {
    int nFileSize = 0;
    unsigned char* bmpFileContent = LoadFileContent(bmpPath, nFileSize);
    if (bmpFileContent == nullptr) {
        return 0;
    }
    int bmpWidth = 0, bmpHeight = 0;
    unsigned char* pixelData = DecodeBMP(bmpFileContent, bmpWidth, bmpHeight);

    if (bmpWidth == 0) {
        delete bmpFileContent;
        return 0;
    }
    GLuint	texture = CreateTexture2D(pixelData, bmpWidth, bmpHeight, GL_RGB);
    delete	bmpFileContent;
    return texture;
}
unsigned char* DecodeBMP(unsigned char* bmpFileData, int& width, int& height) {
    if (0x4D42 == *((unsigned short*)bmpFileData)) {
        int pixelDataOffset = *((int*)(bmpFileData + 10));
        //�˴�ƫ�Ƶ�ַ�󣬾���λͼ���������� �ĵ�ַ
        width = *((int*)(bmpFileData + 18));
        height = *((int*)(bmpFileData + 22));
        unsigned char* pixelData = bmpFileData + pixelDataOffset;
        for (int i = 0; i < width * height * 3; i += 3) {
            unsigned	char temp = pixelData[i];
            pixelData[i] = pixelData[i + 2];
            pixelData[i + 2] = temp;
        }

        return pixelData;
    }
    return nullptr;
}
GLuint CreateTexture2D(unsigned char* pixelData, int width, int height, GLenum type) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //����2d���� ������Ŵ�ʱ��ʲô�㷨�ɼ����أ� ʹ�����Թ���
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);



    //���úú󣬽����������ϴ����Կ��� , �ڴ�->�Դ�
    glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, pixelData);

    //�ڶ������� ,bitmapLevel���ָ���  0����ͬ�������������Ϊ�������ɫ,����:����0������128*128  ��ô1��������Ϊ64*64 ,2 ��������Ϊ32*32 ����ͼ�δ�С��64*64�Ϳ��Դ�1ȡ
    //��Ȼ��ռ�ݹ����ڴ档
    //�����������������������Կ����Ǻ������ظ�ʽ
    //���ģ�����������������ݿ�ߣ�
    //����������д0
    //���߸��������������ڴ�����ʲô��ʽ
    //�ڰ˸���ÿһ�����������У�ÿһ��������ʲô��������
    //���һ���� pixelData�� ������������������λ�ÿ������Կ��ϣ���Ȼ��������Ҳ����

    glBindTexture(GL_TEXTURE_2D, 0);
    //��ʼ������ֹ֮����в���Ӱ�쵱ǰ������
    return texture;
}

*/

/*
#define BMP_Header_Length 54  //ͼ���������ڴ���е�ƫ����
static GLfloat angle = 0.0f;   //��ת�Ƕ�

// ����power_of_two�����ж�һ�������ǲ���2����������
int power_of_two(int n)
{
    if (n <= 0)
        return 0;
    return (n & (n - 1)) == 0;//����������ſᰡ
}






GLuint load_texture(const char* file_name)
{
    GLint width, height, total_bytes;
    GLubyte* pixels = 0;
    GLuint last_texture_ID = 0, texture_ID = 0;

    // ���ļ������ʧ�ܣ�����
    FILE* pFile = fopen(file_name, "rb");
    if (pFile == 0)
        return 0;

    // ��ȡ�ļ���ͼ��Ŀ�Ⱥ͸߶�
    fseek(pFile, 0x0012, SEEK_SET);//ƫ��18�ֽں��ǿ��
    fread(&width, 4, 1, pFile);
    fread(&height, 4, 1, pFile);
    fseek(pFile, BMP_Header_Length, SEEK_SET);
    cout << "width=" << width << endl;
    cout << "height=" << height << endl;
    // ����ÿ��������ռ�ֽ����������ݴ����ݼ����������ֽ���
    {
        GLint line_bytes = width * 3;
        while (line_bytes % 4 != 0)//Ҫ���ͼ���ȵ�λ���Ƿ���4����������������ǣ���Ҫ���룬�����Բ������ڴ���·����ڴ档
            ++line_bytes;
        total_bytes = line_bytes * height;
    }

    // �����������ֽ��������ڴ�
    pixels = (GLubyte*)malloc(total_bytes);
    if (pixels == 0)
    {
        fclose(pFile);
        return 0;
    }

    // ��ȡ��������
    if (fread(pixels, total_bytes, 1, pFile) <= 0)
    {
        free(pixels);
        fclose(pFile);
        return 0;
    }

    // �Ծ;ɰ汾�ļ��ݣ����ͼ��Ŀ�Ⱥ͸߶Ȳ��ǵ������η�������Ҫ��������
    // ��ͼ���߳�����OpenGL�涨�����ֵ��Ҳ����
    {
        GLint max;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
        if (!power_of_two(width)
            || !power_of_two(height)
            || width > max
            || height > max)
        {
            const GLint new_width = 256;
            const GLint new_height = 256; // �涨���ź��µĴ�СΪ�߳���������
            GLint new_line_bytes, new_total_bytes;
            GLubyte* new_pixels = 0;

            // ����ÿ����Ҫ���ֽ��������ֽ���
            new_line_bytes = new_width * 3;
            while (new_line_bytes % 4 != 0)
                ++new_line_bytes;
            new_total_bytes = new_line_bytes * new_height;

            // �����ڴ�
            new_pixels = (GLubyte*)malloc(new_total_bytes);
            if (new_pixels == 0)
            {
                free(pixels);
                fclose(pFile);
                return 0;
            }

            // ������������
            gluScaleImage(GL_RGB,
                width, height, GL_UNSIGNED_BYTE, pixels,
                new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

            // �ͷ�ԭ�����������ݣ���pixelsָ���µ��������ݣ�����������width��height
            free(pixels);
            pixels = new_pixels;
            width = new_width;
            height = new_height;
        }
    }

    // ����һ���µ�������
    glGenTextures(1, &texture_ID);
    if (texture_ID == 0)
    {
        free(pixels);
        fclose(pFile);
        return 0;
    }

    // ���µ������������������������
    // �ڰ�ǰ���Ȼ��ԭ���󶨵������ţ��Ա��������лָ�
    GLint lastTextureID = last_texture_ID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureID);
    glBindTexture(GL_TEXTURE_2D, texture_ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
        GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, lastTextureID);  //�ָ�֮ǰ�������
    free(pixels);
    return texture_ID;
}
*/




void objLoader::drawObj()
{

    glEnable(GL_TEXTURE_2D);
    for (auto faceIndex = mFace.begin(); faceIndex != mFace.end(); ++faceIndex)         //ѭ������face��Ϣ
    {
        //��ʱû��   
        /*
        float x1 = mLocation[faceIndex->vertex[0][0] - 1].Data[0];
        float y1 = mLocation[faceIndex->vertex[0][0] - 1].Data[1];
        float z1 = mLocation[faceIndex->vertex[0][0] - 1].Data[1];

        float x2 = mLocation[faceIndex->vertex[1][0] - 1].Data[0];
        float y2 = mLocation[faceIndex->vertex[1][0] - 1].Data[1];
        float z2 = mLocation[faceIndex->vertex[1][0] - 1].Data[1];

        float x3 = mLocation[faceIndex->vertex[2][0] - 1].Data[0];
        float y3 = mLocation[faceIndex->vertex[2][0] - 1].Data[1];
        float z3 = mLocation[faceIndex->vertex[2][0] - 1].Data[1];
        float A = (y3 - y1) * (z3 - z1) - (z2 - z1) * (y3 - y1);
        float B = (x3 - x1) * (z2 - z1) - (x2 - x1) * (z3 - z1);
        float C = (x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1);
        float D = -(A * x1 + B * y1 + C * z1);
         */
        GLfloat pinmian[] = { 0,0,0,0 };
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        //glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        
        //glTexGenfv(GL_S, GL_OBJECT_PLANE, pinmian);//��ʱû��
        //glTexGenfv(GL_T, GL_OBJECT_PLANE, pinmian);//��ʱû��
        //glTexGenfv(GL_Q, GL_OBJECT_PLANE, pinmian);

        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        //glEnable(GL_TEXTURE_GEN_Q);


        //ÿ�����������������
        glBegin(GL_TRIANGLES);
        //��һ����ķ��ߣ�λ����Ϣ
        glNormal3fv(mNormal[faceIndex->vertex[0][2]-1].Data);
        //glTexCoord2f(0, 0);
        glVertex3fv(mLocation[faceIndex->vertex[0][0]-1].Data);
 

        //�ڶ�����ķ��ߣ�λ����Ϣ
        glNormal3fv(mNormal[faceIndex->vertex[1][2]-1].Data);
        //glTexCoord2f(0.5, 0.5);
        glVertex3fv(mLocation[faceIndex->vertex[1][0]-1].Data);

        //��������ķ��ߣ�λ����Ϣ
        glNormal3fv(mNormal[faceIndex->vertex[2][2]-1].Data);
        //glTexCoord2f(1, 0);
        glVertex3fv(mLocation[faceIndex->vertex[2][0]-1].Data);
        glEnd();
        
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
    }


    glDisable(GL_TEXTURE_2D);


}
void objLoader::setTextureFromBmp(const char* texFileName)//�������
{
    /*
    //mTexture = CreateTexture2DFromBMP(texFileName);

    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    // Ϊ��ǰ�󶨵�����������û��ơ����˷�ʽ
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // ���ز���������
    int width2, height2, nrChannels2;
    unsigned char* data2 = stbi_load("purple.bmp", &width2, &height2, &nrChannels2, 0);
    
    if (data2)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);
        //glGenerateMipmap(GL_TEXTURE_2D);//���ɶ༶��Զ����
       // gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width2, height2, GL_RGBA, GL_UNSIGNED_BYTE, data2);
    }
    else
    {
        std::cout << "��������ʧ��" << std::endl;
    }
    stbi_image_free(data2);//�ͷ�
    */
}


//��AABB��Χ��
void objLoader::drawAABB()
{

    glBegin(GL_QUADS);
    //x max ��x����������
    glVertex3f(vMax.Data[0], vMax.Data[1], vMax.Data[2]);
    glVertex3f(vMax.Data[0], vMin.Data[1], vMax.Data[2]);
    glVertex3f(vMax.Data[0], vMin.Data[1], vMin.Data[2]);
    glVertex3f(vMax.Data[0], vMax.Data[1], vMin.Data[2]);

    //x min ��x������С����
    glVertex3f(vMin.Data[0], vMin.Data[1], vMin.Data[2]);
    glVertex3f(vMin.Data[0], vMax.Data[1], vMin.Data[2]);
    glVertex3f(vMin.Data[0], vMax.Data[1], vMax.Data[2]);
    glVertex3f(vMin.Data[0], vMin.Data[1], vMax.Data[2]);

    //y max ��y����������
    glVertex3f(vMax.Data[0], vMax.Data[1], vMax.Data[2]);
    glVertex3f(vMax.Data[0], vMax.Data[1], vMin.Data[2]);
    glVertex3f(vMin.Data[0], vMax.Data[1], vMin.Data[2]);
    glVertex3f(vMin.Data[0], vMax.Data[1], vMax.Data[2]);


    //y min ��y������С����
    glVertex3f(vMin.Data[0], vMin.Data[1], vMin.Data[2]);
    glVertex3f(vMin.Data[0], vMin.Data[1], vMax.Data[2]);
    glVertex3f(vMax.Data[0], vMin.Data[1], vMax.Data[2]);
    glVertex3f(vMax.Data[0], vMin.Data[1], vMin.Data[2]);

    //z max ��z����������
    glVertex3f(vMax.Data[0], vMax.Data[1], vMax.Data[2]);
    glVertex3f(vMin.Data[0], vMax.Data[1], vMax.Data[2]);
    glVertex3f(vMin.Data[0], vMin.Data[1], vMax.Data[2]);
    glVertex3f(vMax.Data[0], vMin.Data[1], vMax.Data[2]);

    //z min ��z������С����
    glVertex3f(vMin.Data[0], vMin.Data[1], vMin.Data[2]);
    glVertex3f(vMax.Data[0], vMin.Data[1], vMin.Data[2]);
    glVertex3f(vMax.Data[0], vMax.Data[1], vMin.Data[2]);
    glVertex3f(vMin.Data[0], vMax.Data[1], vMin.Data[2]);


    glEnd();

}

//��������
double objLoader::getArea()
{
    if (area != 0.0)
    {
        return area; //�����ظ�����
    }
    else {
          
        for (auto faceIndex = mFace.begin(); faceIndex != mFace.end(); ++faceIndex)         //ѭ������face��Ϣ
        {
            area += countTriangleArea(mLocation[faceIndex->vertex[0][0]-1], mLocation[faceIndex->vertex[1][0]-1], mLocation[faceIndex->vertex[2][0]-1]);//��ÿ�����ϵ������������ɵ�������������
        }
        return area; //���������
    }
}

//�������
double objLoader::getVolume()
{
    if (volume != 0.0)
    {
        return volume; 
    }
    else {
        for (auto faceIndex = mFace.begin(); faceIndex != mFace.end(); ++faceIndex) {
            double temp = mNormal[faceIndex->vertex[0][2]-1].Data[1] + mNormal[faceIndex->vertex[1][2]-1].Data[1] + mNormal[faceIndex->vertex[2][2]-1].Data[1];//�淨�ߵ�y����
            if (temp != 0)
            {
                volume += countPillarVolume(mLocation[faceIndex->vertex[0][0]-1], mLocation[faceIndex->vertex[1][0]-1], mLocation[faceIndex->vertex[2][0]-1]) * abs(temp) / temp;//��ÿ����������ymin��Χ�ɵļ�������ἰ���
            }
        }
        return volume;//�������ֵ
    }
}

//�����������;
double objLoader::countTriangleArea(Float3 a, Float3 b, Float3 c)
{
    double area = -1;
    double side[3];//�洢�����ߵĳ���;

    side[0] = sqrt(pow(a.Data[0] - b.Data[0], 2) + pow(a.Data[1] - b.Data[1], 2) + pow(a.Data[2] - b.Data[2], 2));
    side[1] = sqrt(pow(a.Data[0] - c.Data[0], 2) + pow(a.Data[1] - c.Data[1], 2) + pow(a.Data[2] - c.Data[2], 2));
    side[2] = sqrt(pow(c.Data[0] - b.Data[0], 2) + pow(c.Data[1] - b.Data[1], 2) + pow(c.Data[2] - b.Data[2], 2));

    //���ܹ���������;
    if (side[0] + side[1] <= side[2] || side[0] + side[2] <= side[1] || side[1] + side[2] <= side[0]) return area;

    //���ú��׹�ʽ:s=sqr(p*(p-a)(p-b)(p-c))�����������; 
    double p = (side[0] + side[1] + side[2]) / 2; //���ܳ�;
    area = sqrt(p * (p - side[0]) * (p - side[1]) * (p - side[2]));

    return area;
}

//����ÿ��������������ymin����ͶӰ���ɵļ���������
double objLoader::countPillarVolume(Float3 a, Float3 b, Float3 c)
{
    Float3 maxp;
    Float3 minp;
    Float3 cenp;
    Float3 maxd;
    Float3 mind;
    Float3 cend;
    if (a.Data[1] >= b.Data[1])
    {
        if (a.Data[1] >= c.Data[1])
        {
            maxp = a;
            if (b.Data[1] >= c.Data[1])
            {
                cenp = b;
                minp = c;
            }
            else
            {
                cenp = c;
                minp = b;
            }
        }
    }
    if (b.Data[1] >= a.Data[1])
    {
        if (b.Data[1] >= c.Data[1])
        {
            maxp = b;
            if (a.Data[1] >= c.Data[1])
            {
                cenp = a;
                minp = c;
            }
            else
            {
                cenp = c;
                minp = a;
            }
        }
    }
    if (c.Data[1] >= a.Data[1])
    {
        if (c.Data[1] >= b.Data[1])
        {
            maxp = c;
            if (a.Data[1] >= b.Data[1])
            {
                cenp = a;
                minp = b;
            }
            else
            {
                cenp = b;
                minp = a;
            }
        }
    }

    maxd.Data[0] = maxp.Data[0];
    maxd.Data[1] = vMin.Data[1];
    maxd.Data[2] = maxp.Data[2];

    cend.Data[0] = cenp.Data[0];
    cend.Data[1] = vMin.Data[1];
    cend.Data[2] = cenp.Data[2];

    mind.Data[0] = minp.Data[0];
    mind.Data[1] = vMin.Data[1];
    mind.Data[2] = minp.Data[2];

    double tempArea = countTriangleArea(maxd, cend, mind);
    double volume1 = tempArea * (minp.Data[1] - mind.Data[1]);
    double volume2 = (maxp.Data[1] + cenp.Data[1] - 2 * minp.Data[1]) * tempArea / 3;
    return volume1 + volume2;
}

//����ģ��AABB��Χ��ʱ����������С��
void objLoader::getAABB()
{
    vMax.Data[0] = -999;
    vMax.Data[1] = -999;
    vMax.Data[2] = -999;
    vMin.Data[0] = 999;
    vMin.Data[1] = 999;
    vMin.Data[2] = 999;

    for (int i = 0; i < (int)mLocation.size(); i++)
    {
        //����vMax
        if (vMax.Data[0] < mLocation[i].Data[0])
        {
            vMax.Data[0] = mLocation[i].Data[0];
        }
        if (vMax.Data[1] < mLocation[i].Data[1])
        {
            vMax.Data[1] = mLocation[i].Data[1];
        }
        if (vMax.Data[2] < mLocation[i].Data[2])
        {
            vMax.Data[2] = mLocation[i].Data[2];
        }

        //����vMin
        if (vMin.Data[0] > mLocation[i].Data[0])
        {
            vMin.Data[0] = mLocation[i].Data[0];
        }
        if (vMin.Data[1] > mLocation[i].Data[1])
        {
            vMin.Data[1] = mLocation[i].Data[1];
        }
        if (vMin.Data[2] > mLocation[i].Data[2])
        {
            vMin.Data[2] = mLocation[i].Data[2];
        }
    }
}