#include "SDL.h"
#include "SDL_rotozoom.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define MAIN_MENU_STATE 1
#define GAME_STATE 2
#define LEVEL_SELECT_STATE 3
#define TOTAL_VICTORY_STATE 4
#define ENTER_HIGH_SCORE_STATE 5
#define VIEW_HIGH_SCORES_STATE 6
#define CREDITS_STATE 7

using namespace std;

int MOVETICKS = 150;
int CHANGEDELAY = 100;

string GAMEFONT = "bgothm.ttf";

SDL_Surface *screen = NULL;

SDL_Surface *bg = NULL;
SDL_Surface *dragon = NULL;

SDL_Surface *blocks = NULL;
SDL_Surface *border = NULL;

SDL_Surface *whiteLeft = NULL;
SDL_Surface *whiteRight[16];
SDL_Surface *whiteStart = NULL;

SDL_Surface *blackLeft = NULL;
SDL_Surface *blackRight[16];
SDL_Surface *blackStart = NULL;
SDL_Surface *complete = NULL;
SDL_Surface *complete2 = NULL;

SDL_Surface *controls = NULL;

Mix_Music *gameMusic = NULL;
Mix_Music *menuMusic = NULL;
Mix_Chunk *switchSFX = NULL;
Mix_Chunk *switch2SFX = NULL;
Mix_Chunk *completeSFX = NULL;
Mix_Chunk *complete2SFX = NULL;
Mix_Chunk *complete3SFX = NULL;

SDL_Color red = { 255, 0, 0 };
SDL_Color white = { 255, 255, 255 };
SDL_Color black = { 0, 0, 0 };
SDL_Color yellow = { 244, 233, 0 };
SDL_Color victoryColor = { 255, 0, 0 };

bool rightPushed = false;
bool downPushed = false;
bool upPushed = false;
bool leftPushed = false;

int changeDelay = 0;

int animFrame = 0;
int ANIM_FRAME_LENGTH = 300;
int animCounter = 0;

SDL_Event event;

int gameState = MAIN_MENU_STATE;

struct NameScore {
	string name;
	int score;
};

NameScore hscores[10];
string initials = "AAA";
int charSelected = 0;

struct Coords {
	int x;
	int y;
};

struct Map {
	int b[25][25];
	int colors[5][5];
	int width;
	int height;
};

Map map;

Coords pos[2];
Coords pixelPos[2];
Coords start[2];
int active = 0;

Uint32 lastTick;
Uint32 curTick;
int timeDif;
bool moving[2];

int moveTicks[2];

Coords offset;

int curRotate[2];
int tickRotate[2];

Coords swapDest;
bool swapMode = false;

Uint32 FPS = 0;
Uint32 FPS_count = 0;

int level = 1;
int score = 0;
int bonus = 0;
int swaps;

bool graphics = true;

string sideMessage[2];

int menuOptionSelected = 0;

int menuOptionChange = 50;
int MENU_CHANGE_TICKS = 50;

string displayedMessage = "";
int displayedLevel = 0;
int displayedScore = -1;

int levelSelect = 1;
int NUMLEVELS = 17;

void DrawScreen();
void Update();
int LoadImages(string tileSet, string bgName);
void DrawImage(int x, int y, SDL_Surface* source );
void DrawImage(int x, int y, SDL_Surface* source, SDL_Surface* dest );
void DrawImage(int x, int y, SDL_Surface* source, int x2, int y2, int w, int h );
void DrawText(int x, int y, string text, string font, int size, SDL_Color color);
string itos(int val);
void FreeGraphics();
int LoadLevel();
void saveScore(string initials, int score);
int isHighScore(int score);

int main( int argc, char* args[] )
{
    //Start SDL
    SDL_Init( SDL_INIT_EVERYTHING );
    
	screen = SDL_SetVideoMode( 1024, 768, 32, SDL_SWSURFACE );
	SDL_WM_SetCaption( "Super Zen Dragon", NULL );

	if( TTF_Init() == -1 )
    {
		system("pause");
        return 1;    
    }

	if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 )
    {
		system("pause");
        return 1;    
    }

	SDL_Surface *temp = NULL;
	temp = SDL_LoadBMP( "gfx\\titledragon.bmp");
	if (temp == NULL) return 2;
	dragon = SDL_DisplayFormat( temp );
	if (dragon == NULL) return 3;
	SDL_FreeSurface(temp);

    gameMusic = Mix_LoadMUS( "sound\\music1.wav" );
	menuMusic = Mix_LoadMUS( "sound\\music2.wav" );

	switchSFX = Mix_LoadWAV( "sound\\switch.wav" );
	if (switchSFX == NULL) return 4;
	switch2SFX = Mix_LoadWAV( "sound\\switch2.wav" );
	if (switch2SFX == NULL) return 5;
	completeSFX = Mix_LoadWAV( "sound\\complete1.wav" );
	if (completeSFX == NULL) return 6;
	complete2SFX = Mix_LoadWAV( "sound\\complete2.wav" );
	if (complete2SFX == NULL) return 7;
	complete3SFX = Mix_LoadWAV( "sound\\complete3.wav" );
	if (complete3SFX == NULL) return 8;

	if (LoadLevel() == 1) { printf("Couldn't load level"); system("pause"); return 1; }

	bool running = true;

	lastTick = SDL_GetTicks();

	Mix_PlayMusic( menuMusic, -1 );

	while (running) {

		lastTick = curTick;
		curTick = SDL_GetTicks();

		timeDif = curTick - lastTick;

		FPS++;
		FPS_count += timeDif;
		if (FPS_count > 1000) {
			FPS_count -= 1000;
			cout << FPS << endl;
			FPS = 0;
		}

		animCounter+=timeDif;
		if (animCounter > ANIM_FRAME_LENGTH) {
			animCounter -= ANIM_FRAME_LENGTH;
			animFrame++;
			if (animFrame > 2) animFrame = 0;
		}

		DrawScreen();

		Update();

		while( SDL_PollEvent( &event ) ) {
			
			if( event.type == SDL_QUIT )
			{
				running = false;
			}    

			if ( event.type == SDL_KEYUP) {
				switch( event.key.keysym.sym ) {
					case SDLK_UP: 
						upPushed = false;
						break;
					case SDLK_DOWN:
						downPushed = false;
						break;
					case SDLK_LEFT:
						leftPushed = false;
						break;
					case SDLK_RIGHT:
						rightPushed = false;
						break;
				}
			}

			if( event.type == SDL_KEYDOWN) {
            
				switch( event.key.keysym.sym ) {
					case SDLK_UP: 
						upPushed = true;
						break;
					case SDLK_DOWN:
						downPushed = true;
						break;
					case SDLK_LEFT:
						leftPushed = true;
						break;
					case SDLK_RIGHT:
						rightPushed = true;
						break;
					case SDLK_SPACE:
						swapMode = !swapMode;

						if (swapMode) {
							int color = map.colors[pos[active].x / 5][pos[active].y / 5];
							if(color == 1)
								swapMode = false;
							bool found = false;
							for (int y = 0; y < map.height/5; y++) {
								for (int x = 0; x < map.width/5; x++) {
									if (!(x == pos[active].x/5 && y == pos[active].y/5) && map.colors[x][y] == color) {
										found = true;
										swapDest.x = x;
										swapDest.y = y;
										break;
									}
								}
								if (found) break;
							}
							if (!found) swapMode = false;
						}
						break;
					case SDLK_RETURN:
						if (gameState == MAIN_MENU_STATE) {
							if (menuOptionSelected == 3) running = false;
							else if (menuOptionSelected == 0) {
								level = 1;
								LoadLevel();
								gameState = GAME_STATE;
								score = 0;
								Mix_PauseMusic();
								Mix_PlayMusic( gameMusic, -1 );
								SDL_FillRect(screen, NULL, 0);
							} else if (menuOptionSelected == 1) {
								gameState = LEVEL_SELECT_STATE;
								menuOptionSelected = 0;
								SDL_FillRect(screen, NULL, 0);
							} else {
								gameState = VIEW_HIGH_SCORES_STATE;
								isHighScore(0);
								SDL_FillRect(screen, NULL, 0);
							}
						} else if (gameState == LEVEL_SELECT_STATE) {
							if (menuOptionSelected == 0) {
								level = levelSelect;
								LoadLevel();
								gameState = GAME_STATE;
								Mix_PauseMusic();
								Mix_PlayMusic( gameMusic, -1 );
								SDL_FillRect(screen, NULL, 0);
							} else {
								gameState = MAIN_MENU_STATE;
								menuOptionSelected = 1;
								SDL_FillRect(screen, NULL, 0);
							}
						} else if (gameState == TOTAL_VICTORY_STATE) {

							int s = isHighScore(score);
							if (s == -1) {
								gameState = VIEW_HIGH_SCORES_STATE;
							} else {
								gameState = ENTER_HIGH_SCORE_STATE;
							}

							SDL_FillRect(screen, NULL, 0);
						} else if (gameState == CREDITS_STATE) {
							gameState = MAIN_MENU_STATE;
							Mix_PlayMusic( menuMusic, -1);
							graphics = false;
							score = 0;
							SDL_FillRect(screen, NULL, 0);
						} else if (gameState == VIEW_HIGH_SCORES_STATE) {
							gameState = CREDITS_STATE;

							SDL_FillRect(screen, NULL, 0);
						} else if (gameState == ENTER_HIGH_SCORE_STATE) {
							gameState = VIEW_HIGH_SCORES_STATE;

							saveScore(initials, score);
							SDL_FillRect(screen, NULL, 0);
						}


						if (swapMode) {
							Coords swapSrc;
							swapSrc.x = pos[active].x/5;
							swapSrc.y = pos[active].y/5;

							if( Mix_PlayChannel( -1, switchSFX, 0 ) == -1 ) return 1;

							for (int y = 0; y < 5; y++) {
								for (int x = 0; x < 5; x++) {
									int temp = map.b[swapDest.x * 5 + x][swapDest.y * 5 + y];
									map.b[swapDest.x * 5 + x][swapDest.y * 5 + y] = map.b[swapSrc.x * 5 + x][swapSrc.y * 5 + y];
									map.b[swapSrc.x * 5 + x][swapSrc.y * 5 + y] = temp;
								}
							}

							pos[active].x = (pos[active].x - swapSrc.x*5) + swapDest.x * 5;
							pos[active].y = (pos[active].y - swapSrc.y*5) + swapDest.y * 5;

							if (start[0].x / 5 == swapSrc.x && start[0].y / 5 == swapSrc.y) {
								start[0].x = (start[0].x - swapSrc.x*5) + swapDest.x * 5;
								start[0].y = (start[0].y - swapSrc.y*5) + swapDest.y * 5;
							} else if (start[0].x / 5 == swapDest.x && start[0].y / 5 == swapDest.y) {
								start[0].x = (start[0].x - swapDest.x*5) + swapSrc.x * 5;
								start[0].y = (start[0].y - swapDest.y*5) + swapSrc.y * 5;
							}

							if (start[1].x / 5 == swapSrc.x && start[1].y / 5 == swapSrc.y) {
								start[1].x = (start[1].x - swapSrc.x*5) + swapDest.x * 5;
								start[1].y = (start[1].y - swapSrc.y*5) + swapDest.y * 5;
							} else if (start[1].x / 5 == swapDest.x && start[1].y / 5 == swapDest.y) {
								start[1].x = (start[1].x - swapDest.x*5) + swapSrc.x * 5;
								start[1].y = (start[1].y - swapDest.y*5) + swapSrc.y * 5;
							}

							int inactive = (active == 1) ? 0 : 1;

							if (pos[inactive].x/5 == swapSrc.x && pos[inactive].y/5 == swapSrc.y) {
								pos[inactive].x = (pos[inactive].x - swapSrc.x*5) + swapDest.x * 5;
								pos[inactive].y = (pos[inactive].y - swapSrc.y*5) + swapDest.y * 5;
							} else if (pos[inactive].x/5 == swapDest.x && pos[inactive].y/5 == swapDest.y) {
								pos[inactive].x = (pos[inactive].x - swapDest.x*5) + swapSrc.x * 5;
								pos[inactive].y = (pos[inactive].y - swapDest.y*5) + swapSrc.y * 5;
							}
							swapDest = swapSrc;
							swapMode = false;

							pixelPos[0].x = pos[0].x*30;
							pixelPos[0].y = pos[0].y*30;
							pixelPos[1].x = pos[1].x*30;
							pixelPos[1].y = pos[1].y*30;

							swaps++;

						}
						break;
					case SDLK_TAB:
						if(!swapMode && !moving[0] && !moving[1]) {
							active = (active == 0) ? 1 : 0;
							Mix_PlayChannel( -1, switch2SFX, 0 );
						}
						break;

					case SDLK_r:
						LoadLevel();
						break;

					case SDLK_ESCAPE: 
						if (gameState == MAIN_MENU_STATE)
							running = false;
						else if (gameState == GAME_STATE) {
							gameState = MAIN_MENU_STATE;
							Mix_PauseMusic();
							Mix_PlayMusic( menuMusic, -1 );
							displayedMessage = "";
							displayedScore = -1;
						} else if (gameState == LEVEL_SELECT_STATE) {
							gameState = MAIN_MENU_STATE;
							menuOptionSelected = 1;
							SDL_FillRect(screen, NULL, 0);
						} else if (gameState == TOTAL_VICTORY_STATE) {
							gameState = CREDITS_STATE;
							SDL_FillRect(screen, NULL, 0);
						} else if (gameState == CREDITS_STATE) {
							gameState = MAIN_MENU_STATE;
							Mix_PlayMusic( menuMusic, -1);
							graphics = false;
							score = 0;
							SDL_FillRect(screen, NULL, 0);
						}
				}
			}

		}

		//SDL_Delay((d > 0) ? d : 0);

	}

    //Quit SDL
	FreeGraphics();
    
	Mix_FreeChunk( switchSFX );
	Mix_FreeChunk( switch2SFX );
	Mix_FreeChunk( completeSFX );
	Mix_FreeChunk( complete2SFX );
	Mix_FreeChunk( complete3SFX );
    Mix_FreeMusic( gameMusic );
	Mix_FreeMusic( menuMusic );

	Mix_CloseAudio();

	SDL_FreeSurface(dragon);

    //Quit SDL_ttf
    TTF_Quit();

    SDL_Quit();
    
    return 0;    
}

void FreeGraphics() {

	if (!graphics) return;

	SDL_FreeSurface(blocks);
	SDL_FreeSurface(border);

	SDL_FreeSurface(bg);

	SDL_FreeSurface(whiteLeft);

	for (int i = 0; i < 16; i++) {
		SDL_FreeSurface(whiteRight[i]);
		SDL_FreeSurface(blackRight[i]);
	}

	SDL_FreeSurface(whiteStart);
	SDL_FreeSurface(blackLeft);
	SDL_FreeSurface(blackStart);
	SDL_FreeSurface(complete);
	SDL_FreeSurface(complete2);

	SDL_FreeSurface(controls);
}

int LoadLevel() {
	//Load map
	string m = "levels\\level";
	m += itos(level);
	m += ".txt";
	ifstream fin(m.c_str());

	if (fin.fail()) {

		Mix_PauseMusic();
		Mix_PlayChannel( -1, complete3SFX, 0 );
		gameState = TOTAL_VICTORY_STATE;
		graphics = false;

		printf("Couldn't open level file");
		return 0;
	}

	string tileSet;

	fin >> tileSet;
	tileSet = "gfx\\" + tileSet;

	string bgName;

	fin >> bgName;
	bgName = "gfx\\" + bgName;

	bonus = 1000 * level;
	swaps = 1;

	//Load images
	if (LoadImages(tileSet, bgName) == 1) { printf(tileSet.c_str()); return 1; }

	fin >> map.width;
	fin >> map.height;

	offset.x = (768 - map.width*30) / 2;
	offset.y = (768 - map.height*30) / 2;

	for (int y = 0; y < map.height; y++) {
		for (int x = 0; x < map.width; x++) {
			fin >> map.b[x][y];
		}
	}

	for (int y = 0; y < map.height/5; y++) {
		for (int x = 0; x < map.width/5; x++) {
			fin >> map.colors[x][y];
		}
	}

	fin >> pos[0].x;
	fin >> pos[0].y;
	fin >> pos[1].x;
	fin >> pos[1].y;

	sideMessage[0] = "";
	sideMessage[1] = "";

	while (!fin.eof()) getline(fin, sideMessage[0]);

	int f = sideMessage[0].find("~");
	if (f != string::npos) {
		sideMessage[1] = sideMessage[0].substr(f+1, sideMessage[0].length() - f - 1);
		sideMessage[0] = sideMessage[0].substr(0, f);
	}

	pixelPos[0].x = pos[0].x *30;
	pixelPos[0].y = pos[0].y * 30;
	pixelPos[1].x = pos[1].x * 30;
	pixelPos[1].y = pos[1].y * 30;
 
	moving[0] = false;
	moving[1] = false;

	start[0].x = pos[0].x;
	start[0].y = pos[0].y;
	start[1].x = pos[1].x;
	start[1].y = pos[1].y;

	curRotate[0] = 0;
	curRotate[1] = 0;
	tickRotate[0] = 0;
	tickRotate[1] = 0;

	graphics = true;

	curTick = SDL_GetTicks();

	displayedMessage = "---";

	return 0;
}

void Update() {

	switch (gameState) {
		case MAIN_MENU_STATE:
			if (menuOptionChange < MENU_CHANGE_TICKS) {
				menuOptionChange += timeDif;
				return;
			}

			menuOptionChange = 0;
			
			if (upPushed) {
				menuOptionSelected = (--menuOptionSelected < 0) ? 0 : menuOptionSelected;
			}
			if (downPushed) {
				menuOptionSelected = (++menuOptionSelected > 3) ? 3 : menuOptionSelected;
			}

			return;
			break;

		case LEVEL_SELECT_STATE:
			if (menuOptionChange < MENU_CHANGE_TICKS) {
				menuOptionChange += timeDif;
				return;
			}

			menuOptionChange = 0;

			if (leftPushed) {
				menuOptionSelected = (--menuOptionSelected < 0) ? 0 : menuOptionSelected;
			}
			if (rightPushed) {
				menuOptionSelected = (++menuOptionSelected > 1) ? 1 : menuOptionSelected;
			}
			if (upPushed) {
				levelSelect = (++levelSelect > NUMLEVELS) ? NUMLEVELS : levelSelect;
			}
			if (downPushed) {
				levelSelect = (--levelSelect < 1) ? 1 : levelSelect;
			}

			return;
			break;

		case ENTER_HIGH_SCORE_STATE:
			if (menuOptionChange < MENU_CHANGE_TICKS) {
				menuOptionChange += timeDif;
				return;
			}

			menuOptionChange = 0;

			if (leftPushed) {
				charSelected = (--charSelected < 0) ? 0 : charSelected;
			}
			if (rightPushed) {
				charSelected = (++charSelected > 2) ? 2 : charSelected;
			}

			if (upPushed) {
				string s = initials.substr(charSelected, 1);
				string abc = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

				int i = abc.find(s);

				i--;
				if (i < 0) i = 25;

				s = abc.substr(i,1);

				initials = initials.substr(0, charSelected) + s + initials.substr(charSelected+1, 2-charSelected);
			}

			if (downPushed) {
				string s = initials.substr(charSelected, 1);
				string abc = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

				int i = abc.find(s);

				i++;
				if (i > 25) i = 0;

				s = abc.substr(i,1);

				initials = initials.substr(0, charSelected) + s + initials.substr(charSelected+1, 2-charSelected);
			}

				

		case GAME_STATE:
			break;
	}

	if (!moving[0] && !moving[1] && pos[0].x == start[1].x && pos[0].y == start[1].y && pos[1].x == start[0].x && pos[1].y == start[0].y) {
		//VICTORY
		FreeGraphics();

		Mix_PlayChannel( -1, complete3SFX, 0 );
		DrawText(map.width * 30 / 2 + offset.x, map.height* 30 / 2 + offset.y, "Victory!", GAMEFONT, 90, victoryColor);

		SDL_Flip(screen);

		SDL_Delay(2000);
		SDL_FillRect(screen, NULL, 0);

		score += bonus;

		level++;
		pos[0].x = -1;
		LoadLevel();
	}

	if (moving[active]) {

		moveTicks[active] += timeDif;

		int remainder = 0;
		if (moveTicks[active] > MOVETICKS) {
			remainder = moveTicks[active] - MOVETICKS;
			moveTicks[active] = MOVETICKS;
		}

		if (pixelPos[active].x < pos[active].x*30) {
			pixelPos[active].x = (pos[active].x-1)*30 + (30 * moveTicks[active] / MOVETICKS);
		}
		if (pixelPos[active].x > pos[active].x*30) {
			pixelPos[active].x = (pos[active].x+1)*30 - (30 * moveTicks[active] / MOVETICKS);
		}
		if (pixelPos[active].y < pos[active].y*30) {
			pixelPos[active].y = (pos[active].y-1)*30 + (30 * moveTicks[active] / MOVETICKS);
		}
		if (pixelPos[active].y > pos[active].y*30) {
			pixelPos[active].y = (pos[active].y+1)*30 - (30 * moveTicks[active] / MOVETICKS);
		}
		if (pixelPos[active].x == pos[active].x *30 && pixelPos[active].y == pos[active].y * 30) {
			moving[active] = false;
			moveTicks[active] = remainder;

			int inactive = (active == 1) ? 0 : 1;
			if (pos[active].x == start[inactive].x && pos[active].y == start[inactive].y &&
				!(pos[inactive].x == start[active].x && pos[inactive].y == start[active].y))
				Mix_PlayChannel( -1, completeSFX, 0 );
		}

		return;
	}

	if (swapMode && changeDelay > 0) {
		changeDelay -= timeDif;
		return;
	}

	if (upPushed) {
		if(!swapMode) {
			pos[active].y--;
			if (pos[active].y < 0 || map.b[pos[active].x][pos[active].y] ||
				(pos[0].x == pos[1].x && pos[0].y == pos[1].y)) {
					pos[active].y++;
			} else {
				bonus -= swaps;
				if (bonus < 0) bonus = 0;
			}
			moving[active] = true;
		} else {
			changeDelay = CHANGEDELAY;
			int x = swapDest.x-1;
			int y = swapDest.y;
			int color = map.colors[swapDest.x][swapDest.y];

			bool found = false;

			while (!found) {
				while (y >= 0) {
					while (x >= 0) {
						if (map.colors[x][y] == color && !(x == pos[active].x/5 && y == pos[active].y/5)) {
							found = true;
							swapDest.x = x;
							swapDest.y = y;
						}
						if (found) break;
						x--;
					}
					if (found) break;
					y--;
					x = map.width/5-1;
					if (y < 0) y = map.height/5 - 1;
				}
			}

		}
	}

	if (downPushed) {
		if(!swapMode) {
			pos[active].y++;
			moving[active] = true;
			if (pos[active].y >= map.height || map.b[pos[active].x][pos[active].y] ||
				(pos[0].x == pos[1].x && pos[0].y == pos[1].y)) {
					pos[active].y--;
			} else {
				bonus -= swaps;
				if (bonus < 0) bonus = 0;
			}
			
		} else {
			changeDelay = CHANGEDELAY;
			int x = swapDest.x+1;
			int y = swapDest.y;
			int color = map.colors[swapDest.x][swapDest.y];

			bool found = false;

			while (!found) {
				while (y < map.height/5) {
					while (x < map.width/5) {
						if (map.colors[x][y] == color && !(x == pos[active].x/5 && y == pos[active].y/5)) {
							found = true;
							swapDest.x = x;
							swapDest.y = y;
						}
						if (found) break;
						x++;
					}
					if (found) break;
					y++;
					x = 0;
					if (y >= map.height/5) y = 0;
				}
			}
		}
	}

	if (leftPushed) {
		if(!swapMode) {
			pos[active].x--;
			if (pos[active].x < 0 || map.b[pos[active].x][pos[active].y] ||
				(pos[0].x == pos[1].x && pos[0].y == pos[1].y)) {
					pos[active].x++;
			} else {
				bonus -= swaps;
				if (bonus < 0) bonus = 0;
			}
			moving[active] = true;
		} else {
			changeDelay = CHANGEDELAY;
			int x = swapDest.x-1;
			int y = swapDest.y;
			int color = map.colors[swapDest.x][swapDest.y];

			bool found = false;

			while (!found) {
				while (y >= 0) {
					while (x >= 0) {
						if (map.colors[x][y] == color && !(x == pos[active].x/5 && y == pos[active].y/5)) {
							found = true;
							swapDest.x = x;
							swapDest.y = y;
						}
						if (found) break;
						x--;
					}
					if (found) break;
					y--;
					x = map.width/5-1;
					if (y < 0) y = map.height/5 - 1;
				}
			}

		}
	}

	if (rightPushed) {
		if(!swapMode) {
			pos[active].x++;
			moving[active] = true;
			if (pos[active].x >= map.width || map.b[pos[active].x][pos[active].y] ||
				(pos[0].x == pos[1].x && pos[0].y == pos[1].y)) {
					pos[active].x--;
			} else {
				bonus -= swaps;
				if (bonus < 0) bonus = 0;
			}
			
		} else {
			changeDelay = CHANGEDELAY;
			int x = swapDest.x+1;
			int y = swapDest.y;
			int color = map.colors[swapDest.x][swapDest.y];

			bool found = false;

			while (!found) {
				while (y < map.height/5) {
					while (x < map.width/5) {
						if (map.colors[x][y] == color && !(x == pos[active].x/5 && y == pos[active].y/5)) {
							found = true;
							swapDest.x = x;
							swapDest.y = y;
						}
						if (found) break;
						x++;
					}
					if (found) break;
					y++;
					x = 0;
					if (y >= map.height/5) y = 0;
				}
			}
		}
	}
}

void DrawScreen() {
	SDL_Rect r;
	r.x = 0;
	r.y = 0;
	r.w = 768;
	r.h = map.height*30+offset.y;
	SDL_FillRect(screen, &r, 0);

	switch (gameState) {
		case MAIN_MENU_STATE:

			r.w = 1024;
			r.h = 768;
			SDL_FillRect(screen, &r, 0);

			DrawImage(1024-357, 768-457, dragon);

			DrawText(1024/2, 250, "Super Zen Dragon", GAMEFONT, 95, white);
			if (menuOptionSelected == 0) {
				DrawText(350, 450, "Start Game", GAMEFONT, 60, red);
				DrawText(350, 520, "Level Select", GAMEFONT, 60, white);
				DrawText(350, 590, "View High Scores", GAMEFONT, 60, white);
				DrawText(350, 660, "Quit", GAMEFONT, 60, white);
			} else if(menuOptionSelected == 1) {
				DrawText(350, 450, "Start Game", GAMEFONT, 60, white);
				DrawText(350, 520, "Level Select", GAMEFONT, 60, red);
				DrawText(350, 590, "View High Scores", GAMEFONT, 60, white);
				DrawText(350, 660, "Quit", GAMEFONT, 60, white);
			} else if (menuOptionSelected ==2) {
				DrawText(350, 450, "Start Game", GAMEFONT, 60, white);
				DrawText(350, 520, "Level Select", GAMEFONT, 60, white);
				DrawText(350, 590, "View High Scores", GAMEFONT, 60, red);
				DrawText(350, 660, "Quit", GAMEFONT, 60, white);
			} else {
				DrawText(350, 450, "Start Game", GAMEFONT, 60, white);
				DrawText(350, 520, "Level Select", GAMEFONT, 60, white);
				DrawText(350, 590, "View High Scores", GAMEFONT, 60, white);
				DrawText(350, 660, "Quit", GAMEFONT, 60, red);
			}

			SDL_Flip(screen);

			return;
			break;
		case LEVEL_SELECT_STATE:
			r.w = 1024;
			r.h = 768;
			SDL_FillRect(screen, &r, 0);

			DrawText(1024/2, 250, "Level Select", GAMEFONT, 95, white);

			DrawText(1024/2, 550, "Level: " + itos(levelSelect), GAMEFONT, 60, white);

			if (menuOptionSelected == 0) {
				DrawText(1024/2 - 256, 690, "Start Level", GAMEFONT, 60, red);
				DrawText(1024/2 + 256, 690, "Main Menu", GAMEFONT, 60, white);
			} else {
				DrawText(1024/2 - 256, 690, "Start Level", GAMEFONT, 60, white);
				DrawText(1024/2 + 256, 690, "Main Menu", GAMEFONT, 60, red);
			}

			SDL_Flip(screen);

			return;
			break;
		case TOTAL_VICTORY_STATE:
			SDL_FillRect(screen, NULL, 0);
			DrawText(1024/2, 250, "Congratulations!", GAMEFONT, 80, white);
			DrawText(1024/2, 325, "You win!", GAMEFONT, 80, white);

			DrawText(1024/2, 425, "Final Score:", GAMEFONT, 80, white);
			DrawText(1024/2, 500, itos(score), GAMEFONT, 80, white);

			SDL_Flip(screen);
			return;
			break;
		case CREDITS_STATE:
			SDL_FillRect(screen, NULL, 0);
			DrawText(1024/2, 250, "Credits", GAMEFONT, 80, white);
			DrawText(1024/2, 350, "David McAllister", GAMEFONT, 80, white);

			DrawText(1024/2, 425, "Mark Bosko", GAMEFONT, 80, white);
			DrawText(1024/2, 500, "Don Li", GAMEFONT, 80, white);
			SDL_Flip(screen);

			return;
			break;

		case ENTER_HIGH_SCORE_STATE:
			SDL_FillRect(screen, NULL, 0);

			DrawText(1024/2, 75, "New High Score!", GAMEFONT, 80, white);

			for (int i = 0; i < 3; i++) {
				if (i == charSelected) {
					DrawText(1024/2 - 280 + i * 50, 768/2, initials.substr(i, 1), GAMEFONT, 60, red);
				} else {
					DrawText(1024/2 - 280 + i * 50, 768/2, initials.substr(i, 1), GAMEFONT, 60, white);
				}
			}
			DrawText(1024/2 +256, 768/2, itos(score), GAMEFONT, 60, white);

			SDL_Flip(screen);

			return;
			break;

		case VIEW_HIGH_SCORES_STATE:
			SDL_FillRect(screen, NULL, 0);

			DrawText(1024/2, 25, "High Scores", GAMEFONT, 50, white);

			for (int i = 0; i < 10; i++) {
				DrawText(1024/2, 250 + i * 40, hscores[i].name + "     " + itos(hscores[i].score), GAMEFONT, 30, white);
			}

			SDL_Flip(screen);

			return;
			break;

		case GAME_STATE:
			break;
	}


	DrawImage(0,0, bg, 0, 0, 768, map.height * 30 + offset.y);

	//Display
	for (int y = 0; y < map.height; y++) {
		for (int x = 0; x < map.width; x++) {
			if (map.b[x][y]) {
				DrawImage(x*30 + offset.x, y*30 + offset.y, blocks, map.colors[x/5][y/5]*60-60 + (map.b[x][y]-1)*30, 30+animFrame*30, 30, 30);
			} else {
				int bx = 0;
				
				if (blocks->h > 30) {
					bool left = (x > 0) && !map.b[x-1][y];
					bool right = (x < map.width-1) && !map.b[x+1][y];
					bool up = (y > 0) && !map.b[x][y-1];
					bool down = (y < map.height-1) && !map.b[x][y+1];

					if (!left && up && !down && !right) bx=30;
					if (!left && !up && !down && right) bx=60;
					if (!left && !up && down && !right) bx=90;
					if (left && !up && !down && !right) bx=120;

					if (!left && up && down && !right) bx=150;
					if (left && !up && !down && right) bx=180;
					if (!left && up && !down && right) bx=210;
					if (!left && !up && down && right) bx=240;
					if (left && !up && down && !right) bx=270;
					if (left && up && !down && !right) bx=300;

					if (left && up && !down && right) bx=330;
					if (!left && up && down && right) bx=360;
					if (left && !up && down && right) bx=390;
					if (left && up && down && !right) bx=420;
				}

				DrawImage(x*30 + offset.x, y*30 + offset.y, blocks, bx, 0, 30, 30);
			}
			//cout << map.b[x][y];
		}
		//cout << endl;
	}

	tickRotate[active] += timeDif;
	if (tickRotate[active] >= 16*35) tickRotate[active] -= (16*35);
	curRotate[active] = tickRotate[active] / 35;

	DrawImage(start[0].x*30 + offset.x, start[0].y*30 + offset.y, whiteLeft);
	DrawImage(start[1].x*30 + offset.x, start[1].y*30 + offset.y, blackLeft);

	bool blackOnStart = (start[1].x*30 == pixelPos[1].x) && (start[1].y*30 == pixelPos[1].y);
	bool whiteOnStart = (start[0].x*30 == pixelPos[0].x) && (start[0].y*30 == pixelPos[0].y);
	bool whiteOnEnd = (start[1].x*30 == pixelPos[0].x) && (start[1].y*30 == pixelPos[0].y);
	bool blackOnEnd = (start[0].x*30 == pixelPos[1].x) && (start[0].y*30 == pixelPos[1].y);


	//start
	if (whiteOnStart)
		DrawImage(start[0].x*30 + offset.x, start[0].y*30 + offset.y, whiteStart);
	if (blackOnStart)
		DrawImage(start[1].x*30 + offset.x, start[1].y*30 + offset.y, blackStart);

	//end
	if (blackOnEnd)
		DrawImage(start[0].x*30 + offset.x, start[0].y*30 + offset.y, complete2);
	if (whiteOnEnd)
		DrawImage(start[1].x*30 + offset.x, start[1].y*30 + offset.y, complete);

	
		if (!(whiteOnStart || whiteOnEnd))
			DrawImage(pixelPos[0].x - (whiteRight[curRotate[0]]->w / 2 - 15) + offset.x, pixelPos[0].y - (whiteRight[curRotate[0]]->h / 2 - 15) + offset.y, whiteRight[curRotate[0]]);
		
		if (!(blackOnStart || blackOnEnd))
			DrawImage(pixelPos[1].x - (blackRight[curRotate[1]]->w / 2 - 15) + offset.x, pixelPos[1].y - (blackRight[curRotate[1]]->h / 2 - 15) + offset.y, blackRight[curRotate[1]]);
			

	if (swapMode) {
		DrawImage(swapDest.x*150 + offset.x, swapDest.y*150 + offset.y, border);
		DrawImage((pos[active].x/5)*150 + offset.x, (pos[active].y/5)*150 + offset.y, border);
	}


	r.x = 768;
	r.y = 0;
	r.w = 256;
	r.h = 768;

	
	if (displayedMessage.compare(sideMessage[0])) {
		
		DrawImage(0,map.height*30+offset.y,bg,0, map.height*30+offset.y, 768, 768-(map.height*30+offset.y));

		if (sideMessage[0].length() > 0) DrawText(768 / 2, 768 - 125, sideMessage[0], GAMEFONT, 25, white);
		if (sideMessage[1].length() > 0) DrawText(768 / 2, 768 - 75, sideMessage[1], GAMEFONT, 25, white);
		displayedMessage = sideMessage[0];
	}

	if (level != displayedLevel || bonus != displayedScore) {

		SDL_FillRect(screen, &r, 0xFFFFFFFF);

		//Sidebar
		DrawText(768 + 128, 25, "Level:", GAMEFONT, 40, black);
		DrawText(768 + 128, 75, itos(level), GAMEFONT, 40, black);

		DrawText(768 + 128, 150, "Total Score:", GAMEFONT, 35, black);
		DrawText(768 + 128, 200, itos(score), GAMEFONT, 40, black);

		DrawText(768 + 128, 275, "Level Score:", GAMEFONT, 35, black);
		DrawText(768 + 128, 325, itos(bonus), GAMEFONT, 40, black);

		DrawText(768 + 128, 400, "Controls:", GAMEFONT, 40, black);
		DrawImage(768, 425, controls);

		displayedLevel = level;
		displayedScore = bonus;
	}


	SDL_Flip(screen);
}

int LoadImages(string tileSet, string bgName) {

	SDL_Surface *temp = NULL;
	temp = SDL_LoadBMP( tileSet.c_str());
	if (temp == NULL) return 1;
	blocks = SDL_DisplayFormat( temp );
	if (blocks == NULL) return 1;
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP( bgName.c_str());
	if (temp == NULL) return 1;
	bg = SDL_DisplayFormat( temp );
	if (bg == NULL) return 1;
	SDL_FreeSurface(temp);
	
	temp = SDL_LoadBMP( "gfx\\border.bmp");
	if (temp == NULL) return 1;
	border = SDL_DisplayFormat( temp );
	if (border == NULL) return 1;
	SDL_FreeSurface(temp);
	SDL_SetColorKey(border, SDL_SRCCOLORKEY, SDL_MapRGB(border->format, 255, 128, 255));

	temp = SDL_LoadBMP( "gfx\\white left.bmp");
	if (temp == NULL) return 1;
	whiteLeft = SDL_DisplayFormat( temp );
	if (whiteLeft == NULL) return 1;
	SDL_SetColorKey(whiteLeft, SDL_SRCCOLORKEY, SDL_MapRGB(border->format, 255, 128, 255));
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP( "gfx\\white right.bmp");
	if (temp == NULL) return 1;
	whiteRight[0] = SDL_DisplayFormat( temp );
	if (whiteRight[0] == NULL) return 1;
	SDL_SetColorKey(whiteRight[0], SDL_SRCCOLORKEY, SDL_MapRGB(border->format, 0, 0, 0));
	SDL_FreeSurface(temp);

	for (int i = 1; i < 16; i++) {
		whiteRight[i] = rotozoomSurface (whiteRight[0], 360/16 * i, 1, 0);
		SDL_SetColorKey(whiteRight[i], SDL_SRCCOLORKEY, SDL_MapRGB(border->format, 0, 0, 0));
	}

	temp = SDL_LoadBMP( "gfx\\white start.bmp");
	if (temp == NULL) return 1;
	whiteStart = SDL_DisplayFormat( temp );
	if (whiteStart == NULL) return 1;
	SDL_SetColorKey(whiteStart, SDL_SRCCOLORKEY, SDL_MapRGB(whiteStart->format, 255, 128, 255));
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP( "gfx\\black left.bmp");
	if (temp == NULL) return 1;
	blackLeft = SDL_DisplayFormat( temp );
	if (blackLeft == NULL) return 1;
	SDL_SetColorKey(blackLeft, SDL_SRCCOLORKEY, SDL_MapRGB(border->format, 255, 128, 255));
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP( "gfx\\black right.bmp");
	if (temp == NULL) return 1;
	blackRight[0] = SDL_DisplayFormat( temp );
	if (blackRight[0] == NULL) return 1;
	SDL_SetColorKey(blackRight[0], SDL_SRCCOLORKEY, SDL_MapRGB(border->format, 0, 0, 0));
	SDL_FreeSurface(temp);

	for (int i = 1; i < 16; i++) {
		blackRight[i] = rotozoomSurface (blackRight[0], 360/16 * i, 1, 1);
		SDL_SetColorKey(blackRight[i], SDL_SRCCOLORKEY, SDL_MapRGB(border->format, 0, 0, 0));
	}

	temp = SDL_LoadBMP( "gfx\\black start.bmp");
	if (temp == NULL) return 1;
	blackStart = SDL_DisplayFormat( temp );
	if (blackStart == NULL) return 1;
	SDL_SetColorKey(blackStart, SDL_SRCCOLORKEY, SDL_MapRGB(border->format, 255, 128, 255));
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP( "gfx\\complete.bmp");
	if (temp == NULL) return 1;
	complete = SDL_DisplayFormat( temp );
	if (complete == NULL) return 1;
	SDL_SetColorKey(complete, SDL_SRCCOLORKEY, SDL_MapRGB(border->format, 255, 128, 255));
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP( "gfx\\complete2.bmp");
	if (temp == NULL) return 1;
	complete2 = SDL_DisplayFormat( temp );
	if (complete2 == NULL) return 1;
	SDL_SetColorKey(complete2, SDL_SRCCOLORKEY, SDL_MapRGB(border->format, 255, 128, 255));
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP( "gfx\\controls.bmp");
	if (temp == NULL) return 1;
	controls = SDL_DisplayFormat( temp );
	if (controls == NULL) return 1;
	SDL_FreeSurface(temp);

	return 0;

}

void DrawImage(int x, int y, SDL_Surface* source, int x2, int y2, int w, int h )
{
    SDL_Rect offset;
    
    offset.x = x;
    offset.y = y;

	SDL_Rect sourceRect;
	sourceRect.x = x2;
	sourceRect.y = y2;
	sourceRect.w = w;
	sourceRect.h = h;

	SDL_BlitSurface( source, &sourceRect, screen, &offset );

}


void DrawImage(int x, int y, SDL_Surface* source )
{
    SDL_Rect offset;
    
    offset.x = x;
    offset.y = y;

	SDL_BlitSurface( source, NULL, screen, &offset );

}

void DrawImage(int x, int y, SDL_Surface* source, SDL_Surface* dest )
{
    SDL_Rect offset;
    
    offset.x = x;											
    offset.y = y;

	SDL_BlitSurface( source, NULL, dest, &offset );

}

void DrawText(int x, int y, string text, string fontString, int size, SDL_Color color) {
	TTF_Font *font = TTF_OpenFont( fontString.c_str(), size );
	SDL_Surface *message = TTF_RenderText_Solid( font, text.c_str(), color );

	DrawImage(x - message->w / 2, y - message->h / 2, message);

	SDL_FreeSurface(message);
    TTF_CloseFont( font );
}


string itos(int val) {
	stringstream out;
	out << val;
	return out.str();
}

int isHighScore(int score) {
	ifstream fin("scores.txt");

	for (int i = 0; i < 10; i++) {
		fin >> hscores[i].name;
		fin >> hscores[i].score;
	}

	if (score < hscores[9].score) return -2;

	int s = 9;

	while (score > hscores[s].score && s > -1) {
		s--;
	}

	return s+1;
}

void saveScore(string initials, int score) {

	int ins = isHighScore(score);
	cout << ins;
	if (ins == -1) return;

	for (int i = 9; i > ins; i--) {
		hscores[i].name = hscores[i-1].name;
		hscores[i].score = hscores[i-1].score;
	}

	hscores[ins].name = initials;
	hscores[ins].score = score;

	ofstream fout("scores.txt");

	for (int i = 0; i < 10; i++) {
		fout << hscores[i].name << " " << hscores[i].score << endl;
	}
}
