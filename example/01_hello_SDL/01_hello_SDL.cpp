/*This source code copyrighted by Lazy Foo' Productions (2004-2020)
and may not be redistributed without written permission.*/
#if 0
//Using SDL and standard IO
#include <SDL2/SDL.h>
#include <stdio.h>

//Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 480;

int main( int argc, char* args[] )
{
	//The window we'll be rendering to
	SDL_Window* window = NULL;
	
	//The surface contained by the window
	SDL_Surface* screenSurface = NULL;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
	}
	else
	{
		//Create window
		window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( window == NULL )
		{
			printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
		}
		else
		{
			//Get window surface
			screenSurface = SDL_GetWindowSurface( window );

			//Fill the surface white
			SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0xFF, 0xFF, 0xFF ) );
			
			//Update the surface
			SDL_UpdateWindowSurface( window );

			//Wait two seconds
			SDL_Delay( 2000 );
		}
	}

	//Destroy window
	SDL_DestroyWindow( window );

	//Quit SDL subsystems
	SDL_Quit();

	return 0;
}
#endif

#include <SDL2/SDL.h>
#include <stdio.h>

int main(void)
{
	SDL_Window * window = NULL;
	SDL_Surface * Surface = NULL;

	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		printf("SDL can not init  SDL_Error: %s\n", SDL_GetError());
	}else{
		window = SDL_CreateWindow("My first SDL exam", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
		if(window == NULL){
			printf("SDL can not create window SDL_GetError %s\n", SDL_GetError());
		}else{
			Surface = SDL_GetWindowSurface(window);

			SDL_FillRect(Surface, NULL, SDL_MapRGB(Surface->format, 0x00, 0xFF, 0xFF));

			SDL_UpdateWindowSurface(window);

			SDL_Delay(3000);
		}
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}