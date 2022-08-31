
#include "stb_image.h"
#include <iostream>
#include "objLoader.h"
#include <fstream>

#include <sstream>

using namespace std;

objLoader::objLoader(const char * objFileName)
{

    int nFileSize = 0;
    unsigned char* fileContent = LoadFileContent(objFileName, nFileSize);    //读取文件内容
    if (fileContent == nullptr)     //文件为空 
    {
        return;
    }
    stringstream ssFileContent((char*)fileContent);   //流读取文件内容
    string temp;       //接受无关信息
    char szoneLine[256];        //读取一行的数据
    while (!ssFileContent.eof())
    {
        memset(szoneLine, 0, 256);        //  每次循环初始化数组szoneLine
        ssFileContent.getline(szoneLine, 256);      //流读取一行
        if (strlen(szoneLine) > 0)       //该行不为空
        {
            if (szoneLine[0] == 'v')     //v开头的数据
            {
                stringstream ssOneLine(szoneLine);        //数据存储到流中 方便赋值
                if (szoneLine[1] == 't')       //纹理信息
                {
                    ssOneLine >> temp;     //接受标识符 vt
                    Float2 tempTexcoord;
                    ssOneLine >> tempTexcoord.Data[0] >> tempTexcoord.Data[1];   //数据存入临时变量中
                    mTexcoord.push_back(tempTexcoord);         //存入容器

                }
                if (szoneLine[1] == 'n')            //法线信息
                {
                    ssOneLine >> temp;      //接收标识符vn
                    Float3 tempNormal;
                    ssOneLine >> tempNormal.Data[0] >> tempNormal.Data[1] >> tempNormal.Data[2];
                    mNormal.push_back(tempNormal);
                }
                else                          //点的位置信息
                {
                    ssOneLine >> temp;
                    Float3 tempLocation;
                    ssOneLine >> tempLocation.Data[0] >> tempLocation.Data[1] >> tempLocation.Data[2];
                    mLocation.push_back(tempLocation);
                }
            }
            else if (szoneLine[0] == 'f')          //面信息
            {
                stringstream ssOneLine(szoneLine);     //流读取一行数据
                ssOneLine >> temp; //接收标识符f
                //    f信息    exp： f 1/1/1 2/2/2 3/3/3      位置索引/纹理索引/法线索引   三角面片 三个点构成一个面
                string vertexStr;   //接收流的内容
                Face tempFace;
                for (int i = 0; i < 3; ++i)         //每个面三个点
                {
                    ssOneLine >> vertexStr;           //从流中读取点的索引信息
                    size_t pos = vertexStr.find_first_of('/');       //找到第一个/的位置      //即找到点的位置信息
                    string locIndexStr = vertexStr.substr(0, pos);       //赋值点的位置信息
                    size_t pos2 = vertexStr.find_first_of('/', pos + 1);   //找到第二个/   即找到点的纹理坐标信息
                    string texIndexSrt = vertexStr.substr(pos + 1, pos2 - 1 - pos);       //赋值点的纹理坐标信息
                    string norIndexSrt = vertexStr.substr(pos2 + 1, vertexStr.length() - 1 - pos2);   //赋值点的法线信息
                    tempFace.vertex[i][0] = atoi(locIndexStr.c_str());        //将索引信息从 srting转换为 int     //位置索引信息赋值
                    tempFace.vertex[i][1] = atoi(texIndexSrt.c_str());         //纹理坐标索引信息赋值
                    tempFace.vertex[i][2] = atoi(norIndexSrt.c_str());         //法线信息赋值
                }
                mFace.push_back(tempFace);
            }
        }   //end 非0行
    }  //end while
    delete fileContent;

    getAABB();//计算AABB最大点与最小点
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
        //此处偏移地址后，就是位图中像素数据 的地址
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
    //设置2d纹理， 当纹理放大时用什么算法采集像素， 使用线性过滤
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);



    //设置好后，将像素数据上传至显卡上 , 内存->显存
    glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, pixelData);

    //第二个参数 ,bitmapLevel这种概念  0，不同级别的像素数据为多边形着色,级别:比如0代表了128*128  那么1可以设置为64*64 ,2 可以设置为32*32 。当图形大小是64*64就可以从1取
    //当然会占据过大内存。
    //第三个参数，纹理数据在显卡上是何种像素格式
    //第四，五个参数，像素数据宽高，
    //第六个必须写0
    //第七个，纹理数据在内存上是什么格式
    //第八个，每一个像素数据中，每一个分量是什么样的类型
    //最后一个， pixelData， 像素数据在哪里。从这个位置拷贝到显卡上，当然换个名字也可以

    glBindTexture(GL_TEXTURE_2D, 0);
    //初始化，防止之后会有操作影响当前的纹理
    return texture;
}

*/

/*
#define BMP_Header_Length 54  //图像数据在内存块中的偏移量
static GLfloat angle = 0.0f;   //旋转角度

// 函数power_of_two用于判断一个整数是不是2的整数次幂
int power_of_two(int n)
{
    if (n <= 0)
        return 0;
    return (n & (n - 1)) == 0;//这个方法很炫酷啊
}






GLuint load_texture(const char* file_name)
{
    GLint width, height, total_bytes;
    GLubyte* pixels = 0;
    GLuint last_texture_ID = 0, texture_ID = 0;

    // 打开文件，如果失败，返回
    FILE* pFile = fopen(file_name, "rb");
    if (pFile == 0)
        return 0;

    // 读取文件中图象的宽度和高度
    fseek(pFile, 0x0012, SEEK_SET);//偏移18字节后是宽高
    fread(&width, 4, 1, pFile);
    fread(&height, 4, 1, pFile);
    fseek(pFile, BMP_Header_Length, SEEK_SET);
    cout << "width=" << width << endl;
    cout << "height=" << height << endl;
    // 计算每行像素所占字节数，并根据此数据计算总像素字节数
    {
        GLint line_bytes = width * 3;
        while (line_bytes % 4 != 0)//要检测图像宽度的位数是否是4的整数倍，如果不是，需要补齐，并且以补齐后的内存带下分配内存。
            ++line_bytes;
        total_bytes = line_bytes * height;
    }

    // 根据总像素字节数分配内存
    pixels = (GLubyte*)malloc(total_bytes);
    if (pixels == 0)
    {
        fclose(pFile);
        return 0;
    }

    // 读取像素数据
    if (fread(pixels, total_bytes, 1, pFile) <= 0)
    {
        free(pixels);
        fclose(pFile);
        return 0;
    }

    // 对就旧版本的兼容，如果图象的宽度和高度不是的整数次方，则需要进行缩放
    // 若图像宽高超过了OpenGL规定的最大值，也缩放
    {
        GLint max;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
        if (!power_of_two(width)
            || !power_of_two(height)
            || width > max
            || height > max)
        {
            const GLint new_width = 256;
            const GLint new_height = 256; // 规定缩放后新的大小为边长的正方形
            GLint new_line_bytes, new_total_bytes;
            GLubyte* new_pixels = 0;

            // 计算每行需要的字节数和总字节数
            new_line_bytes = new_width * 3;
            while (new_line_bytes % 4 != 0)
                ++new_line_bytes;
            new_total_bytes = new_line_bytes * new_height;

            // 分配内存
            new_pixels = (GLubyte*)malloc(new_total_bytes);
            if (new_pixels == 0)
            {
                free(pixels);
                fclose(pFile);
                return 0;
            }

            // 进行像素缩放
            gluScaleImage(GL_RGB,
                width, height, GL_UNSIGNED_BYTE, pixels,
                new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

            // 释放原来的像素数据，把pixels指向新的像素数据，并重新设置width和height
            free(pixels);
            pixels = new_pixels;
            width = new_width;
            height = new_height;
        }
    }

    // 分配一个新的纹理编号
    glGenTextures(1, &texture_ID);
    if (texture_ID == 0)
    {
        free(pixels);
        fclose(pFile);
        return 0;
    }

    // 绑定新的纹理，载入纹理并设置纹理参数
    // 在绑定前，先获得原来绑定的纹理编号，以便在最后进行恢复
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
    glBindTexture(GL_TEXTURE_2D, lastTextureID);  //恢复之前的纹理绑定
    free(pixels);
    return texture_ID;
}
*/




void objLoader::drawObj()
{

    glEnable(GL_TEXTURE_2D);
    for (auto faceIndex = mFace.begin(); faceIndex != mFace.end(); ++faceIndex)         //循环遍历face信息
    {
        //暂时没用   
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
        
        //glTexGenfv(GL_S, GL_OBJECT_PLANE, pinmian);//暂时没用
        //glTexGenfv(GL_T, GL_OBJECT_PLANE, pinmian);//暂时没用
        //glTexGenfv(GL_Q, GL_OBJECT_PLANE, pinmian);

        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        //glEnable(GL_TEXTURE_GEN_Q);


        //每组三个点绘制三角形
        glBegin(GL_TRIANGLES);
        //第一个点的法线，位置信息
        glNormal3fv(mNormal[faceIndex->vertex[0][2]-1].Data);
        //glTexCoord2f(0, 0);
        glVertex3fv(mLocation[faceIndex->vertex[0][0]-1].Data);
 

        //第二个点的法线，位置信息
        glNormal3fv(mNormal[faceIndex->vertex[1][2]-1].Data);
        //glTexCoord2f(0.5, 0.5);
        glVertex3fv(mLocation[faceIndex->vertex[1][0]-1].Data);

        //第三个点的法线，位置信息
        glNormal3fv(mNormal[faceIndex->vertex[2][2]-1].Data);
        //glTexCoord2f(1, 0);
        glVertex3fv(mLocation[faceIndex->vertex[2][0]-1].Data);
        glEnd();
        
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
    }


    glDisable(GL_TEXTURE_2D);


}
void objLoader::setTextureFromBmp(const char* texFileName)//纹理相关
{
    /*
    //mTexture = CreateTexture2DFromBMP(texFileName);

    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    // 为当前绑定的纹理对象设置环绕、过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 加载并生成纹理
    int width2, height2, nrChannels2;
    unsigned char* data2 = stbi_load("purple.bmp", &width2, &height2, &nrChannels2, 0);
    
    if (data2)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);
        //glGenerateMipmap(GL_TEXTURE_2D);//生成多级渐远纹理
       // gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width2, height2, GL_RGBA, GL_UNSIGNED_BYTE, data2);
    }
    else
    {
        std::cout << "加载纹理失败" << std::endl;
    }
    stbi_image_free(data2);//释放
    */
}


//画AABB包围盒
void objLoader::drawAABB()
{

    glBegin(GL_QUADS);
    //x max 画x坐标最大的面
    glVertex3f(vMax.Data[0], vMax.Data[1], vMax.Data[2]);
    glVertex3f(vMax.Data[0], vMin.Data[1], vMax.Data[2]);
    glVertex3f(vMax.Data[0], vMin.Data[1], vMin.Data[2]);
    glVertex3f(vMax.Data[0], vMax.Data[1], vMin.Data[2]);

    //x min 画x坐标最小的面
    glVertex3f(vMin.Data[0], vMin.Data[1], vMin.Data[2]);
    glVertex3f(vMin.Data[0], vMax.Data[1], vMin.Data[2]);
    glVertex3f(vMin.Data[0], vMax.Data[1], vMax.Data[2]);
    glVertex3f(vMin.Data[0], vMin.Data[1], vMax.Data[2]);

    //y max 画y坐标最大的面
    glVertex3f(vMax.Data[0], vMax.Data[1], vMax.Data[2]);
    glVertex3f(vMax.Data[0], vMax.Data[1], vMin.Data[2]);
    glVertex3f(vMin.Data[0], vMax.Data[1], vMin.Data[2]);
    glVertex3f(vMin.Data[0], vMax.Data[1], vMax.Data[2]);


    //y min 画y坐标最小的面
    glVertex3f(vMin.Data[0], vMin.Data[1], vMin.Data[2]);
    glVertex3f(vMin.Data[0], vMin.Data[1], vMax.Data[2]);
    glVertex3f(vMax.Data[0], vMin.Data[1], vMax.Data[2]);
    glVertex3f(vMax.Data[0], vMin.Data[1], vMin.Data[2]);

    //z max 画z坐标最大的面
    glVertex3f(vMax.Data[0], vMax.Data[1], vMax.Data[2]);
    glVertex3f(vMin.Data[0], vMax.Data[1], vMax.Data[2]);
    glVertex3f(vMin.Data[0], vMin.Data[1], vMax.Data[2]);
    glVertex3f(vMax.Data[0], vMin.Data[1], vMax.Data[2]);

    //z min 画z坐标最小的面
    glVertex3f(vMin.Data[0], vMin.Data[1], vMin.Data[2]);
    glVertex3f(vMax.Data[0], vMin.Data[1], vMin.Data[2]);
    glVertex3f(vMax.Data[0], vMax.Data[1], vMin.Data[2]);
    glVertex3f(vMin.Data[0], vMax.Data[1], vMin.Data[2]);


    glEnd();

}

//计算表面积
double objLoader::getArea()
{
    if (area != 0.0)
    {
        return area; //避免重复计算
    }
    else {
          
        for (auto faceIndex = mFace.begin(); faceIndex != mFace.end(); ++faceIndex)         //循环遍历face信息
        {
            area += countTriangleArea(mLocation[faceIndex->vertex[0][0]-1], mLocation[faceIndex->vertex[1][0]-1], mLocation[faceIndex->vertex[2][0]-1]);//将每个面上的三个点所构成的三角形面积相加
        }
        return area; //将结果返回
    }
}

//计算体积
double objLoader::getVolume()
{
    if (volume != 0.0)
    {
        return volume; 
    }
    else {
        for (auto faceIndex = mFace.begin(); faceIndex != mFace.end(); ++faceIndex) {
            double temp = mNormal[faceIndex->vertex[0][2]-1].Data[1] + mNormal[faceIndex->vertex[1][2]-1].Data[1] + mNormal[faceIndex->vertex[2][2]-1].Data[1];//面法线的y方向
            if (temp != 0)
            {
                volume += countPillarVolume(mLocation[faceIndex->vertex[0][0]-1], mLocation[faceIndex->vertex[1][0]-1], mLocation[faceIndex->vertex[2][0]-1]) * abs(temp) / temp;//将每个三角形与ymin面围成的几何体的提及相加
            }
        }
        return volume;//返回体积值
    }
}

//求三角形面积;
double objLoader::countTriangleArea(Float3 a, Float3 b, Float3 c)
{
    double area = -1;
    double side[3];//存储三条边的长度;

    side[0] = sqrt(pow(a.Data[0] - b.Data[0], 2) + pow(a.Data[1] - b.Data[1], 2) + pow(a.Data[2] - b.Data[2], 2));
    side[1] = sqrt(pow(a.Data[0] - c.Data[0], 2) + pow(a.Data[1] - c.Data[1], 2) + pow(a.Data[2] - c.Data[2], 2));
    side[2] = sqrt(pow(c.Data[0] - b.Data[0], 2) + pow(c.Data[1] - b.Data[1], 2) + pow(c.Data[2] - b.Data[2], 2));

    //不能构成三角形;
    if (side[0] + side[1] <= side[2] || side[0] + side[2] <= side[1] || side[1] + side[2] <= side[0]) return area;

    //利用海伦公式:s=sqr(p*(p-a)(p-b)(p-c))求三角形面积; 
    double p = (side[0] + side[1] + side[2]) / 2; //半周长;
    area = sqrt(p * (p - side[0]) * (p - side[1]) * (p - side[2]));

    return area;
}

//计算每个三角形与其在ymin面上投影构成的几何体的体积
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

//计算模型AABB包围盒时的最大点与最小点
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
        //计算vMax
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

        //计算vMin
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