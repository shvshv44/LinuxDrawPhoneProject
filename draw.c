#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define TOOL_PENCIL 0

#define COLOR_BLACK 0xFF000000
#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_RED   0xFFFF0000
#define COLOR_GREEN 0xFF00FF00
#define COLOR_BLUE  0xFF0000FF

typedef struct {
	SDL_Window* win;
	SDL_Surface* screen;
	uint32_t color;
	int mouse_down;
	int win_width;
	int win_height;
	int x;
	int y;
} draw_state;

void draw_exit(int code)
{
	exit(code);
}

void draw_error(const char* str)
{
	printf("ERROR: %s", str);
	draw_exit(1);
}

void draw_pencil(draw_state* m, int x, int y)
{
	uint32_t* pixels = m->screen->pixels;
	if (SDL_MUSTLOCK(m->screen)) {
		SDL_LockSurface(m->screen);
	}

	y -= 16;

	pixels[(y*m->screen->w)+x] = m->color;
	if (SDL_MUSTLOCK(m->screen)) {
		SDL_UnlockSurface(m->screen);
	}
}

void draw_rect(draw_state* m, int x, int y, int w, int h, int color) {
	SDL_Rect rect;
	rect.w = w;
	rect.h = h;
	rect.x = x;
	rect.y = y;
	SDL_FillRect(m->screen, &rect, color);
}

void draw_render(draw_state* m)
{
	int l = 32;
	draw_rect(m, 0, 0, l, l, COLOR_BLUE);
	draw_rect(m, l , 0, l, l, COLOR_WHITE);
	draw_rect(m, l * 2, 0, l, l, COLOR_RED);
	draw_rect(m, l * 3, 0, l, l, COLOR_GREEN);
	SDL_UpdateWindowSurface(m->win);
}

draw_state* draw_init(void)
{
	draw_state* m = malloc(sizeof(*m));
	if (m == NULL) {
		draw_error("Failed to initialize state!");
	}

	m->color = COLOR_BLACK;
	m->mouse_down = 0;
	m->win_width = 640;
	m->win_height = 480;
	m->x = 0;
	m->y = 0;

	return m;
}

int main(int argc, char* argv[])
{
	int xMouse, yMouse;
	draw_state* m = draw_init();

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		draw_error("Failed to initialize SDL!");
	}

	m->win = SDL_CreateWindow("draw something", 0, 0, m->win_width, m->win_height, SDL_WINDOW_SHOWN);
	if (m->win == NULL) {
		draw_error("Failed to initialize SDL window!");
	}

	m->screen = SDL_GetWindowSurface(m->win);
	m->color = COLOR_WHITE;

	/* so we don't start with a black m->screen */
	draw_render(m);
	while (1) {
		SDL_Event e;
		
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				draw_exit(0);
				break;
			case SDL_KEYDOWN:
				if (e.key.keysym.sym == SDLK_q || e.key.keysym.sym == SDLK_ESCAPE) {
					draw_exit(0);
				}
				if (e.key.keysym.sym == SDLK_1) {
					m->color = COLOR_BLUE;
				}
				if (e.key.keysym.sym == SDLK_2) {
					m->color = COLOR_WHITE;
				}
				if (e.key.keysym.sym == SDLK_3) {
					m->color = COLOR_RED;
				}
				if (e.key.keysym.sym == SDLK_4) {
					m->color = COLOR_GREEN;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (e.button.button == SDL_BUTTON_LEFT) {
					SDL_GetMouseState(&xMouse,&yMouse);
					if (xMouse < 32 && yMouse < 32) {
						m->color = COLOR_BLUE;
					} else if (xMouse < 64 && yMouse < 32) {
						m->color = COLOR_WHITE;
					} else if (xMouse < 96 && yMouse < 32) {
						m->color = COLOR_RED;
					} else if (xMouse < 128 && yMouse < 32) {
						m->color = COLOR_GREEN;
					} else {
						draw_pencil(m, e.button.x, e.button.y);
					}
					m->mouse_down = 1;
				}
				break;
			case SDL_MOUSEMOTION:
				if (m->mouse_down && e.button.button == SDL_BUTTON_LEFT) {
					SDL_GetMouseState(&xMouse,&yMouse);
					if (xMouse < 32 && yMouse < 32) {
						m->color = COLOR_BLUE;
					} else if (xMouse < 64 && yMouse < 32) {
						m->color = COLOR_WHITE;
					} else if (xMouse < 96 && yMouse < 32) {
						m->color = COLOR_RED;
					} else if (xMouse < 128 && yMouse < 32) {
						m->color = COLOR_GREEN;
					} else {
						draw_pencil(m, e.button.x, e.button.y);
					}
				}
				break;
			default:
				break;
			}
		}
		draw_render(m);
	}
	return 0;
}
