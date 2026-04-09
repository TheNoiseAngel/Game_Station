#ifndef FRAMEWORK_SDL_BASE_H
#define FRAMEWORK_SDL_BASE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <iostream>

namespace Framework
{
struct ResourceManager
{
	std::map<std::string, TTF_Font *> fonts;
	std::map<std::string, SDL_Texture *> textures;
	std::map<std::string, Mix_Music *> musics;
	std::map<std::string, Mix_Chunk *> sounds;
	SDL_Renderer *renderer = nullptr;

	void setRenderer(SDL_Renderer *r) { renderer = r; }
	// Métodos para cargar recursos bajo un nombre
	TTF_Font *loadFont(const std::string &key, const std::string &file, int size)
	{
		if (fonts[key])
			return fonts[key];
		fonts[key] = TTF_OpenFont(file.c_str(), size);
		return fonts[key];
	}
	Mix_Music *loadMusic(const std::string &key, const std::string &file)
	{
		if (musics[key])
			return musics[key];
		musics[key] = Mix_LoadMUS(file.c_str());
		return musics[key];
	}
	Mix_Chunk *loadSound(const std::string &key, const std::string &file)
	{
		if (sounds[key])
			return sounds[key];
		sounds[key] = Mix_LoadWAV(file.c_str());
		return sounds[key];
	}
	SDL_Texture *loadTexture(const std::string &key, const std::string &file)
	{
		if (textures[key])
			return textures[key];
		SDL_Surface *s = SDL_LoadBMP(file.c_str()); // O usar IMG_Load si usas SDL_image
		if (!s)
			return nullptr;
		textures[key] = SDL_CreateTextureFromSurface(renderer, s);
		SDL_FreeSurface(s);
		return textures[key];
	}
	void cleanup()
	{
		for (auto &x : fonts)
			if (x.second)
				TTF_CloseFont(x.second);
		for (auto &x : musics)
			if (x.second)
				Mix_FreeMusic(x.second);
		for (auto &x : sounds)
			if (x.second)
				Mix_FreeChunk(x.second);
		for (auto &x : textures)
			if (x.second)
				SDL_DestroyTexture(x.second);
		fonts.clear();
		musics.clear();
		sounds.clear();
		textures.clear();
	}
};

// ------------------- ESCENAS ------------------- //
class Scene
{
  public:
	virtual ~Scene() {}
	virtual void onEnter() {}
	virtual void onExit() {}
	virtual void handleEvent(const SDL_Event &) {}
	virtual void update(float delta) {}
	virtual void draw(SDL_Renderer *) {}
	bool requestPop = false; // Marcar para volver a escena anterior.
};

class SceneStack
{
	std::vector<std::unique_ptr<Scene>> stack;

  public:
	void push(Scene *s)
	{
		if (!stack.empty())
			stack.back()->onExit();
		stack.emplace_back(s);
		s->onEnter();
	}
	void pop()
	{
		if (!stack.empty())
		{
			stack.back()->onExit();
			stack.pop_back();
			if (!stack.empty())
				stack.back()->onEnter();
		}
	}
	Scene *top() { return stack.empty() ? nullptr : stack.back().get(); }
	bool empty() const { return stack.empty(); }
	void clear()
	{
		while (!stack.empty())
			pop();
	}
};
// ------------------- UI ELEMENTS (puedes ampliarlas) ------------------- //
struct Button
{
	SDL_Rect rect;
	SDL_Color color, textColor;
	std::string label;
	std::function<void()> onClick;
	void draw(SDL_Renderer *ren, TTF_Font *font)
	{
		SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
		SDL_RenderFillRect(ren, &rect);
		// Dibuja el texto centrado
		if (font && !label.empty())
		{
			SDL_Surface *surf = TTF_RenderText_Blended(font, label.c_str(), textColor);
			if (surf)
			{
				SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
				SDL_Rect tRect = {rect.x + rect.w / 2 - surf->w / 2, rect.y + rect.h / 2 - surf->h / 2, surf->w, surf->h};
				SDL_RenderCopy(ren, tex, nullptr, &tRect);
				SDL_FreeSurface(surf);
				SDL_DestroyTexture(tex);
			}
		}
	}
	bool hit(int x, int y) { return x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h; }
};

inline void drawText(SDL_Renderer *ren, TTF_Font *font, const std::string &msg, int x, int y, SDL_Color c)
{
	SDL_Surface *s = TTF_RenderText_Blended(font, msg.c_str(), c);
	if (s)
	{
		SDL_Texture *t = SDL_CreateTextureFromSurface(ren, s);
		SDL_Rect r = {x, y, s->w, s->h};
		SDL_RenderCopy(ren, t, nullptr, &r);
		SDL_FreeSurface(s);
		SDL_DestroyTexture(t);
	}
}

// --------------- MOTOR PRINCIPAL -------------- //
class Engine
{
  public:
	SDL_Window *win = nullptr;
	SDL_Renderer *ren = nullptr;
	int width, height;

	ResourceManager resources;
	SceneStack sceneStack;
	bool running = true;
	int targetFPS = 60;
	Engine(int w, int h) : width(w), height(h) {}
	bool init(const std::string &title)
	{
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
			return false;
		if (TTF_Init() < 0)
			return false;
		if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
			return false;
		win = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
		ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (!win || !ren)
			return false;
		resources.setRenderer(ren);
		return true;
	}
	void cleanup()
	{
		sceneStack.clear();
		resources.cleanup();
		if (ren)
			SDL_DestroyRenderer(ren);
		if (win)
			SDL_DestroyWindow(win);
		Mix_CloseAudio();
		TTF_Quit();
		SDL_Quit();
	}
	// Ciclo principal
	void run()
	{
		Uint32 last = SDL_GetTicks();
		while (running && !sceneStack.empty())
		{
			Uint32 now = SDL_GetTicks();
			float delta = (now - last) / 1000.0f;
			last = now;
			SDL_Event e;
			while (SDL_PollEvent(&e))
			{
				if (e.type == SDL_QUIT)
					running = false;
				else if (sceneStack.top())
					sceneStack.top()->handleEvent(e);
			}
			if (sceneStack.top())
				sceneStack.top()->update(delta);
			SDL_SetRenderDrawColor(ren, 30, 30, 35, 255);
			SDL_RenderClear(ren);
			if (sceneStack.top())
				sceneStack.top()->draw(ren);
			SDL_RenderPresent(ren);
			if (sceneStack.top() && sceneStack.top()->requestPop)
				sceneStack.pop();
			Uint32 frameMS = SDL_GetTicks() - now;
			if (1000 / targetFPS > frameMS)
				SDL_Delay(1000 / targetFPS - frameMS); // Fijar FPS
		}
	}
};

} // namespace Framework
#endif