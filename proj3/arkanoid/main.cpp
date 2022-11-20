//
//  main.cpp
//  arkanoid
//
//  Created by Gangmin Lee on 2022/11/19.
//

//* Simple geometry viewer:  Left mouse: rotate;  Right mouse:   translate;  ESC to quit. */
#include "arkanoid.h"

// 추가해야함 using namespace std;
int main(int argc, char** argv)
{
   glutInit(&argc, argv); // glut 시작 초기화, 보통 그래픽못쓰는데서 한다거나 하면 오류 감지용
   InitObjects(); // 초기화면, 각 공의 위치를 세팅
   InitGL(); // opengl 관련 초기화
   glutMainLoop(); // 이벤트 루프를 돌리는것, 이벤트별로 콜백함수 등록을 마쳤으니 이벤트 루프로 진입하라.
   return 0;
}


