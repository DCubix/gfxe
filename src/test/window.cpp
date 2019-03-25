#include "window.h"

#include <iostream>
#include <algorithm>

#include "glad/glad.h"
#include "log.h"

static void APIENTRY MessageCallback(
		GLenum source, GLenum type, GLuint id,
		GLenum severity, GLsizei length,
		const GLchar* msg, const void* ud
) {
	std::string src = "";
	switch (source) {
		case GL_DEBUG_SOURCE_API: src = "API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: src = "WINDOW SYSTEM"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: src = "SHADER COMPILER"; break;
		case GL_DEBUG_SOURCE_APPLICATION: src = "APPLICATION"; break;
		default: src = "OTHER"; break;
	}

	std::string typ = "";
	switch (type) {
		case GL_DEBUG_TYPE_ERROR: typ = "ERROR"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typ = "DEPRECATED"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typ = "U.B."; break;
		case GL_DEBUG_TYPE_PERFORMANCE: typ = "PERFORMANCE"; break;
		default: src = "OTHER"; break;
	}

	LogLevel lvl = LogLevel::Info;
	switch (severity) {
		case GL_DEBUG_SEVERITY_LOW: lvl = LogLevel::Warning; break;
		case GL_DEBUG_SEVERITY_MEDIUM: lvl = LogLevel::Error; break;
		case GL_DEBUG_SEVERITY_HIGH: lvl = LogLevel::Fatal; break;
		default: break;
	}

	Print(lvl, "OpenGL (", src, " [", typ, "]): ", msg);
}

Window::Window(GameAdapter *adapter, u32 width, u32 height) {
	if (SDL_Init(SDL_INIT_EVERYTHING) > 0) {
		Log(SDL_GetError());
		return;
	}

	m_width = width;
	m_height = height;
	m_adapter = std::unique_ptr<GameAdapter>(adapter);

	Log(m_width, "x", m_height);

	m_window = SDL_CreateWindow(
		"Game Canvas",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width, height,
		SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
	);
	if (!m_window) {
		Log(SDL_GetError());
		SDL_Quit();
		return;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	int contextFlags = 0;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_FLAGS, &contextFlags);
	contextFlags |= SDL_GL_CONTEXT_DEBUG_FLAG;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, contextFlags);

	m_context = SDL_GL_CreateContext(m_window);
	if (!m_context) {
		Log(SDL_GetError());
		SDL_Quit();
		return;
	}

	if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
		Log(SDL_GetError());
		SDL_Quit();
		return;
	}

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(MessageCallback, 0);
}

i32 Window::run() {
	if (m_context == nullptr || m_window == nullptr)
		return -1;

	SDL_Event evt;
	bool running = true;

	const f64 timeStep = 1.0 / 60.0;
	f64 accum = 0.0, lastTime = f64(SDL_GetTicks()) / 1000.0;

	m_adapter->onSetup(this);

	while (running) {
		bool canRender = false;
		f64 currTime = f64(SDL_GetTicks()) / 1000.0;
		f64 delta = currTime - lastTime;
		lastTime = currTime;
		accum += delta;

		for (auto& e : m_keyboard) {
			e.second.pressed = false;
			e.second.released = false;
		}

		while (SDL_PollEvent(&evt)) {
			switch (evt.type) {
				case SDL_QUIT: running = false; break;
				case SDL_KEYDOWN: {
					m_keyboard[evt.key.keysym.sym].pressed = true;
					m_keyboard[evt.key.keysym.sym].held = true;
				} break;
				case SDL_KEYUP: {
					m_keyboard[evt.key.keysym.sym].released = true;
					m_keyboard[evt.key.keysym.sym].held = false;
				} break;
				default: break;
			}
		}

		while (accum >= timeStep) {
			m_adapter->onUpdate(this, f32(timeStep));
			accum -= timeStep;
			canRender = true;
		}

		if (canRender) {
			m_adapter->onDraw(this);
			SDL_GL_SwapWindow(m_window);
		}
	}


	SDL_GL_DeleteContext(m_context);
	SDL_DestroyWindow(m_window);
	SDL_Quit();

	return 0;
}
