//
//  main.cpp
//  arkanoid
//
//  Created by Gangmin Lee on 2022/11/19.
//

//* Simple geometry viewer:  Left mouse: rotate;  Right mouse:   translate;  ESC to quit. */
#define GL_SILENCE_DEPRECATION
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <GLUT/GLUT.h>
#include <string>
using namespace std;

const float planeWidth = 32; // plane 가로
const float planeHeight = 30; // plane 세로
const float planeDepth = 0.2; // plane 두께
const int WALL_ID = 1000;

const int NO_SPHERE = planeWidth*planeHeight; // target_sphere[]의 구의 최대 개수
int cnt_placed_sphere = 0; // 배치에 의해 만들어지는 실제 target_sphere[]의 구의 개수
const float radius_sphere = 0.5;

//const float check_init_x = 0; // 시작 때, 흰 공의 x 좌표
//const float check_init_y = -13.0; // 시작 때, 흰 공의 y좌표

const float check_init_x = 0;
const float check_init_y = -13.0;

const float hit_sphere_init_x = 0; // 시작 때, 빨간 공의 x 좌표
const float hit_sphere_init_y = check_init_y + 2*radius_sphere + 1; // 시작 때 빨간 공의 y 좌표



const string sphere_place[(int)planeHeight] ={ // 가로 길이는 planeWidth, 세로 길이는 planeWidth; '.'이면 빈 배치, '.'이 아니면 공을 배치한다
    "................................",
    "................................",
    "................................",
    "................................",
    "........oo............oo........",
    ".......o..o..........o..o.......",
    "........oo............oo........",
    "................................",
    "...............oo...............",
    "...............oo...............",
    "...............oo...............",
    "....o......................o....",
    "....o......................o....",
    "....o......................o....",
    "....o......................o....",
    "....o......................o....",
    "....o......................o....",
    "....o......................o....",
    "....oooooooooooooooooooooooo....",
    "................................",
    "................................",
    "................................",
    "................................",
    "................................",
    "................................",
    "................................",
    "................................",
    "................................",
    "................................",
    "................................",
};
int choice = 1;

GLfloat BoxVerts[][3] = { // 바닥의 꼭짓점 좌표, 일종의 단위벡터로 바닥크기를 바꾸려면 CWALL의 생성자를 바꿔라
   {-1.0,-1.0,-1.0},
   {-1.0,-1.0,1.0},
   {-1.0,1.0,-1.0},
   {-1.0,1.0,1.0},
   {1.0,-1.0,-1.0},
   {1.0,-1.0,1.0},
   {1.0,1.0,-1.0},
   {1.0,1.0,1.0}
};

GLfloat bNorms[][3] = {
   {-1.0,0.0,0.0},
   {1.0,0.0,0.0},
   {0.0,1.0,0.0},
   {0.0,-1.0,0.0},
   {0.0,0.0,-1.0},
   {0.0,0.0,1.0}
};

int cubeIndices[][4] = {
   {0,1,3,2},
   {4,6,7,5},
   {2,3,7,6},
   {0,4,5,1},
   {0,2,6,4},
   {1,5,7,3}
};

/* Viewer state */
float sdepth = 30; //멀리서보는지 가까이서 보는지
float zNear = 1.0, zFar = 100.0;
float aspect = 5.0 / 4.0;
long xsize, ysize;
int downX, downY;
bool leftButton = false, middleButton = false, rightButton = false;
int i, j;
GLfloat light0Position[] = { 0, 1, 0, 1.0 };
int displayMenu, mainMenu;


string name = "What"; // 이름 변수
int Score = 0;
int Life = 1;
int Player = 1;
GLboolean leftPressed = false;
GLboolean rightPressed = false;

void MyIdleFunc(void) { glutPostRedisplay(); } /* things to do while idle */
void RunIdleFunc(void) { glutIdleFunc(MyIdleFunc); }
void PauseIdleFunc(void) { glutIdleFunc(NULL); }
void renderScene();


enum State {
    GAME_START,
    LIFE_DECREASE,
    GAME_PLAYING,
    GAME_OVER,
    GAME_CLEAR
};

State statecode = GAME_START;

class CSphere
{
public:
   float center_x, center_y, center_z;
   float color_r, color_g, color_b;
   float dir_x, dir_y, dir_z; // 공의 x, y, z축 성분 속도
   float speed;


public:
   GLdouble m_mRotate[16];
   CSphere()
   {
      center_x = center_y = center_z = 0.0;
   }

   void init()
   {
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix(); // 현재 모델뷰 행렬을 저장, 그리고 그 밑으로 그려지는 것들에 대해 현재 모델뷰 행렬을 가지고 다시 누적시켜가면서 표현
      glLoadIdentity(); // 행렬을 단위행렬로 초기화한다.
      glGetDoublev(GL_MODELVIEW_MATRIX, m_mRotate); // 매트릭스 종류, 값을 받을 매트릭스
      glPopMatrix(); // 그동안 누적했던 행렬을 없애고 push한 행렬을 다시 불러온다.
   }

   void setCenter(float x, float y, float z)
   {
      center_x = x;    center_y = y;    center_z = z;
   }

   void setColor(float r, float g, float b)
   {
      color_r = r; color_g = g; color_b = b;
   }

   // 수정되지 않은 다른 사람의 hasIntersected, 공이 끼지 않게 하려면 개선해야함
    bool hasIntersected(const CSphere &ball)
    {
        float deltaX = center_x - ball.center_x;
        float deltaY = center_y - ball.center_y;
        if (sqrt(deltaX * deltaX + deltaY * deltaY) <= 2*radius_sphere)
            return true;
        else
            return false;
    }


   void hitBy(const CSphere hitSphere)
   {
       // 구와 구끼리 충돌시 끼임 문제 해결 부분
       // 구끼리 부딪혀서 방향이 바뀌었는데 끼어있으면, 현재 속도 반대 방향으로 x 성분과 y 성분의 위치를 끼임이 해결될 때까지 0.01씩 바꾼다.
        while (hasIntersected(hitSphere)) {
            center_x -= dir_x / sqrt(dir_x*dir_x + dir_y*dir_y) * 0.01;
            center_y -= dir_y / sqrt(dir_x*dir_x + dir_y*dir_y) * 0.01;
        }
        float deltaX = hitSphere.center_x - center_x;
        float deltaY = hitSphere.center_y - center_y;
        float distance = sqrt(deltaX * deltaX + deltaY * deltaY);
       
        // Rodrigues' rotation formula 사용
        // V(rot) = -v + 2(k inner_product v)k
        float k_x = deltaX / distance;
        float k_y = deltaY / distance;
        float v_x = -dir_x;
        float v_y = -dir_y;
        float inner_product = k_x * v_x + k_y * v_y;
       
        dir_x = -v_x + 2 * inner_product * k_x;
        dir_y = -v_y + 2 * inner_product * k_y;


   }
   void draw()
   {
      glLoadIdentity(); //단위행렬로 초기화
      glTranslatef(0.0, 0.0, -sdepth); //+Z방향이 화면에서 우리가 보는방향이므로 -를하면 축소되는 효과, wall과 위상을 맞추기위해 사용한듯
      glMultMatrixd(m_mRotate); // 마우스 이동에따라 motion콜백함수에서 m_mrotate행렬이 변형되는데 이 mult함수로 단위행렬에 곱하여 m_mrotate대로 회전한다.
      glTranslated(center_x, center_y, center_z); // 중앙으로 이동한다.
      glColor3f(color_r, color_g, color_b); //색 조정
      glutSolidSphere(radius_sphere, 20, 16); //radius_sphere의 반지름 나머지 두 인자는 구를 나타내는 경선과 위선
   }
};

class CWall // 추가해야함 CSphere에서 상속
{
public:
   float width, height, depth;
   float center_x, center_y, center_z;
   float color_r, color_g, color_b;

   GLfloat Verts[8][3];

public:
   GLdouble m_mRotate[16];
   CWall(float w = 0.0, float h = 0.0, float d = 0.0)
   {
      width = w; height = h; depth = d;
      color_r = 0.0; color_g = 1.0; color_b = 0.0;


    float coef{};
      for (int i = 0; i < 8; i++) {
         for (int j = 0; j < 3; j++) {
            if (j == 0) coef = w / 2.0;
            if (j == 1) coef = h / 2.0;
            if (j == 2) coef = d / 2.0;
            Verts[i][j] = coef * BoxVerts[i][j]; // BoxVerts는 일종의 단위벡터, coef는 일종의 단위벡터에 곱해지는 크기
         }
      }
   }

   void init() //initGL에서 initrotate를 호출하고 거기서 호출됨
   { // openGL에서 행렬은 mode를 지정한 후, transelate, rotate등의 변환을 거치는데, 일반적으로 변환을 한 후 LoadIdentity로 초기화를 해줘야 한다. 안그러면 누적되니까.. 근데 이 방법 말고도
      //스택을 이용해서 현재 위치를 push로 저장한 후 변형을 한 후에 그냥 pop으로 받을수도 있다.
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
      glGetDoublev(GL_MODELVIEW_MATRIX, m_mRotate); //매트릭스 종류, 값을 받을 매트릭스, m_mRotate엔 단위행렬이 들어감
      glPopMatrix();
   }

   void setSize(float w, float h, float d)
   {
      width = w;
      height = h;
      depth = d;


       float coef{};
      for (int i = 0; i < 8; i++) {
         for (int j = 0; j < 3; j++) {
            if (j == 0) coef = w / 2.0;
            if (j == 1) coef = h / 2.0;
            if (j == 2) coef = d / 2.0;
            Verts[i][j] = coef * BoxVerts[i][j]; // BoxVerts는 일종의 단위벡터, coef는 일종의 단위벡터에 곱해지는 크기
         }
      }
   }

   void setCenter(float x, float y, float z)
   {
      center_x = x;    center_y = y;    center_z = z;
   }

   void setColor(float r, float g, float b)
   {
      color_r = r; color_g = g; color_b = b;
   }

   void draw() //display 콜백함수에서 호출됨
   {
      glLoadIdentity();
      glTranslatef(0.0, 0.0, -sdepth); //  이 행렬이 현재의 모델뷰행렬에 곱해져서 평행이동을 한다.
      //모델뷰행렬에 1 0 0 0.0
      //            0 1 0 0.0
      //            0 0 1 -sdepth
//                              0 0 0   1  이 곱해지게 된다.
//화면에서 우리쪽이 z벡터가 +가 되는쪽이므로 sdepth가 커질수록 wall이 작아지는 효과를 얻을 수 있다.
      glMultMatrixd(m_mRotate); //현재 행렬에 rotate행렬을곱한다. m_mRotate는 motion콜백함수에서 마우스 움직임에따라 설정된다.
      glTranslatef(center_x, center_y, center_z); //wall을 center_x,y,z만큼의 위치로 평행이동

      glColor3f(color_r, color_g, color_b); //색 설정

      int i;
      int v1, v2, v3, v4;

      for (i = 0; i < 6; i++) {
         v1 = cubeIndices[i][0];
         v2 = cubeIndices[i][1];
         v3 = cubeIndices[i][2];
         v4 = cubeIndices[i][3];

         glBegin(GL_QUADS); //glbegin은 도형을 그리기 시작했다는 뜻
         glNormal3f(bNorms[i][0], bNorms[i][1], bNorms[i][2]);
         glVertex3f(Verts[v1][0], Verts[v1][1], Verts[v1][2]);
         glNormal3f(bNorms[i][0], bNorms[i][1], bNorms[i][2]);
         glVertex3f(Verts[v2][0], Verts[v2][1], Verts[v2][2]);
         glNormal3f(bNorms[i][0], bNorms[i][1], bNorms[i][2]);
         glVertex3f(Verts[v3][0], Verts[v3][1], Verts[v3][2]);
         glNormal3f(bNorms[i][0], bNorms[i][1], bNorms[i][2]);
         glVertex3f(Verts[v4][0], Verts[v4][1], Verts[v4][2]);
         glEnd(); //glbegin과 마찬가지로 도형그리기 끝
      }
   }

   // 윗 방향 벽과 충돌 감지
   bool hasUpIntersected(CSphere* sphere)
   {
      if (sphere->center_y + radius_sphere >= planeHeight / 2.1)
         return (true);
      return (false);
   }

   // 아래 방향 벽과 충돌 감지
   bool hasDownIntersected(CSphere* sphere, CSphere* check, CSphere* check1, CSphere*check2,
    CSphere* check3, CSphere* check4, CSphere* check5) {
      if (sphere->center_y - radius_sphere <= -1 * planeHeight / 2)
      {
          Life -= 1; //아래 벽과 닿으면 Life가 깎임
          statecode = LIFE_DECREASE;
          sphere->center_x = hit_sphere_init_x;
          sphere->center_y = hit_sphere_init_y;
          sphere->center_z = 0.0;

          sphere->dir_x = 0.0;
          sphere->dir_y = 0.0;
          sphere->dir_z = 0.0;

          check->setCenter(check_init_x , check_init_y, 0.0); //하얀공
          check1->setCenter(check_init_x+0.5 , check_init_y, 0.0); //하얀공
          check2->setCenter(check_init_x+1.0, check_init_y, 0.0); //하얀공
          check3->setCenter(check_init_x-0.5, check_init_y, 0.0); //하얀공
          check4->setCenter(check_init_x-1, check_init_y, 0.0); //하얀공
          check5->setCenter(check_init_x+1.5, check_init_y, 0.0); //하얀공

          return (true);
      }
      return (false);
   }

   // 왼쪽 방향 벽과 충돌 감지
   bool hasLeftIntersected(CSphere* sphere) {
      if (sphere->center_x - radius_sphere <= -1 * planeWidth / 2.1)
         return (true);
      return (false);
   }

   // 오른쪽 방향 벽과 충돌 감지
   bool hasRightIntersected(CSphere* sphere)
   {
      if (sphere->center_x + radius_sphere >= planeWidth / 2.1)
         return (true);
      return (false);
   }


   void hitBy(CSphere* sphere, CSphere* check, CSphere* check1, CSphere* check2, CSphere* check3,
   CSphere* check4, CSphere* check5){
      if (hasUpIntersected(sphere))
      {
         sphere->dir_y = -(sphere->dir_y);

         // 구와 벽끼리 충돌시 끼임 문제 해결 부분
         // 구와 벽이 부딪혀서 구의 방향이 바뀌었는데 끼어있으면, 반사 방향으로 x 성분과 y 성분의 위치를 끼임이 해결될 때까지 0.1씩 바꾼다.
         while (hasUpIntersected(sphere)) {
            sphere->center_y -= 0.1;
         }
      }
      else if (hasDownIntersected(sphere, check,check1,check2,check3,check4,check5)) {
         sphere->dir_y = -(sphere->dir_y);

         // 구와 벽끼리 충돌시 끼임 문제 해결 부분
         // 구와 벽이 부딪혀서 구의 방향이 바뀌었는데 끼어있으면, 반사 방향으로 x 성분과 y 성분의 위치를 끼임이 해결될 때까지 0.1씩 바꾼다.
         while (hasDownIntersected(sphere, check,check1,check2,check3,check4,check5)) {
            sphere->center_y += 0.1;

         }


      }
      else if (hasLeftIntersected(sphere))
      {
         sphere->dir_x = -(sphere->dir_x);

         // 구와 벽끼리 충돌시 끼임 문제 해결 부분
         // 구와 벽이 부딪혀서 구의 방향이 바뀌었는데 끼어있으면, 반사 방향으로 x 성분과 y 성분의 위치를 끼임이 해결될 때까지 0.1씩 바꾼다.
         while (hasLeftIntersected(sphere)) {
            sphere->center_x += 0.1;
         }
      }
      else if (hasRightIntersected(sphere))
      {
         sphere->dir_x = -(sphere->dir_x);

         // 구와 벽끼리 충돌시 끼임 문제 해결 부분
         // 구와 벽이 부딪혀서 구의 방향이 바뀌었는데 끼어있으면, 반사 방향으로 x 성분과 y 성분의 위치를 끼임이 해결될 때까지 0.1씩 바꾼다.
         while (hasRightIntersected(sphere)) {
            sphere->center_x -= 0.1;
         }
      }
   }
};
class bar:public CSphere{
   public:
      void draw(){
      glLoadIdentity(); //단위행렬로 초기화
      glTranslatef(0.0, 0.0, -sdepth); //+Z방향이 화면에서 우리가 보는방향이므로 -를하면 축소되는 효과, wall과 위상을 맞추기위해 사용한듯
      glMultMatrixd(m_mRotate); // 마우스 이동에따라 motion콜백함수에서 m_mrotate행렬이 변형되는데 이 mult함수로 단위행렬에 곱하여 m_mrotate대로 회전한다.
      glTranslated(center_x, center_y, center_z); // 중앙으로 이동한다.
      glColor3f(color_r, color_g, color_b); //색 조정
      glutSolidCube(1); //radius_sphere의 반지름 나머지 두 인자는 구를 나타내는 경선과 위선
      }
};
bar check, check1, check2,check3,check4,check5;
CSphere hit_sphere; // 조작하는 흰 구, 반사되는 빨간구
CSphere target_sphere[NO_SPHERE]; // 맞춰야 하는 타겟 파란 공의 개수 최대 NO_SPHERE 개수만큼 존재
CWall g_wall(planeWidth, planeHeight, planeDepth); // 바닥 평면
CWall boundary_wall[4]; // 가장자리 벽



void InitObjects()
{
    // specify initial colors and center positions of each spheres
    hit_sphere.setColor(0.8, 0.2, 0.2); hit_sphere.setCenter(hit_sphere_init_x, hit_sphere_init_y, 0.0); //빨간공
    check.setColor(0.8, 0.8, 0.8); check.setCenter(check_init_x, check_init_y, 0.0); //하얀공
    check1.setColor(0.8, 0.8, 0.8); check1.setCenter(check_init_x+0.5, check_init_y, 0.0); //하얀공
    check2.setColor(0.8, 0.8, 0.8); check2.setCenter(check_init_x+1, check_init_y, 0.0); //하얀공
    check3.setColor(0.8, 0.8, 0.8); check3.setCenter(check_init_x-0.5, check_init_y, 0.0); //하얀공
    check4.setColor(0.8, 0.8, 0.8); check4.setCenter(check_init_x+1.5, check_init_y, 0.0); //하얀공
    check5.setColor(0.8, 0.8, 0.8); check5.setCenter(check_init_x-1, check_init_y, 0.0); //하얀공



    // 파란색 target_sphere 구 배치
    cnt_placed_sphere = 0;
    for (int i = 0; i < planeHeight; i++) {
        for (int j = 0; j < planeWidth; j++) {
            if (sphere_place[i][j] != '.') {
                target_sphere[cnt_placed_sphere].dir_x = 1;
                target_sphere[cnt_placed_sphere].setColor(0, 0, 1);
                target_sphere[cnt_placed_sphere++].setCenter(-planeWidth / 2 + j + radius_sphere, planeHeight / 2 - i - radius_sphere, 0);
            }
        }
    }

    // specify initial colors and center positions of a wall
    g_wall.setColor(2, 2, 2); g_wall.setCenter(0.0, 0.0, -0.6);
    boundary_wall[0].setCenter(0.0, planeHeight / 2, 0.0); // 위쪽 가장자리 벽
    boundary_wall[1].setCenter(0.0, -(planeHeight / 2), 0.0); // 아래쪽 가장자리 벽
    boundary_wall[2].setCenter(planeWidth / 2, 0.0, 0.0); // 오른쪽 가장자리 벽
    boundary_wall[3].setCenter(-(planeWidth / 2), 0.0, 0.0); // 왼쪽 가장자리 벽

}



void ReshapeCallback(int width, int height)
{

   xsize = width;
   ysize = height;
   aspect = (float)xsize / (float)ysize;

   glViewport(0, 0, xsize, ysize); //처음 생성되는 윈도우는 InitGL의 glutInitWindowsize에서 결정되지만 실행되는 프로그램 자체는 여기 이 함수의 해상도에 따라 조절된다. 뭔소린가 싶으면
   // 각각 1920,1080으로 실행해 보면 알기쉬울듯
   glMatrixMode(GL_PROJECTION); //openGL은 모델링을 GL_MODELVIEW, GL_PROJECTION, GL_TEXTURE 3개의 행렬로 화면에표시한다. 만약 (1,1,1)에 뭘 그린다면 해당 좌표는 GL_MODELVIEW 매트릭스를
   // 곱해서 실제적인 위치를 지정한다. 따라서 내부적인 위치정보는 하나더라도 GL_MODELVIEW 매트릭스가 변경된다면 전혀 다른 위치에 그려진다. 이렇게 그려진 모델은
      //GL_PROJECTION 행렬과 곱해져 화면상에 투영된다. 즉 GL_MODELVIEW가 모델링에 대한 실존 위치라면 GL_PROJECTION은 해당 모델링을 최종적으로 어떻게 표시할지 결정한다.

/*
GL_MODELVIEW에 관해: 특정좌표에 도형을 그린다면 GL_MODELVIEW를 곱해 실제적인 위치를 지정, 만약 이게 변경된다면 전혀다른 좌표에 그려짐
glTranslatef, glScalef, glRotatef등을 통해 물체의 위치를 변환 또는 제어함 이를 모델링 변환 이라고 함,모델뷰행렬이란 좌표계 변환을 통해 물체의 위치와 방향을 결정하기 위한
4x4행렬각 버텍스들은 하나의 열로 구성된 행렬로 만들어져서 모델뷰행렬과 곱해짐

GL_PROJECTION에 관해: GL_MODELVIEW가 그려진 도형의 위치라면 도형은 GL_PROJECTION의 행렬과 곱해져 최종적으로 어떻게 화면에 뿌릴것인가를 계산한다.
직교투영 glOrtho 원근투영 gluPerspectivef


*/


   glLoadIdentity(); // 단위행렬로 초기화한다.
   gluPerspective(64.0, aspect, zNear, zFar); //원근투영

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity(); // 단위행렬로 초기화한다.   이전에 적용했던 회전값을 무시하고 다시 설정하기위해 리셋을 해야한다. 이 코드가 없으면 회전이 계속 누적적용된다.
   //이와같이 glMatrixMode(GL_MODELVIEW)이후에 glLoadIdentity()가 나오는게 이러한 이유이다.

   glutPostRedisplay();
}

void renderBitmapCharacter(float x, float y, float z, void* font, char* string)
{
    char* c;
    glRasterPos3f(x, y, z);
    for (c = string; *c != '\0'; c++)
    {
        glutBitmapCharacter(font, *c);
    }
}

void DisplayCallback(void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //glclear는 버퍼들을 미리설정된 값으로 바꾼다.
   //GL_COLOR_BUFFER_BIT는 initgl의 glclearcolor에서 설정된 값이다.
   //한편 3차원을 2차원에 그리기위해 depth라는 개념이 있다. 같은 x,y라도 depth차이에 따라 그려지고 말고가 정해진다.
   //이 줄 자체가 하나의 관용구 비슷하게 쓰이는데, 3차원그림 전체를 지우는 역할을 한다.
   glMatrixMode(GL_MODELVIEW);



   for (i = 0; i < cnt_placed_sphere; i++) {

           target_sphere[i].draw(); //공 그리기

   }
   check.draw(); // 하얀 구 묶음 그리기
   check1.draw();
   check2.draw();
   check3.draw();
   check4.draw();
   check5.draw();

   hit_sphere.draw(); // 빨간 구 그리기
   g_wall.draw(); // 벽 그리기
   for (int i = 0; i < 4; i++) if(i != 1) boundary_wall[i].draw(); // 아래 벽 제외하고 boundary_wall 그리기
   renderBitmapCharacter(35, 15.0, -2, GLUT_BITMAP_HELVETICA_18, (char*)((("SCORE : ")+to_string(Score)).c_str()));
   renderBitmapCharacter(35, 13.0, -2, GLUT_BITMAP_HELVETICA_18, (char*)((("LIFE : ") + to_string(Life)).c_str()));
    renderBitmapCharacter(35, 11.0, -2, GLUT_BITMAP_HELVETICA_18, (char*)((("PLAYER ") + to_string(Player)).c_str()));
    if (statecode==GAME_START||statecode==LIFE_DECREASE) {
        renderBitmapCharacter(11, -6.5, 10, GLUT_BITMAP_HELVETICA_18, (char*)"PRESS ENTER TO PLAY ARKANOID");
    }

   if (Life == 0) { //Life가 0이되면 gameover
       renderBitmapCharacter(12.8, -6.5, 5, GLUT_BITMAP_HELVETICA_18, (char*)"!!!!GAME OVER!!!!");
       renderBitmapCharacter(11.7, -7.5, 5, GLUT_BITMAP_HELVETICA_18, (char*)"PRESS R TO PLAY AGAIN");
       statecode = GAME_OVER;

   }

   if (Score >= cnt_placed_sphere) { //Score가 cnt_placed_sphere==127 여야 모든 공을맞춘 것, 테스트할때는 20을 빼는등 큰 수를 빼야할듯
       renderBitmapCharacter(12.6, -6.5, 5, GLUT_BITMAP_HELVETICA_18, (char*)
                             "YOU WON!!! PRESS R TO PLAY AGAIN");
      
       statecode = GAME_CLEAR;
   }

   glutSwapBuffers(); // front버퍼와 back버퍼를 swapping 하기 위한것, 프론트버퍼내용이 화면에 뿌려지는 동안 새로운 내용이 백버퍼에 쓰이고 백버퍼에 기록이 다 되면 프론트와 백이 바뀐다.
   //백버퍼에 그림을 다 그렸으면 전면버퍼와 통째로 교체한다. 전면과 후면이 일시에 교체되므로 백버퍼에 미리 준비해둔 그림이 나타난다.
//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 빼도 상관없는듯?
}

void KeyboardCallback(unsigned char ch, int x, int y)
{
    switch (ch)
    {

    case 'r': {
        if (statecode == GAME_CLEAR) {
            statecode = GAME_START;
            Life = 1;
            Score = 0;
            Player = 1;

            InitObjects();
            break;
        }
        if(statecode == GAME_OVER){
            statecode = GAME_START;
            Life = 1;
            Score = 0;
            Player = 1;

            InitObjects();
            break;
        }
    }


    case 13 : {//엔터로 바꾸어야 함.

        switch (statecode) {
        case GAME_START: {
           
            hit_sphere.dir_x = 0.0; //sphere[0]은 스페이스를 누르면 움직이는 빨간 공
            hit_sphere.dir_y = 3.0;
            hit_sphere.dir_z = 0.0;
            statecode = GAME_PLAYING;
        }
        case LIFE_DECREASE: {
            hit_sphere.dir_x = 0.0; //sphere[0]은 스페이스를 누르면 움직이는 빨간 공
            hit_sphere.dir_y = 3.0;
            hit_sphere.dir_z = 0.0;
            statecode = GAME_PLAYING;
        }
        default: {
            //doing nothing
        }


        }
        break;
    }
    case 27: {//ESC키
        exit(0);
        break;
    }


   }
   
   glutPostRedisplay();
}



void SpecialCallback(int key, int x, int y) {
    
    if (GLUT_KEY_LEFT == key) {
        leftPressed = true;
    }
    if (GLUT_KEY_RIGHT == key) {
        rightPressed = true;
    }


}

void SpecialUpCallback(int key, int x, int y) {
    if (GLUT_KEY_LEFT == key) {
        leftPressed = false;
    }
    if (GLUT_KEY_RIGHT == key) {
        rightPressed = false;
    }
}

void MouseCallback(int button, int state, int x, int y)
{ // GLUT_DOWN=마우스를 누르고 있는 상태, LEFT,RIGHT,MIDDLE 각각 왼쪽 오른쪽 휠 이런거
   downX = x; downY = y;
   leftButton = ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN));
   middleButton = ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_DOWN));
   rightButton = ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN));
   glutPostRedisplay();
}


void initRotate() { // 구현이 살짝 다름 initGL에서 호출

   for (int i = 0; i < NO_SPHERE; i++)
   {
      target_sphere[i].init();
   }
   hit_sphere.init();
   check.init();
   check1.init();
   check2.init();
   check3.init();
   check4.init();
   check5.init();

   g_wall.init();
   for (int i = 0; i < 4; i++) boundary_wall[i].init();

}

void InitGL() {
   //opengl의 기본 설정값
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // 깊이버퍼가 있고 rgb색상버퍼가 있는 이중버퍼창의 모드를 설정
   glutInitWindowSize(1200, 800);
   glutCreateWindow("ARKANOID"); // Displaymode에서 설정한 버퍼대로 창을 띄워라
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);
   glClearColor(0, 0, 255, 50);
   glPolygonOffset(1.0, 1.0);
   glDisable(GL_CULL_FACE);
   glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
   glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
   glEnable(GL_COLOR_MATERIAL);
   glColorMaterial(GL_FRONT, GL_DIFFUSE);
   glLightfv(GL_LIGHT0, GL_POSITION, light0Position);
   glEnable(GL_LIGHT0);
   initRotate(); //구와 wall의 init호출

   glShadeModel(GL_SMOOTH);
   glEnable(GL_LIGHTING);



   //이하 opengl의 이벤트 처리부분
   //콜백함수란: 이벤트 처리기라고도 한다.w를 누르면 앞으로 가고, esc를 누르면 꺼지는거와 같이 어떤 반응에대한 결과를 나타내는 함수인듯, 디스플레이콜백함수의 경우 프레임마다 호출될것이다.

   glutIdleFunc(renderScene); // 어플리케이션의 휴면(idle)시간에 호출되는 함수(renderscene)를 호출하도록한다. 즉 애니메이션이 된다.
   glutReshapeFunc(ReshapeCallback); // GLUT는 처음 윈도우를 열때, 윈도우 위치를 옮길때, 윈도우 크기를 조절할때 reshapeevent가 발생한걸로 본다. 매개변수속의 함수로 원하는 함수효과를 할수있다.
   // reshapeevent 가 발생하면 괄호안의 파라미터를통해 변경된 윈도우의 폭과 높이를 콜백함수로 전달한다. 그래서 width와 height를 그냥 받아서 쓰는듯.
   glutDisplayFunc(DisplayCallback); // 이 함수는 DisplayCallback이라는 함수를 디스플레이이벤트에 대한 콜백함수로 사용하는 함수, 매개변수로 전달한 함수는 디스플레이이벤트마다 호출된다.
   glutKeyboardFunc(KeyboardCallback); // 키보드가 눌렸을경우 작동하는 콜백함수
  
   glutSpecialFunc(SpecialCallback); //방향키, F1~F12와 같은 특별한 키가 눌린 경우 작동하는 콜백함수
   glutSpecialUpFunc(SpecialUpCallback); //방향키를 뗐을때 작동하는 콜백함수, 부드러운 움직임을 위해 추가
   glutMouseFunc(MouseCallback); // 마우스가 눌렸을경우 작동하는 콜백함수
}



int currentTime, previousTime = -1; // 거기엔 없음
void renderScene() // 구현 다름, 어플리케이션의 휴면시간에 호출되는 함수, glutidleFunc()에서 호출된다.
{

   int timeDelta;
   currentTime = glutGet(GLUT_ELAPSED_TIME);
   if (previousTime == -1) timeDelta = 0;
   else timeDelta = (currentTime - previousTime) / 2;

   float x = hit_sphere.center_x;
   float y = hit_sphere.center_y;
   float z = hit_sphere.center_z;



   if (leftPressed) {
       
       if (statecode == GAME_START || statecode == LIFE_DECREASE) //게임시작단계와 라이프가 깎인 단계에서는 하얀공과 빨간공이 같이 움직임
       {
           hit_sphere.setCenter(hit_sphere.center_x -timeDelta*0.05, hit_sphere.center_y, 0);
           check.setCenter(check.center_x -timeDelta*0.05, check.center_y, 0);
           check1.setCenter(check1.center_x -timeDelta*0.05, check1.center_y, 0);
           check2.setCenter(check2.center_x -timeDelta*0.05, check2.center_y, 0);
           check3.setCenter(check3.center_x -timeDelta*0.05, check3.center_y, 0);
           check4.setCenter(check4.center_x -timeDelta*0.05, check4.center_y, 0);
           check5.setCenter(check5.center_x -timeDelta*0.05, check5.center_y, 0);


       }
       else //다른 단계에서는 하얀공만 움직임
       {
           check.setCenter(check.center_x - timeDelta*0.05, check.center_y, 0);
           check1.setCenter(check1.center_x - timeDelta*0.05, check1.center_y, 0);
           check2.setCenter(check2.center_x - timeDelta*0.05, check2.center_y, 0);
           check3.setCenter(check3.center_x - timeDelta*0.05, check3.center_y, 0);
           check4.setCenter(check4.center_x - timeDelta*0.05, check4.center_y, 0);
           check5.setCenter(check5.center_x - timeDelta*0.05, check5.center_y, 0);

       }
       
   }

   if (rightPressed) {
       
       if (statecode == GAME_START || statecode == LIFE_DECREASE) //게임시작단계와 라이프가 깎인 단계에서는 하얀공과 빨간공이 같이 움직임
       {
           hit_sphere.setCenter(hit_sphere.center_x + timeDelta*0.05, hit_sphere.center_y, 0);
           check.setCenter(check.center_x + timeDelta*0.05, check.center_y, 0);
           check1.setCenter(check1.center_x + timeDelta*0.05, check1.center_y, 0);
           check2.setCenter(check2.center_x + timeDelta*0.05, check2.center_y, 0);
           check3.setCenter(check3.center_x + timeDelta*0.05, check3.center_y, 0);
           check4.setCenter(check4.center_x + timeDelta*0.05, check4.center_y, 0);
           check5.setCenter(check5.center_x + timeDelta*0.05, check5.center_y, 0);

       }
       else //다른 단계에서는 하얀공만 움직임
       {
           check.setCenter(check.center_x + timeDelta*0.05, check.center_y, 0);
           check1.setCenter(check1.center_x + timeDelta*0.05, check1.center_y, 0);
           check2.setCenter(check2.center_x + timeDelta*0.05, check2.center_y, 0);
           check3.setCenter(check3.center_x + timeDelta*0.05, check3.center_y, 0);
           check4.setCenter(check4.center_x + timeDelta*0.05, check4.center_y, 0);
           check5.setCenter(check5.center_x + timeDelta*0.05, check5.center_y, 0);


       }
       
   }
    

     if (statecode == GAME_PLAYING) {
         x = hit_sphere.center_x;
         y = hit_sphere.center_y;
         z = hit_sphere.center_z;

         hit_sphere.setCenter(
             x + timeDelta * 0.010 * hit_sphere.dir_x, // 속도의 성분이 1일때, 구는 timeDelta 당 0.002만큼 움직인다.
             y + timeDelta * 0.010 * hit_sphere.dir_y,
             z + timeDelta * 0.010 * hit_sphere.dir_z);
     }
   glutPostRedisplay(); // 윈도우를 다시그리도록 요청, 바로 디스플레이콜백함수(renderscene)가 호출되진 않고 메인루프(아마 glutMainloop?)에서 호출시점을 결정한다. 이게 없으면 연결이 부자연스러움

   // renderScene에서 공 사이의 충돌을 처리하는 부분
   // 공이 닿는 지점을 검사하고, 닿았을 경우 반사를 실행
    if (hit_sphere.hasIntersected(check) == true) // hit_sphere인 빨간공과 check인 흰공과의 충돌 감지
    {
        hit_sphere.hitBy(check);
    }
   if (hit_sphere.hasIntersected(check1) == true) // hit_sphere인 빨간공과 check인 흰공과의 충돌 감지
    {
        hit_sphere.hitBy(check1);
    }
   if (hit_sphere.hasIntersected(check2) == true) // hit_sphere인 빨간공과 check인 흰공과의 충돌 감지
    {
        hit_sphere.hitBy(check2);
    }
   if (hit_sphere.hasIntersected(check3) == true) // hit_sphere인 빨간공과 check인 흰공과의 충돌 감지
    {
        hit_sphere.hitBy(check3);
    }
   if (hit_sphere.hasIntersected(check4) == true) // hit_sphere인 빨간공과 check인 흰공과의 충돌 감지
    {
        hit_sphere.hitBy(check4);
    }
   if (hit_sphere.hasIntersected(check4) == true) // hit_sphere인 빨간공과 check인 흰공과의 충돌 감지
    {
        hit_sphere.hitBy(check5);
    }

    
    for(int i = 0; i < cnt_placed_sphere; i++){
        if (hit_sphere.hasIntersected(target_sphere[i]) == true) // hit_sphere인 빨간공과 target_sphere[]인 파란공들과의 충돌 감지
        {
            hit_sphere.hitBy(target_sphere[i]); // 우선은 빨간공 흰공 충돌만 처리 나중에 다른 공들도 넣기
            target_sphere[i].setCenter(0, 500, 500); //닿은 공은 멀리 유배보냄
            target_sphere[i].dir_x = 0;
            Score += 1;
            break;
        }
    }
    

   // 벽에 대한 반사 실행, 다른 사람 코드 그대로 가져옴
    g_wall.hitBy(&hit_sphere, &check,&check1,&check2,&check3,&check4,&check5);


    
    for(int i = 0; i < cnt_placed_sphere; i++){
        if(g_wall.hasLeftIntersected(&target_sphere[i]) || g_wall.hasRightIntersected(&target_sphere[i])){
            
            while (g_wall.hasLeftIntersected(&target_sphere[i])) {
                for(int j = 0; j < cnt_placed_sphere; j++)
                    target_sphere[j].center_x += 0.1;
            }
            while (g_wall.hasRightIntersected(&target_sphere[i])) {
                for(int j = 0; j < cnt_placed_sphere; j++)
                    target_sphere[j].center_x -= 0.1;
            }
            for(int i = 0; i < cnt_placed_sphere; i++){
                target_sphere[i].dir_x = -target_sphere[i].dir_x;
            }
            break;
        }
    }

   previousTime = currentTime;
}

// 추가해야함 using namespace std;
int main(int argc, char** argv)
{
   glutInit(&argc, argv); // glut 시작 초기화, 보통 그래픽못쓰는데서 한다거나 하면 오류 감지용
   InitObjects(); // 초기화면, 각 공의 위치를 세팅
   InitGL(); // opengl 관련 초기화
   glutMainLoop(); // 이벤트 루프를 돌리는것, 이벤트별로 콜백함수 등록을 마쳤으니 이벤트 루프로 진입하라.
   return 0;
}


