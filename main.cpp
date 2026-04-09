#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <iostream>
#include "Framework.h"

#include "Tetris.h"
//#include "BreakOut.h"
//#include "BlockPuzzle.h"
//#include "Snake.h"
//#include "Pacman.h"
//#include "Space_invader.h"
//#include "Galaga.h"
//#include "Climber.h"
//#include "Micro-Craft.h"

//#include "MainMenu.h"
//#include "SimpleBoxGame.h"

using namespace Framework;
using namespace std;

int main(int argc, char* argv[]) {
    SysPuntaje::cargar();
    Engine engine(640, 380);
    if (!engine.init("Colección de Juegos SDL2")){
    	return 1;
    }

    engine.resources.loadFont("main", "assets/Corporation_Games.ttf", 24);

    engine.sceneStack.push(new Tetris_Game(&engine.resources, &engine));
    engine.run();
    engine.cleanup();
    return 0;
}
