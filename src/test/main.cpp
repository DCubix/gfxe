#include <iostream>

#include "window.h"
#include "shader.h"
#include "buffer.h"
#include "texture.h"
#include "framebuffer.h"

#include "stb_image.h"

class Game : public GameAdapter {
public:
	void onSetup(Window* win) {
		const std::string vs = R"(#version 430 core
layout (location = 0) in vec3 pos;
out vec3 color;
void main() {
	gl_Position = vec4(pos, 1.0);
	color = pos * 0.5 + 0.5;
})";

		const std::string fs = R"(#version 430 core
out vec4 fragColor;
in vec3 color;
uniform sampler2D tex;
void main() {
	fragColor = texture(tex, color.xy * 2.0);
})";

		shader.create()
			.add(vs, Shader::VertexShader)
			.add(fs, Shader::FragmentShader)
			.link();

		VertexFormat<sizeof(float) * 3> fmt{};
		fmt.add(3, DataType::TypeFloat);

		f32 verts[] = {
			-1.0f, -1.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,
			 0.0f,  1.0f, 0.0f
		};

		arr.create().bind();
		buf.create(Buffer::ArrayBuffer)
			.bind()
			.update(std::vector<float>(verts, verts + 9));
		fmt.enable();
		arr.unbind();

		i32 w, h, comp;
		u8* data = stbi_load("bricks.png", &w, &h, &comp, 4);
		if (data) {
			tex.create(TextureType::Texture2D, Format::RGBA, w, h).bind()
				.filter(TextureFilter::LinearMipMapLinear, TextureFilter::Linear)
				.wrapMode(TextureWrap::Repeat, TextureWrap::Repeat)
				.update(data, DataType::TypeUByte)
				.generateMipmaps();

			stbi_image_free(data);
		}

		fbo.create(640, 480)
			.color(TextureType::Texture2D, Format::RGB);
	}

	void onUpdate(Window* win, f32 dt) {

	}

	void onDraw(Window* win) {
		fbo.bind();
		glClear(GL_COLOR_BUFFER_BIT);

		arr.bind();
		tex.bind();
		shader.bind();
		shader.get("tex").set(i32(0));

		glDrawArrays(GL_TRIANGLES, 0, 3);

		fbo.unbind();
		fbo.bind(FrameBufferTarget::ReadFrameBuffer);
		fbo.blit(0, 0, 640, 480, 0, 0, 640, 480, ClearBufferMask::ColorBuffer, TextureFilter::Nearest);
	}

	FrameBuffer fbo;
	Texture tex;
	Shader shader;
	Buffer buf;
	VertexArray arr;
};

int main(int argc, char** argv) {
	return Window{ new Game(), 640, 480 }.run();
}