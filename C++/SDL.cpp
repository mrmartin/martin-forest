#include "SDL.h"
#include <string>
 
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
 
SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_Texture* LoadImage(std::string file){
    SDL_Surface *loadedImage = nullptr;
    SDL_Texture *texture = nullptr;
  
    loadedImage = SDL_LoadBMP(file.c_str());
    if (loadedImage != nullptr){
        texture = SDL_CreateTextureFromSurface(renderer, loadedImage);
        SDL_FreeSurface(loadedImage);
    }
    else
        std::cout << SDL_GetError() << std::endl;
    return texture;
}

void ApplySurface(int x, int y, SDL_Texture *tex, SDL_Renderer *rend){
    SDL_Rect pos;
    pos.x = x;
    pos.y = y;
    SDL_QueryTexture(tex, NULL, NULL, &pos.w, &pos.h);
 
    SDL_RenderCopy(rend, tex, NULL, &pos);
}

int main(int argc, char** argv){
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1){
        std::cout << SDL_GetError() << std::endl;
        return 1;
    }
 
    window = SDL_CreateWindow("Lesson 2", SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr){
        std::cout << SDL_GetError() << std::endl;
        return 2;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED 
        | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr){
        std::cout << SDL_GetError() << std::endl;
        return 3;
    }
  SDL_Texture *background = nullptr, *image = nullptr;
    background = LoadImage("Lesson2res/background.bmp");
    image = LoadImage("Lesson2res/image.bmp");
    if (background == nullptr || image == nullptr)
        return 4;

  SDL_RenderClear(renderer);
 
    int bW, bH;
    SDL_QueryTexture(background, NULL, NULL, &bW, &bH);
    ApplySurface(0, 0, background, renderer);
    ApplySurface(bW, 0, background, renderer);
    ApplySurface(0, bH, background, renderer);
    ApplySurface(bW, bH, background, renderer);
 int iW, iH;
    SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
    int x = SCREEN_WIDTH / 2 - iW / 2;
    int y = SCREEN_HEIGHT / 2 - iH / 2;
    ApplySurface(x, y, image, renderer);
  SDL_RenderPresent(renderer);
    SDL_Delay(2000);
 SDL_DestroyTexture(background);
    SDL_DestroyTexture(image);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
 
    SDL_Quit();
  
    return 0;
}

