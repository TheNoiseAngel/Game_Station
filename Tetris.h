#pragma once

#include "Framework.h"
#include "Puntajes.h"
#include <ctime>
#include <vector>
#include <iostream>

struct Tetris_Game : Framework::Scene
{
	const int CELL = 30;
	const int CANVAS_W = 300;
	const int CANVAS_H = 600;
	const int SIDEBAR_W = 200;
	const int TOTAL_WIDTH = CANVAS_W + SIDEBAR_W;
	static const int BOARD_W = 10;
	static const int BOARD_H = 20;

	int pieceX = BOARD_W / 2 - 2;
	int pieceY = 0;

	SDL_Keycode LastKey;

	int Score = 0;
	int Level = 1;
	int Lines = 0;
	int SigLevel = 10;

	float LastFall = 0.0f;
	float SPEED_FALL = 0.5f;

	struct Piece
	{
		std::vector<std::vector<int>> shape;
		int colorIndex;
	};

	Piece currentPiece;
	Piece nextPiece;

	enum State
	{
		INICIO,
		JUGANDO,
		GAME_OVER
	} state = INICIO;

	// Colores de las piezas
	const SDL_Color COLORS[8] = {
		{0, 0, 0, 255},		// 0 - vacío
		{0, 255, 255, 255}, // 1 - I (cyan)
		{255, 255, 0, 255}, // 2 - O (amarillo)
		{128, 0, 128, 255}, // 3 - T (púrpura)
		{0, 255, 0, 255},	// 4 - S (verde)
		{255, 0, 0, 255},	// 5 - Z (rojo)
		{0, 0, 255, 255},	// 6 - J (azul)
		{255, 165, 0, 255}	// 7 - L (naranja)
	};

	std::vector<Piece> pieces = {
		// I (color 1)
		{{{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}}, 1},
		// O (color 2)
		{{{0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}}, 2},
		// T (color 3)
		{{{0, 0, 0, 0}, {0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}}, 3},
		// S (color 4)
		{{{0, 0, 0, 0}, {0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}}, 4},
		// Z (color 5)
		{{{0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}}, 5},
		// J (color 6)
		{{{0, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}}, 6},
		// L (color 7)
		{{{0, 0, 0, 0}, {0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}}, 7}};

	int board[BOARD_H][BOARD_W];

	Framework::ResourceManager *res;
	Framework::Engine *engine;

	Tetris_Game(Framework::ResourceManager *r, Framework::Engine *e) : res(r), engine(e)
	{
		std::srand(std::time(nullptr));
		SysPuntaje::cargar();
		RESET_GAME();
	}

	void RESET_GAME()
	{
		// Limpiar tablero
		for (int i = 0; i < BOARD_H; ++i)
			for (int j = 0; j < BOARD_W; ++j)
				board[i][j] = 0;

		pieceX = BOARD_W / 2 - 2;
		pieceY = 0;
		Score = 0;
		Level = 1;
		Lines = 0;
		SigLevel = 10;
		SPEED_FALL = 0.5f;
		LastFall = 0.0f;
		state = INICIO;

		currentPiece = getRandomPiece();
		nextPiece = getRandomPiece();

		Mix_Music *music = res->musics["bg_music"];
		if (music)
			Mix_PlayMusic(music, -1);
	}

	void handleEvent(const SDL_Event &e) override
	{
		if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
			requestPop = true;

		switch (state)
		{
		case INICIO:
			if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE)
			{
				state = JUGANDO;
			}
			break;

		case JUGANDO:
			if (e.type == SDL_KEYDOWN)
			{
				SDL_Keycode key = e.key.keysym.sym;
				if (key == SDLK_LEFT || key == SDLK_a ||
					key == SDLK_RIGHT || key == SDLK_d ||
					key == SDLK_DOWN || key == SDLK_s ||
					key == SDLK_UP || key == SDLK_w ||
					key == SDLK_SPACE)
				{
					LastKey = key;
					if (LastKey == SDLK_LEFT || LastKey == SDLK_a)
					{
						if (!collides(currentPiece.shape, pieceX - 1, pieceY))
							pieceX--;
					}
					else if (LastKey == SDLK_RIGHT || LastKey == SDLK_d)
					{
						if (!collides(currentPiece.shape, pieceX + 1, pieceY))
							pieceX++;
					}
					else if (LastKey == SDLK_DOWN || LastKey == SDLK_s)
					{
						if (!collides(currentPiece.shape, pieceX, pieceY + 1))
						{
							pieceY++;
						}
						else
						{
							lockPieceAndSpawnNext();
						}
					}
					else if (LastKey == SDLK_UP || LastKey == SDLK_w)
					{
						auto rotated = rotatePiece(currentPiece.shape);
						if (!collides(rotated, pieceX, pieceY))
							currentPiece.shape = rotated;

						Mix_Chunk *rotateSnd = res->sounds["rotate"];
						if (rotateSnd)
							Mix_PlayChannel(-1, rotateSnd, 0);
					}
					else if (LastKey == SDLK_SPACE)
					{
						// Hard drop
						while (!collides(currentPiece.shape, pieceX, pieceY + 1))
							pieceY++;
						lockPieceAndSpawnNext();
					}
				}
			}
			break;

		case GAME_OVER:
			if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_r)
			{
				RESET_GAME();
			}
			break;
		}
	}

	void update(float delta) override
	{
		if (state == JUGANDO)
		{
			LastFall += delta;
			if (LastFall >= SPEED_FALL)
			{
				if (!collides(currentPiece.shape, pieceX, pieceY + 1))
				{
					pieceY++;
				}
				else
				{
					lockPieceAndSpawnNext();
				}
				LastFall -= SPEED_FALL;
			}
		}
	}

	void draw(SDL_Renderer *ren) override
	{
		int winW, winH;
		SDL_GetRendererOutputSize(ren, &winW, &winH);

		int offsetX = std::max(0, (winW - TOTAL_WIDTH) / 2);
		int offsetY = std::max(0, (winH - CANVAS_H) / 2);

		SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
		SDL_RenderClear(ren);

		int sidebarX = offsetX + CANVAS_W;

		// Fondos
		SDL_Rect canvasRect{offsetX, offsetY, CANVAS_W, CANVAS_H};
		SDL_SetRenderDrawColor(ren, 0x04, 0x04, 0x04, 255);
		SDL_RenderFillRect(ren, &canvasRect);

		SDL_Rect sb{sidebarX, offsetY, SIDEBAR_W, CANVAS_H};
		SDL_SetRenderDrawColor(ren, 0x30, 0x30, 0x70, 255);
		SDL_RenderFillRect(ren, &sb);

		if (state == INICIO || state == JUGANDO)
		{
			// Tablero (bloques fijos)
			for (int row = 0; row < BOARD_H; ++row)
			{
				for (int col = 0; col < BOARD_W; ++col)
				{
					int colorIdx = board[row][col];
					if (colorIdx != 0)
					{
						SDL_Color c = COLORS[colorIdx];
						SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, c.a);
						SDL_Rect rect = {col * CELL + offsetX, row * CELL + offsetY, CELL, CELL};
						SDL_RenderFillRect(ren, &rect);
						SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
						SDL_RenderDrawRect(ren, &rect);
					}
					else
					{
						// Cuadrícula
						SDL_SetRenderDrawColor(ren, 40, 40, 40, 255);
						SDL_Rect rect = {col * CELL + offsetX, row * CELL + offsetY, CELL, CELL};
						SDL_RenderDrawRect(ren, &rect);
					}
				}
			}

			// Pieza actual

			if (state == JUGANDO)
			{
				const auto &shape = currentPiece.shape;
				SDL_Color pieceColor = COLORS[currentPiece.colorIndex];
				for (int i = 0; i < 4; ++i)
				{
					for (int j = 0; j < 4; ++j)
					{
						if (shape[i][j])
						{
							int x = (pieceX + j) * CELL;
							int y = (pieceY + i) * CELL;
							SDL_SetRenderDrawColor(ren, pieceColor.r, pieceColor.g, pieceColor.b, pieceColor.a);
							SDL_Rect rect = {x + offsetX, y + offsetY, CELL, CELL};
							SDL_RenderFillRect(ren, &rect);
							SDL_SetRenderDrawColor(ren, 200, 200, 200, 255);
							SDL_RenderDrawRect(ren, &rect);
						}
					}
				}
			}

			// Textos
			if (res && res->fonts["main"])
			{
				Framework::drawText(ren, res->fonts["main"], "Puntos: " + std::to_string(Score), sidebarX + 24, offsetY + 60, {255, 255, 255, 255});
				Framework::drawText(ren, res->fonts["main"], "Nivel: " + std::to_string(Level), sidebarX + 24, offsetY + 110, {80, 255, 80, 255});
				Framework::drawText(ren, res->fonts["main"], "Lineas: " + std::to_string(Lines), sidebarX + 24, offsetY + 160, {200, 200, 200, 255});

				// Mostrar récord
				int recordActual = SysPuntaje::records["Tetris"];
				Framework::drawText(ren, res->fonts["main"], "Record: " + std::to_string(recordActual), sidebarX + 24, offsetY + 210, {255, 215, 0, 255});

				if (state == INICIO)
					Framework::drawText(ren, res->fonts["main"], "ESPACIO PARA EMPEZAR", offsetX + 20, offsetY + 250, {255, 255, 0, 255});
			}
		}
		else if (state == GAME_OVER)
		{
			if (res && res->fonts["main"])
			{
				Framework::drawText(ren, res->fonts["main"], "GAME OVER", offsetX + 80, offsetY + 250, {255, 0, 0, 255});
				Framework::drawText(ren, res->fonts["main"], "Pulsa <R> Para reiniciar", offsetX + 50, offsetY + 300, {255, 255, 255, 255});
			}
		}
	}

	void onEnter() override
	{
		// Cargar música de fondo (se cachea bajo la clave "bg_music")
		res->loadMusic("bg_music", "assets/Tetris_Theme.mp3");
		// Cargar efectos
		res->loadSound("rotate", "assets/sound_1.wav");
		res->loadSound("lock", "assets/sound_2.wav");
		res->loadSound("line_clear", "assets/sound_2.wav");
		res->loadSound("game_over", "assets/sound_over.wav");

		// Iniciar reproducción de música en bucle
		Mix_Music *music = res->musics["bg_music"];
		if (music)
			Mix_PlayMusic(music, -1); // -1 = loop infinito
	}

  private:
	std::vector<std::vector<int>> rotatePiece(const std::vector<std::vector<int>> &shape)
	{
		int n = shape.size();
		std::vector<std::vector<int>> rotated(n, std::vector<int>(n, 0));
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j)
				rotated[j][n - 1 - i] = shape[i][j];
		return rotated;
	}

	bool collides(const std::vector<std::vector<int>> &shape, int x, int y)
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				if (shape[i][j] != 0)
				{
					int boardX = x + j;
					int boardY = y + i;
					if (boardX < 0 || boardX >= BOARD_W || boardY >= BOARD_H)
						return true;
					if (boardY >= 0 && board[boardY][boardX] != 0)
						return true;
				}
			}
		}
		return false;
	}

	void fixPiece(const std::vector<std::vector<int>> &shape, int x, int y, int colorIndex)
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				if (shape[i][j] != 0)
				{
					int boardX = x + j;
					int boardY = y + i;
					if (boardY >= 0 && boardY < BOARD_H && boardX >= 0 && boardX < BOARD_W)
						board[boardY][boardX] = colorIndex;
				}
			}
		}
	}

	int clearLines()
	{
		int linesCleared = 0;
		for (int row = BOARD_H - 1; row >= 0;)
		{
			bool full = true;
			for (int col = 0; col < BOARD_W; ++col)
			{
				if (board[row][col] == 0)
				{
					full = false;
					break;
				}
			}
			if (full)
			{
				for (int r = row; r > 0; --r)
					for (int c = 0; c < BOARD_W; ++c)
						board[r][c] = board[r - 1][c];
				for (int c = 0; c < BOARD_W; ++c)
					board[0][c] = 0;
				linesCleared++;
			}
			else
			{
				--row;
			}
		}
		return linesCleared;
	}

	Piece getRandomPiece()
	{
		int idx = rand() % pieces.size();
		return pieces[idx];
	}

	// Función que encapsula la lógica de fijar pieza, eliminar líneas y generar la siguiente
	void lockPieceAndSpawnNext()
	{
		fixPiece(currentPiece.shape, pieceX, pieceY, currentPiece.colorIndex);
		int cleared = clearLines();
		if (cleared > 0)
		{
			Mix_Chunk *lineSnd = res->sounds["line_clear"];
			if (lineSnd)
				Mix_PlayChannel(-1, lineSnd, 0);
		}
		Lines += cleared;
		Score += 10 * cleared * (Level + 1);

		// Subir de nivel cada 10 líneas (clásico)
		if (Lines >= SigLevel)
		{
			Level++;
			SigLevel += 10;
			SPEED_FALL *= 0.85f;
			if (SPEED_FALL < 0.05f)
				SPEED_FALL = 0.05f;
		}

		currentPiece = nextPiece;
		nextPiece = getRandomPiece();
		pieceX = BOARD_W / 2 - 2;
		pieceY = 0;

		if (collides(currentPiece.shape, pieceX, pieceY))
		{
			state = GAME_OVER;
			Mix_HaltMusic();
			Mix_Chunk *goSnd = res->sounds["game_over"];
			if (goSnd)
				Mix_PlayChannel(-1, goSnd, 0);
			SysPuntaje::actualizar("Tetris", Score);
		}
	}
};