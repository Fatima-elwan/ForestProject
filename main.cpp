
#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include<windows.h>
#include<mmsystem.h>
#pragma comment(lib, "winum.lib")
#include <iostream>  // لمراقبة تحميل الملفات

using namespace std;


struct vertex { float x, y, z; };
vector<vertex> deerVertices, catVertices, horseVertices;
float deerJump = 0.0f;
float deerWalk = 0.0f;
float catWalk = 0.0f;
float horseRot = 0.0f;
bool isMoving = false; //للتحكم في جميع الحيوانات
float horseTimer = 0.0f;//للتحكم في توقيت الشبوب
float legSwing = 0.0f;
int deerDir=1;
int catDir=1;
bool isRaining = false;
float rainX[4000], rainY[4000], rainZ[4000];

void initRain(){
for(int i=0; i<4000 ; i++){
    rainX[i] = (rand() % 4000)-2000;
    rainY[i] = (rand() % 600);
    rainZ[i] = (rand() % 4000)-2000;
}
}

void loadOBJ(const char* filename, vector<vertex>& target) {
    vector<vertex> temp_vertices;
    ifstream file(filename);
    if (!file.is_open()) return;
    string line;
    while (getline(file, line)) {
        if (line.substr(0, 2) == "v ") {
            stringstream ss(line.substr(2));
            vertex v;
            ss >> v.x >> v.y >> v.z;
            temp_vertices.push_back(v);
        }
        else if (line.substr(0, 2) == "f ") {
            stringstream ss(line.substr(2));
            string segment;
            while (ss >> segment) {
                // استخراج الرقم الأول فقط قبل أي "/" لضمان عدم حدوث Crash
                int index = stoi(segment.substr(0, segment.find('/')));
                target.push_back(temp_vertices[index - 1]);
            }
        }
    }
    file.close();
}

void drawModel(const vector<vertex>& vertices) {
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < vertices.size(); i++) {
        glNormal3f(vertices[i].x, vertices[i].y, vertices[i].z);
        glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
    }
    glEnd();
}
// ------------------------------

// كم والكاميرا (كودك الأصلي)
float camX = 0.0f, camY = 40.0f, camZ = 350.0f;
float lookX = 0.0f, lookY = 20.0f, lookZ = 0.0f;
bool isDay = true;
float globalTime = 0.0f;

struct Point {
    float x, y, z;
    float sizeX, sizeY, sizeZ;
};

std::vector<Point> trees;
std::vector<Point> cloudPos;
std::vector<Point> birds;
std::vector<Point> brownRocks;
std::vector<Point> mountains;

void initWorld() {
    srand(time(0));

    //ات (إضافة فقط)
    loadOBJ("F:\\Drawing\\FirstProject\\deer.obj", deerVertices);
    loadOBJ("F:\\Drawing\\FirstProject\\cat.obj", catVertices);
    loadOBJ("F:\\Drawing\\FirstProject\\horse.obj", horseVertices);

    // توزيع الأشجار
    for (int i = 0; i < 600; i++) {
        float tx = (rand() % 1500) - 1000;
        float tz = (rand() % 1500) - 1000;
         float roadX = sin(tz * 0.02f) * 30.0f;
        bool isOnRoad = abs(tx - roadX) < 20.0f;
        float distToLake = sqrt(pow(tx - 450.0f,2) + pow(tz - 80.0f, 2));
        bool isOnLake = distToLake <180.0f ;
        bool isOnMountains = (tz < -500.0f);
        if(!isOnRoad && !isOnLake && !isOnMountains){
        float randomWidth = 0.7f + (rand() % 10)  / 2.0f;;
        float randomHeight = 0.8f + (rand() % 15) / 2.0f;;
        trees.push_back({ tx, 0, tz, randomWidth, randomHeight, randomWidth });
    }}
    for (int i = 0; i < 25; i++)
        cloudPos.push_back({ (float)((rand() % 1200) - 600), (float)(280 + rand() % 40), (float)((rand() % 1000) - 500), 3.0 , 2.0f, 2.0f });

    for (int i = 0; i < 50; i++)
        birds.push_back({ (float)(rand()%800-400), (float)(100 + rand()%50), (float)(rand()%1000-500), 0.0f, 0.0f, 0.0f });

    for (int i = 0; i < 25; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float r = 122.0f + (rand() % 10);
        float s = (float)(3.0f + (rand()%30)/10.0f);
        brownRocks.push_back({ cos(angle) * r * 1.6f, 1.0f, sin(angle) * r, s, s, s });
    }

    for (int i = 0; i < 6; i++) {
        float mx = -700.0f + (i * 250.0f);
        float mz = -650.0f - (rand() % 150);
        mountains.push_back({ mx, 0, mz, 0, 0, 0 });
    }
}

void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    if (isDay) {//قيم ساطعة
        GLfloat ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
        GLfloat diffuse[] = { 1.0f, 1.0f, 0.8f, 1.0f };
        GLfloat pos[] = { 100.0f, 350.0f, 100.0f, 1.0f };
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT0, GL_POSITION, pos);
    } else {//قيم خافتة
        GLfloat ambient[] = { 0.1f, 0.1f, 0.3f, 1.0f };
        GLfloat diffuse[] = { 0.2f, 0.2f, 0.5f, 1.0f };
        GLfloat pos[] = { -100.0f, 350.0f, -100.0f, 1.0f };
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT0, GL_POSITION, pos);
    }
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
}

void drawTree(Point& t) {
    glPushMatrix(); glTranslatef(t.x, 0, t.z); glScalef(t.sizeX, t.sizeY, t.sizeZ);
    if (isDay) {
        glDisable(GL_LIGHTING); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0, 0.1, 0.0, 0.3); glPushMatrix(); glTranslatef(2, 0.05, 2); glScalef(1.2, 0.01, 1.2); glutSolidSphere(6, 15, 15); glPopMatrix();

glDisable(GL_BLEND); glEnable(GL_LIGHTING);
    }
    //جذع الشجرة
    glColor3f(0.4f, 0.25f, 0.1f);
    glPushMatrix(); glRotatef(-90, 1, 0, 0); GLUquadric* q = gluNewQuadric(); gluCylinder(q, 2.0, 1.2, 15.0, 16, 1); glPopMatrix();
    // حركة التمايل
    float sway;
    if(!isRaining){
    sway = sin(globalTime + t.x) * 0.3f;}
    else{sway = sin(globalTime + t.x) * 1.0f;}
    //الاوراق
    glColor3f(0.0f, 0.5f, 0.0f);
    glPushMatrix(); glTranslatef(sway, 15, 0);
    for(int i=0; i<5; i++) { glPushMatrix(); glRotatef(i*72, 0, 1, 0); glTranslatef(3, 2, 0); glutSolidSphere(4, 10, 10); glPopMatrix(); }
    glTranslatef(0, 5, 0); glutSolidSphere(6, 10, 10); glPopMatrix();glPopMatrix();
}

void drawLake() {
    glPushMatrix();
    glTranslatef(450, 0.1, 80);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.0f, 0.1f, 0.4f, 0.9f); glVertex3f(0, 0.05, 0);
    glColor4f(0.0f, 0.5f, 0.8f, 0.6f);
    for(int i=0; i<=360; i+=10) {
        float rad = i * 3.14159f / 180.0f;
        float wave = sin(globalTime * 3.0f + i) * 5.0f;
        float r = 120.0f + wave;
        glVertex3f(cos(rad)*r*1.6f, 0.05, sin(rad)*r);
    }
    glEnd();
    for (auto& r : brownRocks) {
        glPushMatrix();
        glTranslatef(r.x, r.y, r.z);
        glColor3f(0.4f, 0.25f, 0.15f);
        glScalef(r.sizeX, r.sizeX*0.7f, r.sizeX);
        glutSolidSphere(2, 10, 8);
        glPopMatrix();
    }
    glDisable(GL_BLEND); glEnable(GL_LIGHTING);
    glPopMatrix();
}

void drawBird(Point& b) {
    float x = b.x + sin(globalTime + b.z) * 80;
    float y = b.y + cos(globalTime * 2) * 2;
    glPushMatrix();
    glTranslatef(x, y, b.z);
    glColor3f(0.1, 0.1, 0.1);
    glutSolidSphere(0.8, 8, 8);
    float wingSwing = sin(globalTime * 10) * 25;
    glPushMatrix(); glRotatef(wingSwing, 0, 0, 1); glBegin(GL_TRIANGLES); glVertex3f(0,0,0); glVertex3f(-4, 1.5, 0); glVertex3f(0, 0.4, 0); glEnd(); glPopMatrix();
    glPushMatrix(); glRotatef(-wingSwing, 0, 0, 1); glBegin(GL_TRIANGLES); glVertex3f(0,0,0); glVertex3f(4, 1.5, 0); glVertex3f(0, 0.4, 0); glEnd(); glPopMatrix();
    glPopMatrix();
}

void drawCompositeMountain(float x, float z, int seed) {
    srand(seed);
    int peaks = 3 + rand() % 3;
    for (int i = 0; i < peaks; i++) {
        float offsetX = (rand() % 100) - 50.0f;
        float offsetZ = (rand() % 60) - 30.0f;
        float h = 150.0f + (rand() % 150);
        float w = 100.0f + (rand() % 100);
        glPushMatrix();
        glTranslatef(x + offsetX, 0, z + offsetZ);
        if(isDay) glColor3f(0.4f, 0.37f, 0.36f);
        else glColor3f(0.2f, 0.2f, 0.3f);
        glRotatef(-90, 1, 0, 0);
        glScalef(1.2f, 1.0f, 1.0f);
        glutSolidCone(w,h,16,4);
        glPopMatrix();
    }
}

void display() {
    if (isRaining){
        if (isDay) glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
        else glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    }
    else{
        if (isDay) glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
        else glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(camX, camY, camZ, lookX, lookY, lookZ, 0, 1, 0);
    setupLighting();

    // --- الأجرام السماوية ---
    glDisable(GL_LIGHTING);
    if (isDay) {
            //رسم الشمس
            if(!isRaining){
        glColor3f(1.0f, 0.9f, 0.0f);
        glPushMatrix(); glTranslatef(-200, 380, -600); glutSolidSphere(25, 30, 30); glPopMatrix();
    } }
    else {
        glPushMatrix();
        //القمر
        glTranslatef(-250, 380, -600);
        glColor3f(1.0f, 0.95f, 0.6f); glutSolidSphere(20, 30, 30);
        glColor3f(0.02f, 0.02f, 0.1f); glTranslatef(10, 5, 0); glutSolidSphere(20, 30, 30);//كرتين متداخلتين لتأثير الإضاءة
        glPopMatrix();
    }
    if (isRaining) {
    glDisable(GL_LIGHTING);
    glColor3f(0.8f, 0.8f, 1.0f); // لون قطرات المطر
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    for (int i = 0; i < 4000; i++) {
        glVertex3f(rainX[i], rainY[i], rainZ[i]);
        glVertex3f(rainX[i], rainY[i] - 10.0f, rainZ[i]);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

    // الغيوم
    for (auto& c : cloudPos) {
        glPushMatrix();
        glTranslatef(c.x + sin(globalTime*0.1)*60, c.y, c.z);
        glScalef(2.0f,1.3f,1.2);
        glColor4f(1, 1, 1, 0.85);
        glutSolidSphere(18, 15, 15);
        glPushMatrix(); glTranslatef(15, -2, 2); glutSolidSphere(14, 12, 12); glPopMatrix();
        glPushMatrix(); glTranslatef(-15, -2, -2); glutSolidSphere(14, 12, 12); glPopMatrix();
        glPushMatrix(); glTranslatef(8, 8, -4); glutSolidSphere(12, 12, 12); glPopMatrix();
        glPushMatrix(); glTranslatef(-8, 7, 4); glutSolidSphere(12, 12, 12); glPopMatrix();
        glPopMatrix();
    }
    glEnable(GL_LIGHTING);

    // الأرضية
    glColor3f(0.2, 0.6, 0.2); glBegin(GL_QUADS);
    glVertex3f(-1500, 0, -1500); glVertex3f(1500, 0, -1500); glVertex3f(1500, 0, 1500); glVertex3f(-1500, 0, 1500);
    glEnd();

    // الطريق
    glColor3f(0.6, 0.45, 0.35); glBegin(GL_QUAD_STRIP);
    for (float z = 1000; z > -600; z -= 10) { float x = sin(z * 0.02f) * 30.0f; glVertex3f(x - 12, 0.2, z); glVertex3f(x + 12, 0.2, z); } glEnd();

    // الجبال
    for (int i=0; i<mountains.size(); i++) {
        drawCompositeMountain(mountains[i].x, mountains[i].z, i + 100);
    }


    for (auto& t : trees) drawTree(t);
    drawLake();
    if(!isRaining){
    for (auto& b : birds) drawBird(b); }



    glPushMatrix();
       glDisable(GL_LIGHTING);
         glColor3f(0.5f, 0.35f, 0.05f);
        glTranslatef(40.0f + deerWalk, 5.0f + deerJump, 220.0f); // مكان الغزال
        if(!deerDir)
         glRotatef(180.0f ,0.0f , 1.0f,0.0f);
         glRotatef(-90.0f,1.0f,0.0f,0.0f);
        glScalef(0.4f,0.7f, 0.4f);
         drawModel(deerVertices);
        glEnable(GL_LIGHTING);
    glPopMatrix();

    glPushMatrix();
        glDisable(GL_LIGHTING);
        glColor3f(0.2f, 0.1f, 0.0f);
        glTranslatef(-65.0f, 8.0f, 210.0f); // مكان الحصان
        glRotatef(-horseRot,1.0f,0.0f,0.0f);
        glRotatef(-90.0f,1.0f,0.0f,0.0f);
        glScalef(0.01f, 0.01f, 0.01f);
        drawModel(horseVertices);
        glEnable(GL_LIGHTING);
    glPopMatrix();

    glPushMatrix();
        glDisable(GL_LIGHTING);
        float bobbing = sin(legSwing) * 0.5f;
        glColor3f(0.7f, 0.7f, 0.7f);
        glTranslatef(15.0f + catWalk, 3.0f + bobbing, 260.0f); // مكان القطة
        float tilting = sin(legSwing) * 5.0f;
        glRotatef(tilting , 0.0f, 0.0f , 1.0f);
        if(!catDir)
            glRotatef(180.0f,0.0f,1.0f,0.0f);
        glRotatef(90.0f,0.0f,1.0f,0.0f);
        glRotatef(-90.0f,1.0f,0.0f,0.0f);
        glRotatef(180.0f,0.0f,0.0f,1.0f);

        glScalef(0.2f, 0.2f, 0.2f);
       drawModel(catVertices);
        glEnable(GL_LIGHTING);
    glPopMatrix();

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': camZ -= 10; break;
        case 's': camZ += 10; break;
        case 'a': camX -= 10; break;
        case 'd': camX += 10; break;
        case 'r': camY += 10; break;
        case 'f': camY -= 10; break;
        case 'n': isDay = !isDay; break;
        case 'm': isMoving = true; break;
        case 'p': isMoving = false; break;
        case 'v': isRaining = !isRaining;
                  if(isRaining){
                    PlaySound(TEXT("F:\\Drawing\\FirstProject\\rain.wav"),NULL,SND_FILENAME | SND_ASYNC | SND_LOOP);
                  }
                  else{
                    PlaySound(NULL,0,0);
                  }
                    break;
        case 27: exit(0); break;
    }
    glutPostRedisplay();
}

void timer(int v) {
    globalTime += 0.05f;
    if(isRaining){
        for(int i = 0;i< 4000; i++){
            rainY[i] -=50.0f;
            if (rainY[i] < 0.0f)
                rainY[i] = 500.0f + (rand()% 50);
        }
    }
    if (isMoving) {

        if(deerDir) deerWalk += 0.8f ;
        else deerWalk -= 0.8f;

        if(deerWalk > 100.0f)
            deerDir = false;
        else if(deerWalk < -50.0f)
            deerDir = true;
        deerJump = abs(sin(deerWalk * 0.4f)) * 8.0f;

        // cat moving
        if(catDir) catWalk -= 0.9f ;
        else catWalk +=0.9f;

            if(catWalk < -100.0f)
            catDir= false;
        else if(catWalk > 50.0f)
            catDir = true;
        legSwing += 0.9f;

        //  حركة الحصان (الشبوب ثم الانتظار)
        horseTimer += 0.15f; // عداد زمني

        // إذا كان الوقت في مرحلة الشبوب (مثلاً أول ثانيتين)
        if (sin(horseTimer) > 0) {
            horseRot = sin(horseTimer) * 45.0f; // يشب لزاوية 45
        } else {
            horseRot = 0; // يبقى على الأرض في النصف الثاني من الوقت
        }
    }

    glutPostRedisplay();
    glutTimerFunc(30, timer, 0);
}


int main(int argc, char** argv) {
    glutInit(&argc, argv); glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1200, 800); glutCreateWindow("Final Nature - Original Code with Animals");
    glEnable(GL_DEPTH_TEST);
    initWorld(); // هنا يتم تحميل الـ OBJ
    initRain();
    glutDisplayFunc(display); glutKeyboardFunc(keyboard); glutTimerFunc(0, timer, 0);
    glMatrixMode(GL_PROJECTION); gluPerspective(45.0, 1.5, 1.0, 3000.0); glMatrixMode(GL_MODELVIEW);
    glutMainLoop(); return 0;
}
