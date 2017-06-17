#include "NoWork/Font.h"
#include "NoWork/Log.h"
#include "NoWork/Framework.h"



NoWork* Font::m_Framework = 0;

typedef struct {
	float x, y, z;    // position
	float s, t;       // texture
	float r, g, b, a; // color
} vertex_t;

const static std::string fontShaderVertSrc = SHADER_SRC(
	#version 130\n

	in vec3 vertex;
in vec2 tex_coord;
in vec4 color;

	out vec2 texcoord;
	out vec4 col;

	void main(void) {
		gl_Position = vec4(vertex.xy, 0.0, 1.0);
		texcoord = tex_coord;
		col = color;
	});
const static std::string fontShaderFragSrc = SHADER_SRC(
	#version 130\n

	in vec2 texcoord;
in vec4 col;

uniform sampler2D texture;

out vec4 colorOut;

void main(void) {
	colorOut = vec4(col.rgb, texture2D(texture, texcoord).r * col.a);
});

Font::Font()
{
}

Font::~Font()
{
	
}

bool Font::Init(NoWork* framework)
{
	m_Framework = framework;


	return true;
}

void Font::Shutdown()
{
}

NOWORK_API  FontPtr Font::Create(const std::string& path, float size)
{
	FontPtr f = std::make_shared<Font>();

	//setup font cache
	const wchar_t *cache = L" !\"#$%&'()*+,-./0123456789:;<=>?"
		L"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
		L"`abcdefghijklmnopqrstuvwxyz{|}~";

	f->m_Atlas = texture_atlas_new(512, 512, 1);
	f->m_Font = texture_font_new_from_file(f->m_Atlas, size, path.c_str());
	size_t missed = texture_font_load_glyphs(f->m_Font, cache);
	if (missed > 0)
		LOG_WARNING("Fontcache: missed " << missed << " glyphs!");

	f->m_Buffer = vertex_buffer_new("vertex:3f,tex_coord:2f,color:4f");
	f->m_Shader = Shader::Create(fontShaderVertSrc, fontShaderFragSrc);

	return f;
}

void Font::SetSize(int s)
{
	m_Size = s;
}

NOWORK_API void Font::PrintText(const std::string t, glm::vec2 pos, glm::vec4 color /*= glm::vec4(1,1,1,1)*/, glm::vec2 scale /*= glm::vec2(1, 1)*/)
{
	glm::vec2 screenScale = 2.0f / (glm::vec2)m_Framework->ScreenSize();
	screenScale *= scale;
	pos = pos * screenScale;

	size_t i;
	float r = color.r, g = color.g, b = color.b, a = color.a;
	for (i = 0; i < t.size(); ++i)
	{
		texture_glyph_t *glyph = texture_font_get_glyph(m_Font, t[i]);
		if (!glyph) continue;

		float kerning = 0.0f;
		if (i > 0)
		{
			kerning = texture_glyph_get_kerning(glyph, t[i - 1]);
		}
		pos.x += kerning * screenScale.x;
		float x0 = (pos.x + glyph->offset_x * screenScale.x) - 1.0f;
		float y0 = (pos.y + glyph->offset_y * screenScale.y) - 1.0f;
		float x1 = (x0 + glyph->width * screenScale.x);
		float y1 = (y0 - glyph->height * screenScale.y);
		float s0 = glyph->s0;
		float t0 = glyph->t0;
		float s1 = glyph->s1;
		float t1 = glyph->t1;
		GLuint index = m_Buffer->vertices->size;
		GLuint indices[] = { index, index + 1, index + 2,
			index, index + 2, index + 3 };
		vertex_t vertices[] = { { x0, y0, 0, s0, t0, r, g, b, a },
		{ x0, y1, 0, s0, t1, r, g, b, a },
		{ x1, y1, 0, s1, t1, r, g, b, a },
		{ x1, y0, 0, s1, t0, r, g, b, a } };
		vertex_buffer_push_back_indices(m_Buffer, indices, 6);
		vertex_buffer_push_back_vertices(m_Buffer, vertices, 4);
		pos.x += glyph->advance_x * screenScale.x;
	}
}

NOWORK_API void Font::Render()
{
	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_Atlas->id);
	m_Shader->Use();
	m_Shader->SetParameteri("texture", 0);
	vertex_buffer_render(m_Buffer, GL_TRIANGLES);
	vertex_buffer_clear(m_Buffer);
	glEnable(GL_DEPTH_TEST);
}

NOWORK_API void Font::PrintShadowedText(const std::string t, glm::vec2 pos, glm::vec4 color /*= glm::vec4(1, 1, 1, 1)*/, glm::vec2 scale /*= glm::vec2(1, 1)*/, glm::vec2 shadowOffset /*= glm::vec2(1,1)*/)
{
	glm::vec4 shadowColor = color * 0.25f;
	PrintText(t, glm::vec2(pos.x + shadowOffset.x, pos.y - shadowOffset.y), shadowColor, scale);
	PrintText(t, pos, color, scale);
}

NOWORK_API float Font::TextWidth(const std::string t)
{
	float w = 0;
	for (size_t i = 0; i < t.size(); ++i)
	{
		texture_glyph_t *glyph = texture_font_get_glyph(m_Font, t[i]);
		if (!glyph) continue;

		float kerning = 0.0f;
		if (i > 0)
		{
			kerning = texture_glyph_get_kerning(glyph, t[i - 1]);
		}
		w += kerning;
		w += glyph->offset_x + glyph->width + 1;
	}
	return w;
}
