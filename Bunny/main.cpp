#include "objLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

using namespace std;
unsigned char* LoadFileContent(const char * path, int& filesize) {
    unsigned char* fileContent = nullptr;
    filesize = 0;
    FILE* pFile = fopen(path, "rb");
    if (pFile) {
        fseek(pFile, 0, SEEK_END);
        int nLen = ftell(pFile);
        if (nLen > 0) {
            rewind(pFile);
            fileContent = new unsigned char[nLen + 1];
            fread(fileContent, nLen, sizeof(unsigned char), pFile);
            fileContent[nLen] = '\0';
            filesize = nLen;
        }
        fclose(pFile);
    }
    return fileContent;
}

unsigned int texture;
int width = 1000;
int height = 1000;

static GLfloat angle = 0.0f;
objLoader objModel = objLoader("bunny.obj");

static double pi = 3.1415926; //实现移动鼠标观察模型所需变量
bool mouseLeftDown;
bool mouseRightDown;//mouseLeftDown和 mouseRightDown变量是鼠标左右键按下与否

float mouseX, mouseY;//mouseX和mouseY变量是鼠标滑动时，前一刻的鼠标所在位置
//cameraDistance这个变量根据鼠标右键按下并上下滑动来控制场景中物体移入屏幕的远近
float cameraDistance = 4.0f;
//cameraAngleX和cameraAngleY变量根据鼠标左键按下并滑动来控制场景中物体的旋转。
float cameraAngleX;
float cameraAngleY;

// 定义材质
GLfloat obj_color[] = { 0.5f, 0.5f, 0.5f, 1.0f };//obj模型
GLfloat AABB_color[] = { 1.0f, 1.0f, 1.0f, 0.5f };//AABB包围盒
GLfloat blue_color[] = { 0.0f, 0.0f, 1.0f, 0.5f };//x轴
GLfloat green_color[] = { 0.0f, 1.0f, 0.0f, 0.5f };//y轴
GLfloat red_color[] = { 1.0f, 0.0f, 0.0f, 0.5f };//z轴

GLfloat mat_specular[] = { 0,0,0.1,2.0 };
GLfloat mat_emission[] = { 0,0,0,2.0 };
GLfloat mat_shininess = 60.0f;

//光源
void setLightRes() {
    GLfloat light_position[] = { 0.0f, 1.0f, 1.0f, 1.0f };//光源位置数组 
    GLfloat light_ambient[] = { 0.0f, 0.0f, 0.1f, 0.5f };//光源环境光强度数组
    GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };//光源散射光强度数组  
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };//光源镜面反射光强度数组 

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
}





void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(12.0f, (GLdouble)width / (GLdouble)height, 1.0f, 300.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

//移动鼠标360观察模型
void moseMove(int button, int state, int x, int y) {
    mouseX = x;
    mouseY = y;

    if (button == GLUT_LEFT_BUTTON)//鼠标左键
    {
        if (state == GLUT_DOWN)
        {
            mouseLeftDown = true;
        }
        else if (state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if (button == GLUT_RIGHT_BUTTON)//鼠标右键
    {
        if (state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if (state == GLUT_UP)
            mouseRightDown = false;
    }
}


void changeViewPoint(int x, int y) {//改变观察点
    if (mouseLeftDown)
    {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
    if (mouseRightDown)
    {
        cameraDistance += (pow(pow(x, 2) + pow(y, 2), 0.5) - pow(pow(mouseX, 2) + pow(mouseY, 2), 0.5)) * 0.2f;
        mouseX = x;
        mouseY = y;
    }

    glutPostRedisplay();
}


void myIdle() {
    angle += 1.0f;
    if (angle >= 360.0f)
        angle = 0.0f;
    glutPostRedisplay();// 标记当前窗口需要重新绘制。通过glutMainLoop下一次循环时，窗口显示将被回调以重新显示窗口的正常面板。
    Sleep(50);
}

void setTexture() {
    glBindTexture(GL_TEXTURE_2D, texture);
    glGenTextures(1, &texture);


    // 为当前绑定的纹理对象设置环绕、过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 加载并生成纹理
    int width2, height2, nrChannels2;

    unsigned char* data2 = stbi_load("sun.jpg", &width2, &height2, &nrChannels2, 0);
    //std::cout << data2 << std::endl;

    if (data2)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);

        //glGenerateMipmap(GL_TEXTURE_2D);//生成多级渐远纹理
        //gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width2, height2, GL_RGBA, GL_UNSIGNED_BYTE, data2);
    }
    else
    {
        std::cout << "加载纹理失败" << std::endl;
    }

    stbi_image_free(data2);//释放图片数据
}

void delTexture() {
    glBindTexture(GL_TEXTURE_2D,0);

    glDeleteTextures(1, &texture);

}




void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 启动混合并设置混合因子
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(cameraDistance * (sin(cameraAngleY * pi / 180)) * (cos(cameraAngleX * pi / 180)),
        cameraDistance * (sin(cameraAngleY * pi / 180)) * (sin(cameraAngleX * pi / 180)),
        cameraDistance * cos(cameraAngleY * pi / 180),
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    setLightRes();


    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, obj_color);//材质的环境颜色和散射颜色
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);//材质的镜面反射颜色
    glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);//材质的发射光颜色
    glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);//镜面反射指数

    glTranslatef(0.2, 0.0, 0.2);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.1, 0.0, 0.1);
    glRotatef(2 * angle, 0.0f, -1.0f, 0.0f);

    objModel.drawObj();
    glBindTexture(GL_TEXTURE_2D, 0);
    //绘制obj模型

    //下面将绘制半透明物体了，因此将深度缓冲设置为只读
    glDepthMask(GL_FALSE);

    glPushMatrix();
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
    glMaterialf(GL_FRONT, GL_SHININESS, 30.0);

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, AABB_color);
    objModel.drawAABB();

    glRotatef(2 * angle, 0.0f, 1.0f, 0.0f);
    glTranslatef(-0.1, 0.0, -0.1);
    glRotatef(angle, 0.0f, -1.0f, 0.0f);
    glTranslatef(-0, 0.0, -0);

    //画线作为参照物
    glLineWidth(30.0f);

    glBegin(GL_LINES);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red_color);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 1.0, 0.0);
    glEnd();


    glPopMatrix();
    // 完成半透明物体的绘制，将深度缓冲区恢复为可读可写的形式
    glDepthMask(GL_TRUE);

    glutSwapBuffers();
   
}
void keyaction(unsigned char key, int x, int y)
{
    switch (key) {

    case 'T':
        setTexture();
        break;
    case 'D':
        delTexture();
        break;
    default:
        break;
    }
    glutPostRedisplay();
}



void init(int argc, char* argv[]) {

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 1000);
    glEnable(GL_DEPTH_TEST);

    glShadeModel(GL_SMOOTH);
    glutCreateWindow("Bunny");


}


int main(int argc, char* argv[]) {
    init(argc, argv);
    glutDisplayFunc(display);//绘制
    glutReshapeFunc(&reshape);//函数先控制窗口大小改变时的操作
    glutIdleFunc(&myIdle); //函数再控制窗口有无事件发生时的操作
    glutKeyboardFunc(keyaction);
    glutMouseFunc(&moseMove);//鼠标交互函数 
    glutMotionFunc(&changeViewPoint);//追踪passive motion

    printf("bunny的表面积为：%f   ", objModel.getArea());
    printf("bunny体积为：%f    ", objModel.getVolume());
    

    glutMainLoop();
    return 0;
}
