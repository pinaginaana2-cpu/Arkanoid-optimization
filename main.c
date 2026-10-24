 
#include <GLUT/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
 
#define WIDTH 800
#define HEIGHT 600
#define PADDLE_WIDTH 100
#define PADDLE_HEIGHT 15
#define PADDLE_Y 50
#define BALL_SIZE 8
#define BALL_SPEED 4.0f
#define ROWS 12
#define COLS 14
#define BLOCK_WIDTH 55
#define BLOCK_HEIGHT 20
 
typedef enum { BLOCK_NORMAL = 0, BLOCK_TOUGH, BLOCK_INDESTRUCTIBLE, BLOCK_EMPTY } BlockType;
typedef enum { BONUS_NONE = 0, BONUS_EXTEND, BONUS_SLOW, BONUS_LIFE, BONUS_BIG_BALL } BonusType;
 
typedef struct {
   BlockType type;
   int hits;
   int active;
} Block;
typedef struct {
   BonusType type;
   float x, y;
   int active;
   float speedY;
   float animAngle;
} Bonus;
 
// Игровое поле
Block blocks[ROWS][COLS];
Bonus bonuses[50];
int bonusCount = 0;
// платформа
float paddleX;
int paddleWidth = PADDLE_WIDTH;
// мяч
float ballX, ballY;
float ballVx = BALL_SPEED, ballVy = BALL_SPEED;
int ballActive = 0;
int ballSize = BALL_SIZE;
//счёт и прогресс
int score = 0;
int lives = 3;
int blocksRemaining = 0;
float gameTime = 0.0f;
int gameRunning = 1;
int currentLevel = 1;
//таймеры бонусов,счетчики кадров
int extendedTimer = 0;
int slowTimer = 0;
int bigBallTimer = 0;
float originalBallSpeed = BALL_SPEED;
//состояние игры
int gameStarted = 0;
int leftPressed = 0, rightPressed = 0;
 
float bgOffset = 0.0f;
 
void initLevel(int level);
void drawText(float x, float y, const char* text, float r, float g, float b);
void drawTextShadow(float x, float y, const char* text, float r, float g, float b);
void drawBlock(float x, float y, BlockType type, int hits);
void drawPaddle();
void drawBall();
void updateBonuses();
void applyBonus(BonusType type);
void spawnBonus(float x, float y, BonusType type);
void updateGame();
void updatePaddleFromKeys();
void display();
void keyboard(unsigned char key, int x, int y);
void specialKeys(int key, int x, int y);
void specialKeysUp(int key, int x, int y);
void mouseClick(int button, int state, int x, int y);
void timer(int value);
void drawGradientBackground();
 
//рисование текста
void drawText(float x, float y, const char* text, float r, float g, float b) {
   glColor3f(r, g, b);
   glRasterPos2f(x, y);
   for (const char* c = text; *c != '\0'; c++)
       glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
}
//эффект тени
void drawTextShadow(float x, float y, const char* text, float r, float g, float b) {
   drawText(x + 1, y - 1, text, 0, 0, 0);
   drawText(x, y, text, r, g, b);
}
//отрисовка фона
void drawGradientBackground() {
   glDisable(GL_LIGHTING);
   glBegin(GL_QUADS);
   glColor3f(0.05f, 0.05f, 0.15f);
   glVertex2f(0, HEIGHT);
   glVertex2f(WIDTH, HEIGHT);
   glColor3f(0.0f, 0.0f, 0.05f);
   glVertex2f(WIDTH, 0);
   glVertex2f(0, 0);
   glEnd();
   //звезды
   glPointSize(1.5f);
   glBegin(GL_POINTS);
   for (int i = 0; i < 100; i++) {
       float starX = fmod(sin(i * 123.456f) * 1000, WIDTH);
       float starY = fmod(cos(i * 78.901f) * 800, HEIGHT - 150) + 150;
       float brightness = 0.3f + sin(gameTime + i) * 0.2f;
       glColor3f(brightness, brightness, brightness);
       glVertex2f(starX, starY);
   }
   glEnd();
}
 
void drawBlock(float x, float y, BlockType type, int hits) {
   glBegin(GL_QUADS);
   switch(type) {
       case BLOCK_NORMAL:
           glColor3f(0.2f, 0.9f, 0.2f);
           glVertex2f(x, y);
           glColor3f(0.1f, 0.6f, 0.1f);
           glVertex2f(x + BLOCK_WIDTH, y);
           glColor3f(0.05f, 0.4f, 0.05f);
           glVertex2f(x + BLOCK_WIDTH, y + BLOCK_HEIGHT);
           glColor3f(0.15f, 0.7f, 0.15f);
           glVertex2f(x, y + BLOCK_HEIGHT);
           break;
       case BLOCK_TOUGH:
           if (hits == 2) {
               glColor3f(1.0f, 0.8f, 0.2f);
               glVertex2f(x, y);
               glColor3f(0.9f, 0.6f, 0.1f);
               glVertex2f(x + BLOCK_WIDTH, y);
           } else {
               glColor3f(0.9f, 0.5f, 0.1f);
               glVertex2f(x, y);
               glColor3f(0.7f, 0.3f, 0.0f);
               glVertex2f(x + BLOCK_WIDTH, y);
           }
           glColor3f(0.6f, 0.3f, 0.0f);
           glVertex2f(x + BLOCK_WIDTH, y + BLOCK_HEIGHT);
           glColor3f(0.8f, 0.4f, 0.05f);
           glVertex2f(x, y + BLOCK_HEIGHT);
           break;
       case BLOCK_INDESTRUCTIBLE:
           glColor3f(0.4f, 0.4f, 0.5f);
           glVertex2f(x, y);
           glColor3f(0.3f, 0.3f, 0.4f);
           glVertex2f(x + BLOCK_WIDTH, y);
           glColor3f(0.2f, 0.2f, 0.3f);
           glVertex2f(x + BLOCK_WIDTH, y + BLOCK_HEIGHT);
           glColor3f(0.35f, 0.35f, 0.45f);
           glVertex2f(x, y + BLOCK_HEIGHT);
           break;
       default: return;
   }
   glEnd();
   //белая рамка вокруг блока
   glColor3f(1.0f, 1.0f, 1.0f);
   glLineWidth(1.0f);
   glBegin(GL_LINE_LOOP);
   glVertex2f(x, y);
   glVertex2f(x + BLOCK_WIDTH, y);
   glVertex2f(x + BLOCK_WIDTH, y + BLOCK_HEIGHT);
   glVertex2f(x, y + BLOCK_HEIGHT);
   glEnd();
   //блик
   glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
   glBegin(GL_QUADS);
   glVertex2f(x + 2, y + BLOCK_HEIGHT - 3);
   glVertex2f(x + BLOCK_WIDTH - 2, y + BLOCK_HEIGHT - 3);
   glVertex2f(x + BLOCK_WIDTH - 2, y + BLOCK_HEIGHT - 1);
   glVertex2f(x + 2, y + BLOCK_HEIGHT - 1);
   glEnd();
}
 
void drawPaddle() {
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   //свечение вокруг платформы
   glColor4f(0.0f, 0.5f, 1.0f, 0.3f);
   glBegin(GL_QUADS);
   glVertex2f(paddleX - 5, PADDLE_Y - 3);
   glVertex2f(paddleX + paddleWidth + 5, PADDLE_Y - 3);
   glVertex2f(paddleX + paddleWidth + 5, PADDLE_Y + PADDLE_HEIGHT + 3);
   glVertex2f(paddleX - 5, PADDLE_Y + PADDLE_HEIGHT + 3);
   glEnd();
   //основная платформа
   glBegin(GL_QUADS);
   glColor3f(0.2f, 0.6f, 1.0f);
   glVertex2f(paddleX, PADDLE_Y);
   glColor3f(0.0f, 0.4f, 0.9f);
   glVertex2f(paddleX + paddleWidth, PADDLE_Y);
   glColor3f(0.0f, 0.2f, 0.7f);
   glVertex2f(paddleX + paddleWidth, PADDLE_Y + PADDLE_HEIGHT);
   glColor3f(0.1f, 0.4f, 0.8f);
   glVertex2f(paddleX, PADDLE_Y + PADDLE_HEIGHT);
   glEnd();
   //блик на платформе
   glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
   glBegin(GL_QUADS);
   glVertex2f(paddleX + 2, PADDLE_Y + PADDLE_HEIGHT - 2);
   glVertex2f(paddleX + paddleWidth - 2, PADDLE_Y + PADDLE_HEIGHT - 2);
   glVertex2f(paddleX + paddleWidth - 2, PADDLE_Y + PADDLE_HEIGHT - 1);
   glVertex2f(paddleX + 2, PADDLE_Y + PADDLE_HEIGHT - 1);
   glEnd();
   glDisable(GL_BLEND);
}
 
void drawBall() {
   int currentBallSize = ballSize;
   if (bigBallTimer > 0) currentBallSize = BALL_SIZE + 6;
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   //внешнее свечение
   glColor4f(1.0f, 0.3f, 0.3f, 0.4f);
   glBegin(GL_QUADS);
   glVertex2f(ballX - currentBallSize/2 - 3, ballY - currentBallSize/2 - 3);
   glVertex2f(ballX + currentBallSize/2 + 3, ballY - currentBallSize/2 - 3);
   glVertex2f(ballX + currentBallSize/2 + 3, ballY + currentBallSize/2 + 3);
   glVertex2f(ballX - currentBallSize/2 - 3, ballY + currentBallSize/2 + 3);
   glEnd();
   //основной мяч
   glBegin(GL_QUADS);
   glColor3f(1.0f, 0.3f, 0.3f);
   glVertex2f(ballX - currentBallSize/2, ballY - currentBallSize/2);
   glColor3f(0.9f, 0.2f, 0.2f);
   glVertex2f(ballX + currentBallSize/2, ballY - currentBallSize/2);
   glColor3f(0.7f, 0.1f, 0.1f);
   glVertex2f(ballX + currentBallSize/2, ballY + currentBallSize/2);
   glColor3f(0.85f, 0.15f, 0.15f);
   glVertex2f(ballX - currentBallSize/2, ballY + currentBallSize/2);
   glEnd();
   //блик
   glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
   glBegin(GL_QUADS);
   glVertex2f(ballX - currentBallSize/4, ballY + currentBallSize/4);
   glVertex2f(ballX, ballY + currentBallSize/4);
   glVertex2f(ballX, ballY + currentBallSize/2);
   glVertex2f(ballX - currentBallSize/4, ballY + currentBallSize/2);
   glEnd();
   glDisable(GL_BLEND);
}
 
void spawnBonus(float x, float y, BonusType type) {
   if (bonusCount >= 50) return;
   bonuses[bonusCount].type = type;
   bonuses[bonusCount].x = x;
   bonuses[bonusCount].y = y;
   bonuses[bonusCount].active = 1;
   bonuses[bonusCount].speedY = -2.0f;
   bonuses[bonusCount].animAngle = 0;
   bonusCount++;
}
 
void applyBonus(BonusType type) {
   switch(type) {
       case BONUS_EXTEND:
           paddleWidth = PADDLE_WIDTH + 50;
           if (paddleWidth > 200) paddleWidth = 200;
           extendedTimer = 500;
           break;
       case BONUS_SLOW:
           if (!slowTimer) originalBallSpeed = fabs(ballVx);
           ballVx = (ballVx > 0 ? 2.0f : -2.0f);
           ballVy = (ballVy > 0 ? 2.0f : -2.0f);
           slowTimer = 500;
           break;
       case BONUS_LIFE:
           lives++;
           break;
       case BONUS_BIG_BALL:
           bigBallTimer = 500;
           break;
       default: break;
   }
}
 
void updateBonuses() {
   for (int i = 0; i < bonusCount; i++) {
       if (!bonuses[i].active)
           continue;
       bonuses[i].y += bonuses[i].speedY;
       bonuses[i].animAngle += 0.1f;
       //проверка столкновения с платформой
       if (bonuses[i].y + 10 >= PADDLE_Y && bonuses[i].y <= PADDLE_Y + PADDLE_HEIGHT &&
           bonuses[i].x >= paddleX && bonuses[i].x <= paddleX + paddleWidth) {
           applyBonus(bonuses[i].type);
           bonuses[i].active = 0;
       }
       if (bonuses[i].y < 0) bonuses[i].active = 0;
   }
}
 
void initLevel(int level) {
   //сброс всех прараметров
   paddleWidth = PADDLE_WIDTH;
   paddleX = WIDTH/2 - paddleWidth/2;
   extendedTimer = 0;
   slowTimer = 0;
   bigBallTimer = 0;
   ballSize = BALL_SIZE;
   ballVx = BALL_SPEED;
   ballVy = BALL_SPEED;
   //очистка блоков
   blocksRemaining = 0;
   for (int i = 0; i < ROWS; i++)
       for (int j = 0; j < COLS; j++)
           blocks[i][j].active = 0;
   
   if (level == 1) {
       for (int i = 0; i < 6; i++)
           for (int j = 0; j < 12; j++) {
               blocks[i][j+1].active = 1;
               blocks[i][j+1].type = BLOCK_NORMAL;
               blocks[i][j+1].hits = 1;
               blocksRemaining++;
           }
       for (int j = 3; j < 10; j++) {
           blocks[3][j].type = BLOCK_TOUGH;
           blocks[3][j].hits = 2;
       }
   }
   else if (level == 2) {
       for (int i = 0; i < 8; i++)
           for (int j = 4 - i/2; j <= 7 + i/2; j++)
               if (j >= 1 && j <= 12) {
                   blocks[i][j].active = 1;
                   blocks[i][j].type = (i % 3 == 0) ? BLOCK_TOUGH : BLOCK_NORMAL;
                   blocks[i][j].hits = (i % 3 == 0) ? 2 : 1;
                   blocksRemaining++;
               }
       for (int i = 0; i < 4; i++) {
           blocks[i][0].active = blocks[i][COLS-1].active = 1;
           blocks[i][0].type = blocks[i][COLS-1].type = BLOCK_INDESTRUCTIBLE;
           blocks[i][0].hits = blocks[i][COLS-1].hits = 999;
       }
   }
   else if (level == 3) {
       for (int i = 0; i < 8; i++)
           for (int j = 1; j < COLS-1; j++)
               if ((i + j) % 2 == 0) {
                   blocks[i][j].active = 1;
                   blocks[i][j].type = (i > 4) ? BLOCK_TOUGH : BLOCK_NORMAL;
                   blocks[i][j].hits = (i > 4) ? 2 : 1;
                   blocksRemaining++;
               }
       for (int row = 8; row <= 9; row++) {
           for (int j = 0; j < COLS; j++) {
               if ((j < 2) || (j > COLS-3) || (j >= 5 && j <= 8)) {
                   blocks[row][j].active = 1;
                   blocks[row][j].type = BLOCK_INDESTRUCTIBLE;
                   blocks[row][j].hits = 999;
                   blocksRemaining++;}
           }
       }
   }
}
 
void updatePaddleFromKeys() {
   int speed = 8;
   if (leftPressed) {
       paddleX -= speed;
       if (paddleX < 0)
           paddleX = 0;
       if (!ballActive && gameStarted)
           ballX = paddleX + paddleWidth/2;
   }
   if (rightPressed) {
       paddleX += speed;
       if (paddleX + paddleWidth > WIDTH)
           paddleX = WIDTH - paddleWidth;
       if (!ballActive && gameStarted)
           ballX = paddleX + paddleWidth/2;
   }
}
 
void updateGame() {
   updatePaddleFromKeys();
   if (!gameRunning || !gameStarted)
       return;
   
   if (extendedTimer > 0) {
       extendedTimer--;
       if (extendedTimer == 0)
           paddleWidth = PADDLE_WIDTH;
   }
   if (slowTimer > 0) {
       slowTimer--;
       if (slowTimer == 0) {
           float factor = originalBallSpeed / fabs(ballVx);
           ballVx *= factor;
           ballVy *= factor;
       }
   }
   if (bigBallTimer > 0) {
       bigBallTimer--;
       if (bigBallTimer == 0)
           ballSize = BALL_SIZE;
   }
   //движение мяча и границы
   if (ballActive) {
       ballX += ballVx;
       ballY += ballVy;
       //отскок от левой стены
       if (ballX - ballSize/2 <= 0) {
           ballX = ballSize/2;
           ballVx = -ballVx;
       }
       //отскок от правой стены
       if (ballX + ballSize/2 >= WIDTH) {
           ballX = WIDTH - ballSize/2;
           ballVx = -ballVx;
       }
       //отскок от верхней стены
       if (ballY + ballSize/2 >= HEIGHT) {
           ballY = HEIGHT - ballSize/2;
           ballVy = -ballVy;
       }
       //проверка столкновения с платформой
       if (ballY - ballSize/2 <= PADDLE_Y + PADDLE_HEIGHT &&
           ballY + ballSize/2 >= PADDLE_Y &&
           ballX + ballSize/2 >= paddleX &&
           ballX - ballSize/2 <= paddleX + paddleWidth) {
           
           ballVy = fabs(ballVy);
           
           float hitPos = (ballX - paddleX) / paddleWidth;
           float angle = (hitPos - 0.5f) * 2.5f;
           ballVx += angle * 1.5f;
           
           if (ballVx > 6.0f)
               ballVx = 6.0f;
           if (ballVx < -6.0f)
               ballVx = -6.0f;
           if (fabs(ballVx) < 1.5f) ballVx = (ballVx > 0) ? 1.5f : -1.5f;
           
           ballY = PADDLE_Y + PADDLE_HEIGHT + ballSize/2;
       }
       //потеря жизни
       //проверка мяч упал вниз
       if (ballY - ballSize/2 <= 0) {
           lives--;
           ballActive = 0;
           gameStarted = 0;
           ballX = paddleX + paddleWidth/2;
           ballY = PADDLE_Y + PADDLE_HEIGHT + ballSize;
           ballVx = BALL_SPEED;
           ballVy = BALL_SPEED;
           if (lives <= 0) {
               gameRunning = 0;
           }
           return;
       }
       //столкновения с блоками
       int startX = (WIDTH - COLS * BLOCK_WIDTH) / 2;
       int startY = HEIGHT - 100;
       
       for (int i = 0; i < ROWS; i++) {
           for (int j = 0; j < COLS; j++) {
               if (!blocks[i][j].active)
                   continue;
               
               
               float bx = startX + j * BLOCK_WIDTH;
               float by = startY - i * BLOCK_HEIGHT;
               
               float ballLeft = ballX - ballSize/2;
               float ballRight = ballX + ballSize/2;
               float ballBottom = ballY - ballSize/2;
               float ballTop = ballY + ballSize/2;
               
               if (ballRight > bx && ballLeft < bx + BLOCK_WIDTH &&
                   ballTop > by && ballBottom < by + BLOCK_HEIGHT) {
                   //определение стороны столкновения
                   float overlapLeft = ballRight - bx;
                   float overlapRight = (bx + BLOCK_WIDTH) - ballLeft;
                   float overlapTop = ballTop - by;
                   float overlapBottom = (by + BLOCK_HEIGHT) - ballBottom;
                   
                   float minOverlap = overlapLeft;
                   if (overlapRight < minOverlap)
                       minOverlap = overlapRight;
                   if (overlapTop < minOverlap)
                       minOverlap = overlapTop;
                   if (overlapBottom < minOverlap)
                       minOverlap = overlapBottom;
                   
                   if (minOverlap == overlapLeft || minOverlap == overlapRight)
                       ballVx = -ballVx;
                   else
                       ballVy = -ballVy;
                   
                   int bonusChance = (bigBallTimer > 0) ? 50 : 30;
                   //обработка разрушения блока
                   if (blocks[i][j].type == BLOCK_TOUGH) {
                       blocks[i][j].hits--;
                       if (blocks[i][j].hits <= 0) {
                           blocks[i][j].active = 0;
                           blocksRemaining--;
                           score += 20;
                           if (rand() % 100 < bonusChance) {
                               spawnBonus(bx + BLOCK_WIDTH/2, by + BLOCK_HEIGHT/2,   1+rand() % 4);
                           }
                       }
                   } else if (blocks[i][j].type == BLOCK_NORMAL) {
                       blocks[i][j].active = 0;
                       blocksRemaining--;
                       score += 10;
                       if (rand() % 100 < bonusChance) {
                           spawnBonus(bx + BLOCK_WIDTH/2, by + BLOCK_HEIGHT/2, 1+rand() % 4);
                       }
                   }
                   goto collision_done;
               }
           }
       }
       collision_done:;
       //проверка завершения уровня
       if (blocksRemaining == 0) {
           if (currentLevel < 3) {
               currentLevel++;
               initLevel(currentLevel);
               ballActive = 0;
               gameStarted = 0;
               ballX = paddleX + paddleWidth/2;
               ballY = PADDLE_Y + PADDLE_HEIGHT + ballSize;
           } else {
               gameRunning = 0;
           }
       }
   }
   updateBonuses();
}
// отрисовка кадра
void display() {
   drawGradientBackground();
   // отрисовка блоков
   int startX = (WIDTH - COLS * BLOCK_WIDTH) / 2;
   int startY = HEIGHT - 100;
   for (int i = 0; i < ROWS; i++)
       for (int j = 0; j < COLS; j++)
           if (blocks[i][j].active)
               drawBlock(startX + j * BLOCK_WIDTH, startY - i * BLOCK_HEIGHT,
                        blocks[i][j].type, blocks[i][j].hits);
   
   drawPaddle();
   drawBall();
   // отрисовка бонусов
   for (int i = 0; i < bonusCount; i++) {
       if (bonuses[i].active) {
           float size = 12 + sin(bonuses[i].animAngle) * 2;
           switch(bonuses[i].type) {
               case BONUS_EXTEND: glColor3f(0.0f, 1.0f, 1.0f);
                   break;
               case BONUS_SLOW: glColor3f(0.5f, 0.0f, 1.0f);
                   break;
               case BONUS_LIFE: glColor3f(1.0f, 0.2f, 0.2f);
                   break;
               case BONUS_BIG_BALL: glColor3f(1.0f, 0.8f, 0.0f);
                   break;
               default: glColor3f(1.0f, 1.0f, 1.0f);
           }
           glBegin(GL_QUADS);
           glVertex2f(bonuses[i].x - size/2, bonuses[i].y - size/2);
           glVertex2f(bonuses[i].x + size/2, bonuses[i].y - size/2);
           glVertex2f(bonuses[i].x + size/2, bonuses[i].y + size/2);
           glVertex2f(bonuses[i].x - size/2, bonuses[i].y + size/2);
           glEnd();
       }
   }
   //интерфейс
   char text[256];
   sprintf(text, "SCORE: %d", score);
   drawTextShadow(15, HEIGHT - 30, text, 1.0f, 0.8f, 0.2f);
   
   sprintf(text, "LIVES: %d", lives);
   drawTextShadow(180, HEIGHT - 30, text, 1.0f, 0.3f, 0.3f);
   
   sprintf(text, "LEVEL: %d", currentLevel);
   drawTextShadow(350, HEIGHT - 30, text, 0.3f, 0.8f, 1.0f);
   
   sprintf(text, "BLOCKS: %d", blocksRemaining);
   drawTextShadow(520, HEIGHT - 30, text, 0.5f, 1.0f, 0.5f);
   
   sprintf(text, "TIME: %.0f", gameTime);
   drawTextShadow(680, HEIGHT - 30, text, 0.8f, 0.8f, 0.8f);
   // информация о активных бонусах
   int yOffset = HEIGHT - 60;
   if (extendedTimer > 0) {
       drawText(15, yOffset, "BONUS: WIDE PADDLE", 0.0f, 1.0f, 1.0f);
       yOffset -= 20;
   }
   if (slowTimer > 0) {
       drawText(15, yOffset, "BONUS: SLOW BALL", 0.5f, 0.0f, 1.0f);
       yOffset -= 20;
   }
   if (bigBallTimer > 0) {
       drawText(15, yOffset, "BONUS: BIG BALL", 1.0f, 0.8f, 0.0f);
       yOffset -= 20;
   }
   //меню старта
   if (!gameStarted && gameRunning) {
       drawTextShadow(WIDTH/2 - 150, HEIGHT/2 + 60, " ARKANOID ", 1.0f, 0.5f, 0.0f);
       drawTextShadow(WIDTH/2 - 140, HEIGHT/2 + 30, "Press SPACE to launch ball", 1.0f, 1.0f, 1.0f);
       drawTextShadow(WIDTH/2 - 120, HEIGHT/2, "Use LEFT/RIGHT arrows", 0.8f, 0.8f, 0.8f);
       drawTextShadow(WIDTH/2 - 160, HEIGHT/2 - 30, "Press 1, 2 or 3 to select level", 0.5f, 1.0f, 0.5f);
       drawTextShadow(WIDTH/2 - 80, HEIGHT/2 - 60, "Press R to restart", 0.5f, 0.5f, 1.0f);
   }
   //экран победы/поражения
   if (!gameRunning) {
       if (lives > 0) {
           drawTextShadow(WIDTH/2 - 60, HEIGHT/2, "YOU WIN!", 1.0f, 0.8f, 0.0f);
           drawTextShadow(WIDTH/2 - 100, HEIGHT/2 - 30, "Press R to play again", 1.0f, 1.0f, 1.0f);
       } else {
           drawTextShadow(WIDTH/2 - 70, HEIGHT/2, "GAME OVER!", 1.0f, 0.2f, 0.2f);
           drawTextShadow(WIDTH/2 - 100, HEIGHT/2 - 30, "Press R to restart", 1.0f, 1.0f, 1.0f);
       }
   }
   
   glutSwapBuffers();
}
//обработчик клавиатуры
void keyboard(unsigned char key, int x, int y) {
   if (key == 27) exit(0);
   //перезапуск игры
   if (key == 'r' || key == 'R') {
       score = 0;
       lives = 3;
       gameRunning = 1;
       gameStarted = 0;
       ballActive = 0;
       gameTime = 0;
       initLevel(currentLevel);
       ballX = paddleX + paddleWidth/2;
       ballY = PADDLE_Y + PADDLE_HEIGHT + ballSize;
       bonusCount = 0;
       leftPressed = rightPressed = 0;
       ballVx = BALL_SPEED;
       ballVy = BALL_SPEED;
       extendedTimer = slowTimer = bigBallTimer = 0;
       ballSize = BALL_SIZE;
   }
   //выбор уровня
   if (!gameStarted && gameRunning) {
       if (key == '1') {
           currentLevel = 1;
           initLevel(1);
           ballX = paddleX + paddleWidth/2;
           ballY = PADDLE_Y + PADDLE_HEIGHT + ballSize;
       }
       if (key == '2') {
           currentLevel = 2;
           initLevel(2);
           ballX = paddleX + paddleWidth/2;
           ballY = PADDLE_Y + PADDLE_HEIGHT + ballSize;
       }
       if (key == '3') {
           currentLevel = 3;
           initLevel(3);
           ballX = paddleX + paddleWidth/2;
           ballY = PADDLE_Y + PADDLE_HEIGHT + ballSize;
       }
   }
   //запуск мяча(пробел)
   if (key == ' ' && gameRunning && !gameStarted) {
       gameStarted = 1;
       ballActive = 1;
   }
}
//отработчики стрелок
void specialKeys(int key, int x, int y) {
   if (!gameRunning)
       return;
   if (key == GLUT_KEY_LEFT)
       leftPressed = 1;
   if (key == GLUT_KEY_RIGHT)
       rightPressed = 1;
}
 
void specialKeysUp(int key, int x, int y) {
   if (key == GLUT_KEY_LEFT)
       leftPressed = 0;
   if (key == GLUT_KEY_RIGHT)
       rightPressed = 0;
}
//обработчик мыши
void mouseClick(int button, int state, int x, int y) {
   if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && gameRunning && !gameStarted) {
       gameStarted = 1;
       ballActive = 1;
   }
}
 
void timer(int value) {
   if (gameRunning && gameStarted) gameTime += 0.016f;
   bgOffset += 0.01f;
   updateGame();
   glutPostRedisplay();
   glutTimerFunc(16, timer, 0);
}
 
int main(int argc, char** argv) {
   srand((unsigned int)time(NULL));
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize(WIDTH, HEIGHT);
   glutCreateWindow("Arkanoid");
   
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D(0, WIDTH, 0, HEIGHT);
   glMatrixMode(GL_MODELVIEW);
   
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   
   paddleX = WIDTH/2 - paddleWidth/2;
   ballX = paddleX + paddleWidth/2;
   ballY = PADDLE_Y + PADDLE_HEIGHT + ballSize;
   ballActive = 0;
   gameStarted = 0;
   
   initLevel(1);
   
   glutDisplayFunc(display);
   glutKeyboardFunc(keyboard);
   glutSpecialFunc(specialKeys);
   glutSpecialUpFunc(specialKeysUp);
   glutMouseFunc(mouseClick);
   glutTimerFunc(16, timer, 0);
   
   glutMainLoop();
   return 0;
}