
#include <SDL2/SDL.h>
#include <stdio.h>

bool init();
bool loadMedia();
void close();

SDL_Window *gWindow = NULL;
SDL_Surface *gSurface = NULL;
SDL_Surface *gHelloWorld = NULL;

bool init()
{
    bool success = true;

    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("faild init sdl %s\n", SDL_GetError());
        success = false;
    }else{
        gWindow = SDL_CreateWindow("02 SDL", 
                                    SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED,
                                    800,
                                    600,
                                    SDL_WINDOW_SHOWN
                                    );
        if(gWindow == NULL){
            printf("failed create window %s\n", SDL_GetError());
            success = false;
        }else{
            gSurface = SDL_GetWindowSurface(gWindow);
        }
    }

    return success;
}

bool loadMedia()
{
    bool success = true;

    gHelloWorld = SDL_LoadBMP("hello_world.bmp");
    if(gHelloWorld == NULL){
        printf("faild to load image %s\n", SDL_GetError());
        success = false;
    }

    return success;
}

void close()
{
    SDL_FreeSurface(gHelloWorld);
    gHelloWorld = NULL;
    
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    SDL_Quit();
}

int main(void)
{
    if(!init())
        printf("Failed to initialize!\n");
    else{
        if(!loadMedia())
            printf("Failed to load media!\n");
        else{
            //display
            SDL_BlitSurface(gHelloWorld, NULL, gSurface, NULL);

            SDL_UpdateWindowSurface(gWindow);

            SDL_Delay(3000);

        }
    }

    close();

    return 0;
}
