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

static double pi = 3.1415926; //ʵ���ƶ����۲�ģ���������
bool mouseLeftDown;
bool mouseRightDown;//mouseLeftDown�� mouseRightDown������������Ҽ��������

float mouseX, mouseY;//mouseX��mouseY��������껬��ʱ��ǰһ�̵��������λ��
//cameraDistance���������������Ҽ����²����»��������Ƴ���������������Ļ��Զ��
float cameraDistance = 4.0f;
//cameraAngleX��cameraAngleY�����������������²����������Ƴ������������ת��
float cameraAngleX;
float cameraAngleY;

// �������
GLfloat obj_color[] = { 0.5f, 0.5f, 0.5f, 1.0f };//objģ��
GLfloat AABB_color[] = { 1.0f, 1.0f, 1.0f, 0.5f };//AABB��Χ��
GLfloat blue_color[] = { 0.0f, 0.0f, 1.0f, 0.5f };//x��
GLfloat green_color[] = { 0.0f, 1.0f, 0.0f, 0.5f };//y��
GLfloat red_color[] = { 1.0f, 0.0f, 0.0f, 0.5f };//z��

GLfloat mat_specular[] = { 0,0,0.1,2.0 };
GLfloat mat_emission[] = { 0,0,0,2.0 };
GLfloat mat_shininess = 60.0f;

//��Դ
void setLightRes() {
    GLfloat light_position[] = { 0.0f, 1.0f, 1.0f, 1.0f };//��Դλ������ 
    GLfloat light_ambient[] = { 0.0f, 0.0f, 0.1f, 0.5f };//��Դ������ǿ������
    GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };//��Դɢ���ǿ������  
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };//��Դ���淴���ǿ������ 

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

//�ƶ����360�۲�ģ��
void moseMove(int button, int state, int x, int y) {
    mouseX = x;
    mouseY = y;

    if (button == GLUT_LEFT_BUTTON)//������
    {
        if (state == GLUT_DOWN)
        {
            mouseLeftDown = true;
        }
        else if (state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if (button == GLUT_RIGHT_BUTTON)//����Ҽ�
    {
        if (state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if (state == GLUT_UP)
            mouseRightDown = false;
    }
}


void changeViewPoint(int x, int y) {//�ı�۲��
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
    glutPostRedisplay();// ��ǵ�ǰ������Ҫ���»��ơ�ͨ��glutMainLoop��һ��ѭ��ʱ��������ʾ�����ص���������ʾ���ڵ�������塣
    Sleep(50);
}

void setTexture() {
    glBindTexture(GL_TEXTURE_2D, texture);
    glGenTextures(1, &texture);


    // Ϊ��ǰ�󶨵�����������û��ơ����˷�ʽ
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // ���ز���������
    int width2, height2, nrChannels2;

    unsigned char* data2 = stbi_load("sun.jpg", &width2, &height2, &nrChannels2, 0);
    //std::cout << data2 << std::endl;

    if (data2)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);

        //glGenerateMipmap(GL_TEXTURE_2D);//���ɶ༶��Զ����
        //gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width2, height2, GL_RGBA, GL_UNSIGNED_BYTE, data2);
    }
    else
    {
        std::cout << "��������ʧ��" << std::endl;
    }

    stbi_image_free(data2);//�ͷ�ͼƬ����
}

void delTexture() {
    glBindTexture(GL_TEXTURE_2D,0);

    glDeleteTextures(1, &texture);

}




void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ������ϲ����û������
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(cameraDistance * (sin(cameraAngleY * pi / 180)) * (cos(cameraAngleX * pi / 180)),
        cameraDistance * (sin(cameraAngleY * pi / 180)) * (sin(cameraAngleX * pi / 180)),
        cameraDistance * cos(cameraAngleY * pi / 180),
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    setLightRes();


    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, obj_color);//���ʵĻ�����ɫ��ɢ����ɫ
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);//���ʵľ��淴����ɫ
    glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);//���ʵķ������ɫ
    glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);//���淴��ָ��

    glTranslatef(0.2, 0.0, 0.2);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.1, 0.0, 0.1);
    glRotatef(2 * angle, 0.0f, -1.0f, 0.0f);

    objModel.drawObj();
    glBindTexture(GL_TEXTURE_2D, 0);
    //����objģ��

    //���潫���ư�͸�������ˣ���˽���Ȼ�������Ϊֻ��
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

    //������Ϊ������
    glLineWidth(30.0f);

    glBegin(GL_LINES);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red_color);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 1.0, 0.0);
    glEnd();


    glPopMatrix();
    // ��ɰ�͸������Ļ��ƣ�����Ȼ������ָ�Ϊ�ɶ���д����ʽ
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
    glutDisplayFunc(display);//����
    glutReshapeFunc(&reshape);//�����ȿ��ƴ��ڴ�С�ı�ʱ�Ĳ���
    glutIdleFunc(&myIdle); //�����ٿ��ƴ��������¼�����ʱ�Ĳ���
    glutKeyboardFunc(keyaction);
    glutMouseFunc(&moseMove);//��꽻������ 
    glutMotionFunc(&changeViewPoint);//׷��passive motion

    printf("bunny�ı����Ϊ��%f   ", objModel.getArea());
    printf("bunny���Ϊ��%f    ", objModel.getVolume());
    

    glutMainLoop();
    return 0;
}
