#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <signal.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define TOOL_PENCIL 0
#define COLOR_SIZE 32

#define COLOR_BLACK 0xFF000000
#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_RED   0xFFFF0000
#define COLOR_GREEN 0xFF00FF00
#define COLOR_BLUE  0xFF0000FF

#define PORT 8080
#define MAX_DELTA 0.001f

bool curr_clicked = false;

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

void click_sig(int sig) 
{
	curr_clicked = !curr_clicked;
	printf("curr_clicked changed to : %d\n", curr_clicked);
	signal(SIGUSR1, click_sig);
}

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
		draw_error("Failed to initialize state!\n");
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
	printf("STARTS ALL!!!!\n");
	int parentId = fork();
	if (parentId != 0) 
	{
		bool last_click = curr_clicked;
		printf("PARENT ALIVE!!!\n");
		int server_fd, new_socket, valread;
		struct sockaddr_in address;
		int opt = 1;
		int addrlen = sizeof(address);
		char buffer[1024] = {0};
		FILE *fptr;
	
		// Creating socket file descriptor
		if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
		{
			perror("socket failed");
			exit(EXIT_FAILURE);
		}
	
		// Forcefully attaching socket to the port 8080
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
		{
			perror("setsockopt");
			exit(EXIT_FAILURE);
		}
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons( PORT );
		
		// Forcefully attaching socket to the port 8080
		if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
		{
			perror("bind failed");
			exit(EXIT_FAILURE);
		}
		if (listen(server_fd, 3) < 0)
		{
			perror("listen");
			exit(EXIT_FAILURE);
		}
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}

 		fptr = fopen("../logs/locationLogs.txt","w");

		while(1) {
			valread = read( new_socket , buffer, 1024);
			printf("%s\n",buffer);
		
			//find values!!!
			int i = 0;
    			char *p = strtok (buffer, " ");
			char *cursurVals[12];
			while (p != NULL)
			{
				cursurVals[i++] = p;
				p = strtok (NULL, " ");
			}
		
			float deltaX = atof(cursurVals[0]);
			float deltaY = atof(cursurVals[1]);
			int clicked = atoi(cursurVals[2]);

			
			if(deltaX > MAX_DELTA)
				deltaX = MAX_DELTA;

			if(deltaY > MAX_DELTA)
				deltaY = MAX_DELTA;

			if(deltaX < - MAX_DELTA)
				deltaX = - MAX_DELTA;

			if(deltaY < - MAX_DELTA)
				deltaY = - MAX_DELTA;	

				

			if(deltaX != 0 && deltaY != 0) {
				float sensitivity = 4000;
				int moveX = (int) (deltaX*sensitivity);
				int moveY = (int) (deltaY*sensitivity);
				fprintf(fptr,"%d %d %d\n",moveX,moveY,clicked);

				// Send signal for click event
				if(last_click != clicked) {
					kill(parentId, SIGUSR1);
				}

				//Move The Cursor On Screen
				char * command = (char *) malloc(1024 * sizeof(char));
				sprintf(command, "xdotool mousemove_relative -- %d %d", moveX, moveY);
				printf("%s\n", command);
				int sys = system(command);
			}
		
		
			memset(buffer, 0, sizeof(buffer));
			//buffer[0] = "\0";
			last_click = clicked;
		}

	
		return 0;
	} // FINISH OF PARENT MOVE CURSOR PROCESS
	else
	{
		printf("CHILD ALIVE!!!\n");
		signal(SIGUSR1, click_sig);
		printf("SIGUSR1 has registered!\n");

		int xMouse, yMouse;
		int xMouseLast = 0, yMouseLast = 0; // For lines...
		draw_state* m = draw_init();

		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			draw_error("Failed to initialize SDL!\n");
		}

		m->win = SDL_CreateWindow("draw something", 0, 0, m->win_width, m->win_height, SDL_WINDOW_SHOWN);
		if (m->win == NULL) {
			draw_error("Failed to initialize SDL window!\n");
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
						if (xMouse < COLOR_SIZE && yMouse < COLOR_SIZE) {
							m->color = COLOR_BLUE;
						} else if (xMouse < (COLOR_SIZE * 2) && yMouse < COLOR_SIZE) {
							m->color = COLOR_WHITE;
						} else if (xMouse < (COLOR_SIZE * 3) && yMouse < COLOR_SIZE) {
							m->color = COLOR_RED;
						} else if (xMouse < (COLOR_SIZE * 4) && yMouse < COLOR_SIZE) {
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
			
			SDL_GetMouseState(&xMouse,&yMouse);
			if(curr_clicked) {
				if (xMouse < 32 && yMouse < 32) {
					m->color = COLOR_BLUE;
				} else if (xMouse < 64 && yMouse < 32) {
					m->color = COLOR_WHITE;
				} else if (xMouse < 96 && yMouse < 32) {
					m->color = COLOR_RED;
				} else if (xMouse < 128 && yMouse < 32) {
					m->color = COLOR_GREEN;
				} else {
					draw_pencil(m, xMouse, yMouse);
				}
			}
			
			// For lines
			xMouseLast = xMouse;
			yMouseLast = yMouse;

			// Render!
			draw_render(m);
		}

		return 0;
	} // FINISH OF CHILD DRAW PANEL PROCESS
}
