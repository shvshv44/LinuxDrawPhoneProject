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
} mage_state;

void mage_exit(int code)
{
	exit(code);
}

void mage_error(const char* str)
{
	printf("ERROR: %s", str);
	mage_exit(1);
}

void mage_pencil(mage_state* m, int x, int y)
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

void mage_render(mage_state* m)
{
	SDL_UpdateWindowSurface(m->win);
}

mage_state* mage_init(void)
{
	mage_state* m = malloc(sizeof(*m));
	if (m == NULL) {
		mage_error("Failed to initialize state!");
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
	mage_state* m = mage_init();

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		mage_error("Failed to initialize SDL!");
	}

	m->win = SDL_CreateWindow("draw somethin", 0, 0, m->win_width, m->win_height, SDL_WINDOW_SHOWN);
	if (m->win == NULL) {
		mage_error("Failed to initialize SDL window!");
	}

	m->screen = SDL_GetWindowSurface(m->win);
	m->color = COLOR_WHITE;


	/* so we don't start with a black m->screen */
	mage_render(m);
	while (1) {
		SDL_Event e;
		
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				mage_exit(0);
				break;
			case SDL_KEYDOWN:
				if (e.key.keysym.sym == SDLK_q || e.key.keysym.sym == SDLK_ESCAPE) {
					mage_exit(0);
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
					mage_pencil(m, e.button.x, e.button.y);
					m->mouse_down = 1;
				}
				break;
			case SDL_MOUSEMOTION:
				if (m->mouse_down && e.button.button == SDL_BUTTON_LEFT) {
					mage_pencil(m, e.button.x, e.button.y);
				}
				break;
			default:
				break;
			}
		}
		mage_render(m);
	}
	return 0;
}
