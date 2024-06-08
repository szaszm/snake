#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <time.h>
#include <stdint.h>

typedef struct __point_t {
	int x;
	int y;
} point_t;
typedef enum __direction_t { DIRECTION_UP, DIRECTION_DOWN, DIRECTION_LEFT, DIRECTION_RIGHT } direction_t;

struct snake_t {
	point_t *reszek;
	int hossz; // reszek[] hossza
	direction_t irany;
	double sleep;
} snake;
point_t kaja;

const point_t grid_size = { 60, 35 };
const int quad_size = 15;
const int reszek_kezdo = 6;
const int falon_atmegy = 1;

int snake_speed = 3;
int pontok = 0;
int ment = 1;
int vege = 0;
int rekord = 0;
int ujrekord = 0;

int frames = 0;
int fps = 0;

void draw_quad(point_t at);
void gameover(void);
void quit(int val);
void update_pontok(void);
void kaja_gen(void);
void kaja_draw(void);
void kaja_init(void);
void snake_setspeed(void);
void snake_init(void);
void snake_novel(point_t pos);
void snake_draw(void);
void snake_update(int val);
void calcFPS(int ignored);
void glut_draw(void);
void glut_keyboard(unsigned char key, int x, int y);
void glut_special(int key, int x, int y);
void glut_reshape(int width, int height);
void glut_render_fonts(char *text, int x, int y);
void update(int value);

int main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
	glutInitWindowSize(grid_size.x * quad_size, (grid_size.y + 1) * quad_size);
	glutCreateWindow("Snake game");

	glutDisplayFunc(glut_draw);
	glutKeyboardFunc(glut_keyboard);
	glutSpecialFunc(glut_special);
	glutReshapeFunc(glut_reshape);

	glutTimerFunc(1000, calcFPS, 0);
	glutTimerFunc(snake.sleep * 1000, snake_update, 0);

	snake_init();
	kaja_init();

	glutMainLoop();
	return EXIT_SUCCESS;
}

void draw_quad(point_t pos) {
	float start_x = -1.0f + pos.x * (2.0f / grid_size.x)   + (2.0f / (grid_size.x * quad_size));
	float start_y = 1.0f - pos.y * (2.0f / grid_size.y)    - (2.0f / (grid_size.y * quad_size));
	float end_x   = start_x + (2.0f / grid_size.x)         - (2.0f / (grid_size.x * quad_size));
	float end_y   = start_y - (2.0f / grid_size.y)         + (2.0f / (grid_size.y * quad_size));
	glBegin(GL_QUADS);
		glVertex3f(start_x, start_y, 0.0f);
		glVertex3f(start_x,   end_y, 0.0f);
		glVertex3f(  end_x,   end_y, 0.0f);
		glVertex3f(  end_x, start_y, 0.0f);
	glEnd();
}
void gameover(void) {
	FILE *rekordfajl;
	int ujfajl = 0;
	if((rekordfajl = fopen("rekord.txt", "r")) == NULL) {
		ujfajl = 1;
		rekordfajl = fopen("rekord.txt", "w");
		fclose(rekordfajl);
	}
	else {
		char buf[BUFSIZ];
		fgets(buf, BUFSIZ, rekordfajl);
		rekord = atoi(buf);
		fclose(rekordfajl);
	}
	if(rekord < pontok) {
		rekordfajl = fopen("rekord.txt", "w");
		fprintf(rekordfajl, "%d\n", pontok);
		fclose(rekordfajl);
		ujrekord = 1;
	}
	vege = 1;
	glutTimerFunc(3000, quit, 0);
}
void quit(int val) {
	free(snake.reszek);
	exit(0);
}
void update_pontok(void) {
	char text[BUFSIZ];
	sprintf(text, "pontok: %d   sebesseg: %d   FPS: %d", pontok, snake_speed, fps);
	glut_render_fonts(text, 5, 5);
}

/*      KAJA */
void kaja_gen(void) {
	int i;
	kaja.x = rand() % grid_size.x;
	kaja.y = rand() % grid_size.y;
	for(i = 0; i < snake.hossz; i++)
		if(snake.reszek[i].x == kaja.x && snake.reszek[i].y == kaja.y) kaja_gen();
}
void kaja_draw(void) {
	glColor3f(1.0f, 0.0f, 0.0f);
	draw_quad(kaja);
}
void kaja_init(void) {
	uint64_t seed = time(NULL);
	srand(seed);
	kaja_gen();
}
/* END: KAJA */


/*      SNAKE */
void snake_setspeed(void) {
	if(snake_speed < 1) snake_speed = 1;
	if(snake_speed > 7) snake_speed = 7;
	switch(snake_speed) {
		case 1: snake.sleep = 0.3;   break;
		case 2: snake.sleep = 0.2;   break;
		case 3: snake.sleep = 0.1;  break;
		case 4: snake.sleep = 0.075; break;
		case 5: snake.sleep = 0.05;   break;
		case 6: snake.sleep = 0.025;  break;
		case 7: snake.sleep = 0.010;   break;
	}
}
void snake_init(void) {
	if((snake.reszek = (point_t *)malloc(reszek_kezdo * sizeof(point_t))) == NULL) exit(EXIT_FAILURE);
	snake.hossz = reszek_kezdo;
	snake.irany = DIRECTION_RIGHT;
	snake_setspeed();
	int i;
	for(i = 0; i < reszek_kezdo; i++) {
		snake.reszek[i].x = 1 + reszek_kezdo - i;
		snake.reszek[i].y = grid_size.y / 2;
	}
}
void snake_novel(point_t oldlast) {
	if((snake.reszek = (point_t *)realloc(snake.reszek, ++snake.hossz * sizeof(point_t))) == NULL) exit(-1);
	snake.reszek[snake.hossz - 1] = oldlast;
}
void snake_draw(void) {
	glColor3f(0.0f, 1.0f, 0.0f);
	int i;
	for(i = 0; i < snake.hossz; i++) draw_quad(snake.reszek[i]);
}
void snake_update(int val) {
	point_t oldlast = snake.reszek[snake.hossz - 1];
	int i;
	for(i = snake.hossz - 1; i > 0; i--) {
		snake.reszek[i].x = snake.reszek[i-1].x;
		snake.reszek[i].y = snake.reszek[i-1].y;
	}
	switch(snake.irany) {
		case DIRECTION_UP:
			snake.reszek[0].y--;
			break;
		case DIRECTION_DOWN:
			snake.reszek[0].y++;
			break;
		case DIRECTION_LEFT:
			snake.reszek[0].x--;
			break;
		case DIRECTION_RIGHT:
			snake.reszek[0].x++;
			break;
	}
	// Falon athaladas
	if(falon_atmegy) {
		if(snake.reszek[0].x >= grid_size.x) snake.reszek[i].x -= grid_size.x;
		if(snake.reszek[0].y >= grid_size.y) snake.reszek[i].y -= grid_size.y;
		if(snake.reszek[0].x < 0) snake.reszek[i].x += grid_size.x;
		if(snake.reszek[0].y < 0) snake.reszek[i].y += grid_size.y;
	}
	else {
		if(snake.reszek[0].x >= grid_size.x || snake.reszek[0].x < 0 || snake.reszek[0].y >= grid_size.y || snake.reszek[0].y < 0) gameover();
	}
	// Onmagaba utkozes
	for(i = 1; i < snake.hossz; i++) 
		if(snake.reszek[0].x == snake.reszek[i].x && snake.reszek[0].y == snake.reszek[i].y) gameover();
	// Kaja
	if(snake.reszek[0].x == kaja.x && snake.reszek[0].y == kaja.y) {
		pontok += snake_speed; // Pont osztas sebessegtol fuggoen
		snake_novel(oldlast);
		kaja_gen();
	}

	ment = 1;
	update(0);
	glutTimerFunc(snake.sleep * 1000, snake_update, 0);
}
/* END: SNAKE */

/*      FPS CALCULATOR */
void calcFPS(int ign) {
	fps = frames;
	frames = 0;
	glutTimerFunc(1000, calcFPS, 0);
}

/*      GLUT */
void glut_draw(void) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	if(!vege) {
		snake_draw();
		kaja_draw();
		update_pontok();
		frames++;
	}
	else {
		char text[BUFSIZ];
		if(ujrekord) sprintf(text, "Jatek vege!  pontok: %d   Uj rekord!", pontok);
		else sprintf(text, "Jatek vege!  pontok: %d   rekord: %d", pontok, rekord);
		glut_render_fonts(text, 20, grid_size.y*5);
	}

	glutSwapBuffers();
}
void glut_keyboard(unsigned char key, int x, int y) {
	if(key == 'q' || key == 27) exit(0);
	if(key == '+') { if(snake_speed < 7) snake_speed++; snake_setspeed(); }
	if(key == '-') { if(snake_speed > 0) snake_speed--; snake_setspeed(); }
}
void glut_special(int key, int x, int y) {
	if(key == GLUT_KEY_UP && snake.irany != DIRECTION_DOWN && ment) {
		snake.irany = DIRECTION_UP;
		ment = 0;
	}
	if(key == GLUT_KEY_DOWN && snake.irany != DIRECTION_UP && ment) {
		snake.irany = DIRECTION_DOWN;
		ment = 0;
	}
	if(key == GLUT_KEY_LEFT && snake.irany != DIRECTION_RIGHT && ment) {
		snake.irany = DIRECTION_LEFT;
		ment = 0;
	}
	if(key == GLUT_KEY_RIGHT && snake.irany != DIRECTION_LEFT && ment) {
		snake.irany = DIRECTION_RIGHT;
		ment = 0;
	}
}
void glut_reshape(int width, int height) {
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
}
void glut_render_fonts(char *text, int x, int y) {
	glColor3f(1.0f, 1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, grid_size.x*quad_size, 0.0, grid_size.y*quad_size);

	glRasterPos2i(x, y);
	void *font = GLUT_BITMAP_9_BY_15;
	while(*text) glutBitmapCharacter(font, *text++);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}
void update(int val) {
	glutPostRedisplay();
}
/* END: GLUT */
