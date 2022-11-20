//
//  arkanoid.h
//  arkanoid
//
//  Created by Gangmin Lee on 2022/11/19.
//

#define GL_SILENCE_DEPRECATION
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <GLUT/GLUT.h>
#include <string>
using namespace std;


const float planeWidth = 32; 
const float planeHeight = 30; 
const float planeDepth = 0.2; 
const int WALL_ID = 1000;

const int NO_SPHERE = planeWidth*planeHeight; 
int cnt_placed_sphere = 0; 
const float radius_sphere = 0.5;




const float Control_bar_init_x = 0;
const float Control_bar_init_y = -13.0;

const float hit_sphere_init_x = 0; 
const float hit_sphere_init_y = Control_bar_init_y + 2*radius_sphere + 1; 



const string sphere_place[(int)planeHeight] ={ 
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


float sdepth = 30; 
float zNear = 1.0, zFar = 100.0;
float aspect = 5.0 / 4.0;
long xsize, ysize;
int downX, downY;
bool leftButton = false, middleButton = false, rightButton = false;
int i, j;
int displayMenu, mainMenu;
string name = "20195517 이강민"; 
int Score = 0;
int Life = 1;
int Player = 1;



void MyIdleFunc(void) { glutPostRedisplay(); }
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




GLfloat BoxVerts[][3] = { 
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

GLfloat light0Position[] = { 0, 1, 0, 1.0 };
GLboolean leftPressed = false;
GLboolean rightPressed = false;



State statecode = GAME_START;

class CSphere
{
public:
   float center_x, center_y, center_z;
   float color_r, color_g, color_b;
   float dir_x, dir_y, dir_z; 
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
      glPushMatrix(); 
      glLoadIdentity(); 
      glGetDoublev(GL_MODELVIEW_MATRIX, m_mRotate); 
      glPopMatrix(); 
   }

   void setCenter(float x, float y, float z)
   {
      center_x = x;    center_y = y;    center_z = z;
   }

   void setColor(float r, float g, float b)
   {
      color_r = r; color_g = g; color_b = b;
   }

   
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
       
       
        while (hasIntersected(hitSphere)) {
            center_x -= dir_x / sqrt(dir_x*dir_x + dir_y*dir_y) * 0.01;
            center_y -= dir_y / sqrt(dir_x*dir_x + dir_y*dir_y) * 0.01;
        }
        float deltaX = hitSphere.center_x - center_x;
        float deltaY = hitSphere.center_y - center_y;
        float distance = sqrt(deltaX * deltaX + deltaY * deltaY);
       
        
        
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
      glLoadIdentity(); 
      glTranslatef(0.0, 0.0, -sdepth); 
      glMultMatrixd(m_mRotate); 
      glTranslated(center_x, center_y, center_z); 
      glColor3f(color_r, color_g, color_b); 
      glutSolidSphere(radius_sphere, 20, 16); 
   }
};

class CWall 
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
            Verts[i][j] = coef * BoxVerts[i][j]; 
         }
      }
   }

   void init() 
   { 
      
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
      glGetDoublev(GL_MODELVIEW_MATRIX, m_mRotate); 
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
            Verts[i][j] = coef * BoxVerts[i][j]; 
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

   void draw() 
   {
      glLoadIdentity();
      glTranslatef(0.0, 0.0, -sdepth); 
      
      
      


      glMultMatrixd(m_mRotate); 
      glTranslatef(center_x, center_y, center_z); 

      glColor3f(color_r, color_g, color_b); 

      int i;
      int v1, v2, v3, v4;

      for (i = 0; i < 6; i++) {
         v1 = cubeIndices[i][0];
         v2 = cubeIndices[i][1];
         v3 = cubeIndices[i][2];
         v4 = cubeIndices[i][3];

         glBegin(GL_QUADS); 
         glNormal3f(bNorms[i][0], bNorms[i][1], bNorms[i][2]);
         glVertex3f(Verts[v1][0], Verts[v1][1], Verts[v1][2]);
         glNormal3f(bNorms[i][0], bNorms[i][1], bNorms[i][2]);
         glVertex3f(Verts[v2][0], Verts[v2][1], Verts[v2][2]);
         glNormal3f(bNorms[i][0], bNorms[i][1], bNorms[i][2]);
         glVertex3f(Verts[v3][0], Verts[v3][1], Verts[v3][2]);
         glNormal3f(bNorms[i][0], bNorms[i][1], bNorms[i][2]);
         glVertex3f(Verts[v4][0], Verts[v4][1], Verts[v4][2]);
         glEnd(); 
      }
   }

   
   bool hasUpIntersected(CSphere* sphere)
   {
      if (sphere->center_y + radius_sphere >= planeHeight / 2.1)
         return (true);
      return (false);
   }

   
   bool hasDownIntersected(CSphere* sphere, CSphere* Control_bar, CSphere* Control_bar1, CSphere*Control_bar2,
    CSphere* Control_bar3, CSphere* Control_bar4, CSphere* Control_bar5) {
      if (sphere->center_y - radius_sphere <= -1 * planeHeight / 2)
      {
          Life -= 1; 
          statecode = LIFE_DECREASE;
          sphere->center_x = hit_sphere_init_x;
          sphere->center_y = hit_sphere_init_y;
          sphere->center_z = 0.0;

          sphere->dir_x = 0.0;
          sphere->dir_y = 0.0;
          sphere->dir_z = 0.0;

          Control_bar->setCenter(Control_bar_init_x , Control_bar_init_y, 0.0); 
          Control_bar1->setCenter(Control_bar_init_x+0.5 , Control_bar_init_y, 0.0); 
          Control_bar2->setCenter(Control_bar_init_x+1.0, Control_bar_init_y, 0.0); 
          Control_bar3->setCenter(Control_bar_init_x-0.5, Control_bar_init_y, 0.0); 
          Control_bar4->setCenter(Control_bar_init_x-1, Control_bar_init_y, 0.0); 
          Control_bar5->setCenter(Control_bar_init_x+1.5, Control_bar_init_y, 0.0); 

          return (true);
      }
      return (false);
   }

   
   bool hasLeftIntersected(CSphere* sphere) {
      if (sphere->center_x - radius_sphere <= -1 * planeWidth / 2.1)
         return (true);
      return (false);
   }

   
   bool hasRightIntersected(CSphere* sphere)
   {
      if (sphere->center_x + radius_sphere >= planeWidth / 2.1)
         return (true);
      return (false);
   }


   void hitBy(CSphere* sphere, CSphere* Control_bar, CSphere* Control_bar1, CSphere* Control_bar2, CSphere* Control_bar3,
   CSphere* Control_bar4, CSphere* Control_bar5){
      if (hasUpIntersected(sphere))
      {
         sphere->dir_y = -(sphere->dir_y);

         
         
         while (hasUpIntersected(sphere)) {
            sphere->center_y -= 0.1;
         }
      }
      else if (hasDownIntersected(sphere, Control_bar,Control_bar1,Control_bar2,Control_bar3,Control_bar4,Control_bar5)) {
         sphere->dir_y = -(sphere->dir_y);

         
         
         while (hasDownIntersected(sphere, Control_bar,Control_bar1,Control_bar2,Control_bar3,Control_bar4,Control_bar5)) {
            sphere->center_y += 0.1;

         }


      }
      else if (hasLeftIntersected(sphere))
      {
         sphere->dir_x = -(sphere->dir_x);

         
         
         while (hasLeftIntersected(sphere)) {
            sphere->center_x += 0.1;
         }
      }
      else if (hasRightIntersected(sphere))
      {
         sphere->dir_x = -(sphere->dir_x);

         
         
         while (hasRightIntersected(sphere)) {
            sphere->center_x -= 0.1;
         }
      }
   }
};
class bar:public CSphere{
   public:
      void draw(){
      glLoadIdentity(); 
      glTranslatef(0.0, 0.0, -sdepth); 
      glMultMatrixd(m_mRotate); 
      glTranslated(center_x, center_y, center_z); 
      glColor3f(color_r, color_g, color_b); 
      glutSolidCube(1); 
      }
};
bar Control_bar, Control_bar1, Control_bar2, Control_bar3, Control_bar4, Control_bar5;
CSphere hit_sphere; 
CSphere target_sphere[NO_SPHERE]; 
CWall g_wall(planeWidth, planeHeight, planeDepth); 
CWall boundary_wall[4]; 



void InitObjects()
{
    
    hit_sphere.setColor(0.8, 0.2, 0.2); hit_sphere.setCenter(hit_sphere_init_x, hit_sphere_init_y, 0.0); 
    Control_bar.setColor(0.8, 0.8, 0.8); Control_bar.setCenter(Control_bar_init_x, Control_bar_init_y, 0.0); 
    Control_bar1.setColor(0.8, 0.8, 0.8); Control_bar1.setCenter(Control_bar_init_x+0.5, Control_bar_init_y, 0.0); 
    Control_bar2.setColor(0.8, 0.8, 0.8); Control_bar2.setCenter(Control_bar_init_x+1, Control_bar_init_y, 0.0); 
    Control_bar3.setColor(0.8, 0.8, 0.8); Control_bar3.setCenter(Control_bar_init_x-0.5, Control_bar_init_y, 0.0); 
    Control_bar4.setColor(0.8, 0.8, 0.8); Control_bar4.setCenter(Control_bar_init_x+1.5, Control_bar_init_y, 0.0); 
    Control_bar5.setColor(0.8, 0.8, 0.8); Control_bar5.setCenter(Control_bar_init_x-1, Control_bar_init_y, 0.0); 



    
    cnt_placed_sphere = 0;
    for (int i = 0; i < planeHeight; i++) {
        for (int j = 0; j < planeWidth; j++) {
            if (sphere_place[i][j] != '.') {
                target_sphere[cnt_placed_sphere].dir_x = 1;
                target_sphere[cnt_placed_sphere].setColor(1.0, 0.8314, 0.0);
                target_sphere[cnt_placed_sphere++].setCenter(-planeWidth / 2 + j + radius_sphere, planeHeight / 2 - i - radius_sphere, 0);
            }
        }
    }

    
    g_wall.setColor(0.078, 0.20, 0.024); g_wall.setCenter(0.0, 0.0, -0.6);
    boundary_wall[0].setColor(0.0,0.0,0.0);
    boundary_wall[0].setCenter(0.0, planeHeight / 2, 0.0); 
    boundary_wall[1].setColor(0.0,0.0,0.0);
    boundary_wall[1].setCenter(0.0, -(planeHeight / 2), 0.0); 
    boundary_wall[2].setColor(0.0,0.0,0.0);
    boundary_wall[2].setCenter(planeWidth / 2, 0.0, 0.0); 
    boundary_wall[3].setColor(0.0,0.0,0.0);
    boundary_wall[3].setCenter(-(planeWidth / 2), 0.0, 0.0); 

}



void ReshapeCallback(int width, int height)
{

   xsize = width;
   ysize = height;
   aspect = (float)xsize / (float)ysize;

   glViewport(0, 0, xsize, ysize);
   glMatrixMode(GL_PROJECTION);



   glLoadIdentity(); 
   gluPerspective(64.0, aspect, zNear, zFar); 

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
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
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
   
   
   
   glMatrixMode(GL_MODELVIEW);



   for (i = 0; i < cnt_placed_sphere; i++) {

           target_sphere[i].draw(); 

   }
   Control_bar.draw(); 
   Control_bar1.draw();
   Control_bar2.draw();
   Control_bar3.draw();
   Control_bar4.draw();
   Control_bar5.draw();

   hit_sphere.draw(); 
   g_wall.draw(); 
   for (int i = 0; i < 4; i++) if(i != 1) boundary_wall[i].draw(); 
   renderBitmapCharacter(35, 15.0, -2, GLUT_BITMAP_HELVETICA_18, (char*)((("SCORE : ")+to_string(Score)).c_str()));
   renderBitmapCharacter(35, 13.0, -2, GLUT_BITMAP_HELVETICA_18, (char*)((("LIFE : ") + to_string(Life)).c_str()));
    renderBitmapCharacter(35, 11.0, -2, GLUT_BITMAP_HELVETICA_18, (char*)((("PLAYER ") + to_string(Player)).c_str()));
    if (statecode==GAME_START||statecode==LIFE_DECREASE) {
        renderBitmapCharacter(11, -6.5, 10, GLUT_BITMAP_HELVETICA_18, (char*)"PRESS ENTER TO PLAY ARKANOID");
    }

   if (Life == 0) { 
       renderBitmapCharacter(12.8, -6.5, 5, GLUT_BITMAP_HELVETICA_18, (char*)"!!!!GAME OVER!!!!");
       renderBitmapCharacter(11.7, -7.5, 5, GLUT_BITMAP_HELVETICA_18, (char*)"PRESS R TO PLAY AGAIN");
       statecode = GAME_OVER;

   }

   if (Score >= cnt_placed_sphere) { 
       renderBitmapCharacter(12.6, -6.5, 5, GLUT_BITMAP_HELVETICA_18, (char*)
                             "YOU WON!!! PRESS R TO PLAY AGAIN");
      
       statecode = GAME_CLEAR;
   }

   glutSwapBuffers(); 
   

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


    case 13 : {

        switch (statecode) {
        case GAME_START: {
           
            hit_sphere.dir_x = 0.0; 
            hit_sphere.dir_y = 3.0;
            hit_sphere.dir_z = 0.0;
            statecode = GAME_PLAYING;
        }
        case LIFE_DECREASE: {
            hit_sphere.dir_x = 0.0; 
            hit_sphere.dir_y = 3.0;
            hit_sphere.dir_z = 0.0;
            statecode = GAME_PLAYING;
        }
        default: {
            
        }


        }
        break;
    }
    case 27: {
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
{ 
   downX = x; downY = y;
   leftButton = ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN));
   middleButton = ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_DOWN));
   rightButton = ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN));
   glutPostRedisplay();
}


void initRotate() { 

   for (int i = 0; i < NO_SPHERE; i++)
   {
      target_sphere[i].init();
   }
   hit_sphere.init();
   Control_bar.init();
   Control_bar1.init();
   Control_bar2.init();
   Control_bar3.init();
   Control_bar4.init();
   Control_bar5.init();

   g_wall.init();
   for (int i = 0; i < 4; i++) boundary_wall[i].init();

}

void InitGL() {
   
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); 
   glutInitWindowSize(1200, 800);
   glutCreateWindow("ARKANOID"); 
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);
   glClearColor(0.502, 0.502,0.502, 0);
   glPolygonOffset(1.0, 1.0);
   glDisable(GL_CULL_FACE);
   glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
   glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
   glEnable(GL_COLOR_MATERIAL);
   glColorMaterial(GL_FRONT, GL_DIFFUSE);
   glLightfv(GL_LIGHT0, GL_POSITION, light0Position);
   glEnable(GL_LIGHT0);
   initRotate(); 

   glShadeModel(GL_SMOOTH);
   glEnable(GL_LIGHTING);



   
   

   glutIdleFunc(renderScene); 
   glutReshapeFunc(ReshapeCallback); 
   
   glutDisplayFunc(DisplayCallback); 
   glutKeyboardFunc(KeyboardCallback); 
  
   glutSpecialFunc(SpecialCallback); 
   glutSpecialUpFunc(SpecialUpCallback); 
   glutMouseFunc(MouseCallback); 
}



int currentTime, previousTime = -1; 
void renderScene() 
{

   int timeDelta;
   currentTime = glutGet(GLUT_ELAPSED_TIME);
   if (previousTime == -1) timeDelta = 0;
   else timeDelta = (currentTime - previousTime) / 2;

   float x = hit_sphere.center_x;
   float y = hit_sphere.center_y;
   float z = hit_sphere.center_z;



   if (leftPressed) {
       
       if (statecode == GAME_START || statecode == LIFE_DECREASE) 
       {
           hit_sphere.setCenter(hit_sphere.center_x -timeDelta*0.05, hit_sphere.center_y, 0);
           Control_bar.setCenter(Control_bar.center_x -timeDelta*0.05, Control_bar.center_y, 0);
           Control_bar1.setCenter(Control_bar1.center_x -timeDelta*0.05, Control_bar1.center_y, 0);
           Control_bar2.setCenter(Control_bar2.center_x -timeDelta*0.05, Control_bar2.center_y, 0);
           Control_bar3.setCenter(Control_bar3.center_x -timeDelta*0.05, Control_bar3.center_y, 0);
           Control_bar4.setCenter(Control_bar4.center_x -timeDelta*0.05, Control_bar4.center_y, 0);
           Control_bar5.setCenter(Control_bar5.center_x -timeDelta*0.05, Control_bar5.center_y, 0);


       }
       else 
       {
           Control_bar.setCenter(Control_bar.center_x - timeDelta*0.05, Control_bar.center_y, 0);
           Control_bar1.setCenter(Control_bar1.center_x - timeDelta*0.05, Control_bar1.center_y, 0);
           Control_bar2.setCenter(Control_bar2.center_x - timeDelta*0.05, Control_bar2.center_y, 0);
           Control_bar3.setCenter(Control_bar3.center_x - timeDelta*0.05, Control_bar3.center_y, 0);
           Control_bar4.setCenter(Control_bar4.center_x - timeDelta*0.05, Control_bar4.center_y, 0);
           Control_bar5.setCenter(Control_bar5.center_x - timeDelta*0.05, Control_bar5.center_y, 0);

       }
       
   }

   if (rightPressed) {
       
       if (statecode == GAME_START || statecode == LIFE_DECREASE) 
       {
           hit_sphere.setCenter(hit_sphere.center_x + timeDelta*0.05, hit_sphere.center_y, 0);
           Control_bar.setCenter(Control_bar.center_x + timeDelta*0.05, Control_bar.center_y, 0);
           Control_bar1.setCenter(Control_bar1.center_x + timeDelta*0.05, Control_bar1.center_y, 0);
           Control_bar2.setCenter(Control_bar2.center_x + timeDelta*0.05, Control_bar2.center_y, 0);
           Control_bar3.setCenter(Control_bar3.center_x + timeDelta*0.05, Control_bar3.center_y, 0);
           Control_bar4.setCenter(Control_bar4.center_x + timeDelta*0.05, Control_bar4.center_y, 0);
           Control_bar5.setCenter(Control_bar5.center_x + timeDelta*0.05, Control_bar5.center_y, 0);

       }
       else 
       {
           Control_bar.setCenter(Control_bar.center_x + timeDelta*0.05, Control_bar.center_y, 0);
           Control_bar1.setCenter(Control_bar1.center_x + timeDelta*0.05, Control_bar1.center_y, 0);
           Control_bar2.setCenter(Control_bar2.center_x + timeDelta*0.05, Control_bar2.center_y, 0);
           Control_bar3.setCenter(Control_bar3.center_x + timeDelta*0.05, Control_bar3.center_y, 0);
           Control_bar4.setCenter(Control_bar4.center_x + timeDelta*0.05, Control_bar4.center_y, 0);
           Control_bar5.setCenter(Control_bar5.center_x + timeDelta*0.05, Control_bar5.center_y, 0);


       }
       
   }
    

     if (statecode == GAME_PLAYING) {
         x = hit_sphere.center_x;
         y = hit_sphere.center_y;
         z = hit_sphere.center_z;

         hit_sphere.setCenter(
             x + timeDelta * 0.010 * hit_sphere.dir_x, 
             y + timeDelta * 0.010 * hit_sphere.dir_y,
             z + timeDelta * 0.010 * hit_sphere.dir_z);
     }
   glutPostRedisplay(); 

   
   
    if (hit_sphere.hasIntersected(Control_bar) == true) 
    {
        hit_sphere.hitBy(Control_bar);
    }
   if (hit_sphere.hasIntersected(Control_bar1) == true) 
    {
        hit_sphere.hitBy(Control_bar1);
    }
   if (hit_sphere.hasIntersected(Control_bar2) == true) 
    {
        hit_sphere.hitBy(Control_bar2);
    }
   if (hit_sphere.hasIntersected(Control_bar3) == true) 
    {
        hit_sphere.hitBy(Control_bar3);
    }
   if (hit_sphere.hasIntersected(Control_bar4) == true) 
    {
        hit_sphere.hitBy(Control_bar4);
    }
   if (hit_sphere.hasIntersected(Control_bar4) == true) 
    {
        hit_sphere.hitBy(Control_bar5);
    }

    
    for(int i = 0; i < cnt_placed_sphere; i++){
        if (hit_sphere.hasIntersected(target_sphere[i]) == true) 
        {
            hit_sphere.hitBy(target_sphere[i]); 
            target_sphere[i].setCenter(0, 500, 500); 
            target_sphere[i].dir_x = 0;
            Score += 1;
            break;
        }
    }
    

   
    g_wall.hitBy(&hit_sphere, &Control_bar,&Control_bar1,&Control_bar2,&Control_bar3,&Control_bar4,&Control_bar5);


    
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
