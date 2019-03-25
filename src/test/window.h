#ifndef WINDOW_H
#define WINDOW_H

#include "integer.h"
#include "SDL2/SDL.h"

#include <memory>
#include <vector>
#include <unordered_map>

class Window;
class GameAdapter {
public:
	virtual void onSetup(Window* canvas) {}
	virtual void onUpdate(Window* canvas, f32 dt) {}
	virtual void onDraw(Window* canvas) {}
};

class Window {
public:
	Window(GameAdapter *adapter, u32 width, u32 height);

	i32 run();

	u32 width() const { return m_width; }
	u32 height() const { return m_height; }

	bool isPressed(u32 key) { return m_keyboard[key].pressed; }
	bool isReleased(u32 key) { return m_keyboard[key].released; }
	bool isHeld(u32 key) { return m_keyboard[key].held; }

private:
	SDL_Window *m_window;
	SDL_GLContext m_context;

	std::unique_ptr<GameAdapter> m_adapter;

	u32 m_width, m_height;

	struct State {
		bool pressed, released, held;
	};

	std::unordered_map<u32, State> m_keyboard;
};

#endif // WINDOW_H