#include <GL/freeglut_std.h>
#include <math.h>
#include <stdio.h>

/* update to use glfw or sdl
   https://www.glfw.org/docs/latest/quick.html#quick_include,
   https://www.libsdl.org/release/SDL-1.2.15/docs/html/guidevideoopengl.html*/

#define WIDTH 1024
#define HEIGHT 512

#define PI M_PI
#define PI2 M_PI_2
#define PI3 3 * PI / 2

#define ONE_RAD M_PI / 180

typedef struct ButtonKeys {
  int a, d, w, s;
} Keyboard;

typedef struct Player {
  int x, y;
  float x_delta, y_delta, angle;
} Player;

typedef struct Ray {
  float x, y, xf, yf, magnitude;
  int angle;
} Ray;


Player player;
Keyboard keyboard;

/* frames and fps */
float frame1, fps, frame2;

/* map */
int mapX = 8, mapY = 8, mapS = 64;
int map[] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 1, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 1, 1, 1, 1, 1,
    1, 1, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};

float dist(float ax, float ay, float bx, float by, float angle) {
  return sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay));
}

void calculeMagnitude(Ray *ray) {
  ray->magnitude = sqrt((ray->xf - ray->x) * (ray->xf - ray->x) + (ray->yf - ray->y) * (ray->yf - ray->y));
}

float randRay(Ray *ray) {
  return ray->angle * PI / 180;
}

void drawMap2D() {
  int xo, yo;
  for (int y = 0; y < mapY; y++) {
    for (int x = 0; x < mapX; x++) {
      if (map[y * mapX + x]) {
        glColor3f(1, 1, 1);
      } else {
        glColor3f(0, 0, 0);
      }

      int xo = x * mapS;
      int yo = y * mapS;

      glBegin(GL_QUADS);
      glVertex2i(xo, yo);
      glVertex2i(xo, yo + mapS);
      glVertex2i(xo + mapS, yo + mapS);
      glVertex2i(xo + mapS, yo);
      glEnd();
    }
  }
}

void drawPlayer() {
  glColor3f(1, 1, 1);
  glPointSize(8);
  glBegin(GL_POINTS);
  glVertex2i(player.x, player.y);
  glEnd();

  glColor3f(1, 1, 1);
  glLineWidth(3);
  glBegin(GL_LINES);
  glVertex2i(player.x, player.y);
  glVertex2i(player.x + player.x_delta * 5, player.y + player.y_delta * 5);
  glEnd();
}

void updatePlayerPosition(float fps){
  if (keyboard.a == 1){
    player.angle -=  0.005 * fps;
    if (player.angle < 0)
      player.angle += PI * 2;

    player.x_delta = cos(player.angle) * 5;
    player.y_delta = sin(player.angle) * 5;
  }
  if (keyboard.d == 1){
    player.angle +=  0.005 * fps;
    if (player.angle > PI * 2)
      player.angle -= PI * 2;

    player.x_delta = cos(player.angle) * 5;
    player.y_delta = sin(player.angle) * 5;
  }
  if (keyboard.w == 1) {
    player.y += player.y_delta;
    player.x += player.x_delta;
  }
  if (keyboard.s == 1) {
    player.y -= player.y_delta;
    player.x -= player.x_delta;
  }
  glutPostRedisplay();
}

void drawRays2D() {
  /* N>>6 is equal N/64 (because 2^6=64) */
  int r, mx, my, mp, dof;
  float rx, ry, ra, xo, yo, distT;

  ra = player.angle - (30 * ONE_RAD);

  for (r = 0; r < 70; r++) {
    /* normalize ray angle*/
    if (ra < 0) {
      ra += 2 * PI;
    } else if (ra > 2 * PI) {
      ra -= 2 * PI;
    }

    /* horizontal  check */
    dof = 0;
    float distH = 1000000, hx = player.x, hy = player.y;
    float aTan = -1 / tan(ra);

    if (ra > PI) {
      ry = (((int)player.y >> 6) << 6) - 0.0001;
      rx = (player.y - ry) * aTan + player.x;
      yo = -64;
      xo = -yo * aTan;
    } else if (ra < PI) {
      ry = (((int)player.y >> 6) << 6) + 64;
      rx = (player.y - ry) * aTan + player.x;
      yo = 64;
      xo = -yo * aTan;
    } else if (ra == 0 || ra == PI) {
      rx = player.x;
      ry = player.y;
      dof = 8;
    }
    while (dof < 8) {
      mx = (int)(rx) >> 6;
      my = (int)(ry) >> 6;
      mp = my * mapX + mx;
      if (mp > 0 && mp < mapX * mapY && map[mp] == 1) {
        hx = rx;
        hy = ry;
        distH = dist(player.x, player.y, hx, hy, player.angle);
        dof = 8;
      } else {
        rx += xo;
        ry += yo;
        dof += 1;
      }
    }

    /* vertical check */
    dof = 0;
    float distV = 1000000, vx = player.x, vy = player.y;
    float nTan = -tan(ra);

    if (ra > PI2 && ra < PI3) {
      rx = (((int)player.x >> 6) << 6) - 0.0001;
      ry = (player.x - rx) * nTan + player.y;
      xo = -64;
      yo = -xo * nTan;
    } else if (ra < PI2 || ra > PI3) {
      rx = (((int)player.x >> 6) << 6) + 64;
      ry = (player.x - rx) * nTan + player.y;
      xo = 64;
      yo = -xo * nTan;
    } else if (ra == 0 || ra == PI) {
      rx = player.x;
      ry = player.y;
      dof = 8;
    }
    while (dof < 8) {
      mx = (int)(rx) >> 6;
      my = (int)(ry) >> 6;
      mp = my * mapX + mx;
      if (mp > 0 && mp < mapX * mapY && map[mp] == 1) {
        vx = rx;
        vy = ry;
        distV = dist(player.x, player.y, vx, vy, player.angle);
        dof = 8;
      } else {
        rx += xo;
        ry += yo;
        dof += 1;
      }
    }

    if (distV < distH) {
      rx = vx;
      ry = vy;
      distT = distV;
      glColor3f(0.9f, 0, 0);
    } else if (distH < distV) {
      rx = hx;
      ry = hy;
      distT = distH;
      glColor3f(0.6f, 0, 0);
    }

    printf("rx=%f ry=%f ", rx, ry);
    printf("dh=%f dv=%f\n", distH, distV);
    glLineWidth(1);
    glBegin(GL_LINES);
    glVertex2i(player.x, player.y);
    glVertex2i(rx, ry);
    glEnd();

    ra += ONE_RAD;

    /* draw 3d wall */

    float ca = player.angle - ra; /* fix fisheye */
    if (ca < 0) {
      ca += 2 * PI;
    } else if (ca > 2 * PI) {
      ca -= 2 * PI;
    }
    distT = distT * cos(ca);

    int lineH = (mapS * HEIGHT) / (distT);
    if (lineH > HEIGHT)
      lineH = HEIGHT;

    int lineOff = (HEIGHT / 2) - (lineH / 2);

    glLineWidth(8);
    glBegin(GL_LINES);
    glVertex2i(r * 8 + (WIDTH / 2), lineOff);
    glVertex2i(r * 8 + (WIDTH / 2), lineOff + lineH);
    glEnd();

    /* ceil */
    glColor3f(0, 0.5, 0);
    glLineWidth(8);
    glBegin(GL_LINES);
    glVertex2i(r * 8 + (WIDTH / 2), 0);
    glVertex2i(r * 8 + (WIDTH / 2), lineOff);
    glEnd();

    /* floor */
    glColor3f(0, 0.5, 0.6);
    glLineWidth(8);
    glBegin(GL_LINES);
    glVertex2i(r * 8 + (WIDTH / 2), lineOff + lineH);
    glVertex2i(r * 8 + (WIDTH / 2), HEIGHT);
    glEnd();
  }
}

void drawDisplay() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* frame calc here */
  frame2 = glutGet(GLUT_ELAPSED_TIME); fps=(frame2 - frame1); frame1 = glutGet(GLUT_ELAPSED_TIME);

  /* update here */
  updatePlayerPosition(fps);

  /* draw here */
  drawMap2D();
  drawPlayer();
  drawRays2D();

  printf("x=%i, y=%i=, dx=%f, dy=%f, pa=%f\n", player.x, player.y,
         player.x_delta, player.y_delta, player.angle);

  glutSwapBuffers();
}

void init() {
  glClearColor(.3f, .3f, .3f, 0);
  gluOrtho2D(0, WIDTH, HEIGHT, 0);

  player.x = 300;
  player.y = 300;
  player.angle = 2 * PI;
  player.x_delta = cos(player.angle) * 5;
  player.y_delta = sin(player.angle) * 5;
}

void forceReshapeWindow(int w, int h) {
  glutReshapeWindow(WIDTH, HEIGHT);
}


void handlerButton(unsigned char key, int x, int y) {
  switch (key) {
  case 'a':
    keyboard.a = 1;
    break;
  case 'd':
    keyboard.d = 1;
    break;
  case 'w':
    keyboard.w = 1;
    break;
  case 's':
    keyboard.s = 1;
    break;
  }
}


void handlerButtonUp(unsigned char key, int x, int y) {
  switch (key) {
  case 'a':
    keyboard.a = 0;
    break;
  case 'd':
    keyboard.d = 0;
    break;
  case 'w':
    keyboard.w = 0;
    break;
  case 's':
    keyboard.s = 0;
    break;
  }
}


int main(int argc, char *argv[]) {
  /* TODO - comment all opengl stuff, like a documentation resume*/
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize(WIDTH, HEIGHT);
  glutInitWindowPosition(200, 200);
  glutCreateWindow("");
  init();
  glutReshapeFunc(forceReshapeWindow);
  glutDisplayFunc(drawDisplay);
  glutKeyboardFunc(handlerButton);
  glutKeyboardUpFunc(handlerButtonUp);
  glutMainLoop();
}
