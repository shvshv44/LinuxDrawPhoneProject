#include <iostream>
#include <SDL2/SDL.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define PORT 8080
#define WAIT_TIME 50; //TODO: USE FOR DELAYS

int msleep(long msec)
{
	struct timespec ts;
	int res;

	if (msec < 0)
	{
		errno = EINVAL;
		return -1;
	}

	ts.tv_sec = msec / 1000;
	ts.tv_nsec = (msec % 1000) * 1000000;

	do
	{
		res = nanosleep(&ts, &ts);
	} while (res && errno == EINTR);

	return res;
}

int main(int argc, char *args[])
{
	//Socket
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char *hello = "Hello from client";
	char buffer[1024] = {0};

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");

		return -1;
	}

	serv_addr.sin_family = AF_INET;

	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary form

	if (inet_pton(AF_INET, "192.168.1.23", &serv_addr.sin_addr) <= 0)

	{
		printf("\nInvalid address/ Address not supported \n");

		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)

	{
		printf("\nConnection Failed \n");

		return -1;
	}

	send(sock, hello, strlen(hello), 0);

	//Window
	float prevX = 0, prevY = 0;
	float prevMouseX = 0, prevMouseY = 0;
	SDL_Log("TRY SHAQ \n");

	if (SDL_Init(SDL_INIT_VIDEO) == 0)
	{
		SDL_Window *window = NULL;
		SDL_Renderer *renderer = NULL;

		SDL_DisplayMode DM;
		SDL_GetCurrentDisplayMode(0, &DM);
		int screenW = DM.w;
		int screenH = DM.h;
		int screenSep = DM.h - DM.h / 4;
		bool isClicked = false;

		if (SDL_CreateWindowAndRenderer(screenW, screenH, 0, &window, &renderer) == 0)
		{
			SDL_bool done = SDL_FALSE;

			while (!done)
			{
				SDL_Event event;

				//SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
				//SDL_RenderClear(renderer);

				SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
				SDL_RenderDrawLine(renderer, 0, screenSep, screenW, screenSep);
				SDL_RenderPresent(renderer);

				SDL_Rect r;
				r.x = 0;
				r.y = screenSep + 1;
				r.w = screenW;
				r.h = screenH - screenSep - 1;
				if (!isClicked)
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				else
					SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
				SDL_RenderFillRect(renderer, &r);
				SDL_RenderPresent(renderer);


				while (SDL_PollEvent(&event))
				{
					int mouseX, mouseY;
					SDL_GetMouseState(&mouseX, &mouseY);

					if (event.type == SDL_QUIT)
					{
						done = SDL_TRUE;
					}

					switch (event.type)
					{
					case SDL_FINGERMOTION:
					{
						if (mouseY < screenSep)
						{
							float tmpX = event.tfinger.x;
							float tmpY = event.tfinger.y;
							float deltaX = tmpX - prevX;
							float deltaY = tmpY - prevY;

							char *curtxt = (char *)malloc(1024 * sizeof(char));
							sprintf(curtxt, "%f %f %d ", deltaX, deltaY, (isClicked)? 1: 0);
							send(sock, curtxt, strlen(curtxt), 0);

							prevX = tmpX;
							prevY = tmpY;
							prevMouseX = mouseX;
							prevMouseY = mouseY;
						}

						break;
					}
					
					case SDL_MOUSEBUTTONDOWN: {
						if(mouseY > screenSep && event.button.button == SDL_BUTTON_LEFT) {
							isClicked = !isClicked;	
							}
							break;
					}
					
					default:
						break;
					}
				}
			}
		}

		if (renderer)
		{
			SDL_DestroyRenderer(renderer);
		}
		if (window)
		{
			SDL_DestroyWindow(window);
		}
	}
	SDL_Quit();
	return 0;
}
