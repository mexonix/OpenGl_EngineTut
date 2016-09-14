#include "SpriteBatch.h"

#include <algorithm>

namespace MexEngine {

	SpriteBatch::SpriteBatch() :
		_vbo(0),
		_vao(0)
	{
	}


	SpriteBatch::~SpriteBatch()
	{
		//for (size_t i = 0; i < _glyphs.size(); i++)
		//{
		//	delete _glyphs[i];
		//}

	}

	void SpriteBatch::init()
	{
		_createVertexArray();

	}

	void SpriteBatch::begin(GlyphSortType sortType /*= GlyphSortType::TEXTURE*/)
	{
		_sortType = sortType;
		_renderBatches.clear();
		_glyphs.clear();
	}

	void SpriteBatch::end()
	{
		_sortGlyphs();
		_createRenderBatches();
		for (size_t i = 0; i < _glyphs.size(); i++)
		{
			delete _glyphs[i];
		}

	}

	void SpriteBatch::draw(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint texture, float depth, const Color& color)
	{
		Glyph* newGlyph = new Glyph;

		newGlyph->texture = texture;
		newGlyph->depth = depth;

		newGlyph->topLeft.color = color;
		newGlyph->topLeft.setPosition(destRect.x, destRect.y + destRect.w);
		newGlyph->topLeft.setUV(uvRect.x, uvRect.y + uvRect.w);

		newGlyph->bottomLeft.color = color;
		newGlyph->bottomLeft.setPosition(destRect.x, destRect.y);
		newGlyph->bottomLeft.setUV(uvRect.x, uvRect.y);

		newGlyph->topRight.color = color;
		newGlyph->topRight.setPosition(destRect.x + destRect.z, destRect.y + destRect.w);
		newGlyph->topRight.setUV(uvRect.x + uvRect.z, uvRect.y + uvRect.w);

		newGlyph->bottomRight.color = color;
		newGlyph->bottomRight.setPosition(destRect.x + destRect.z, destRect.y);
		newGlyph->bottomRight.setUV(uvRect.x + uvRect.z, uvRect.y );


		_glyphs.push_back(newGlyph);

	}

	void SpriteBatch::renderBatch()
	{
		glBindVertexArray(_vao);
		for (size_t i = 0; i < _renderBatches.size(); i++)
		{
			glBindTexture(GL_TEXTURE_2D, _renderBatches[i].texture);

			glDrawArrays(GL_TRIANGLES, _renderBatches[i].offset, _renderBatches[i].numVertices);
		}
		glBindVertexArray(0);
	}



	void SpriteBatch::_createRenderBatches()
	{
		std::vector<Vertex> vertices;
		vertices.resize(_glyphs.size() * 6);

		if (_glyphs.empty())
		{
			return;
		}

		int offset = 0;
		int currVert = 0;

		_renderBatches.emplace_back(offset, 6, _glyphs[0]->texture);
		vertices[currVert++] = _glyphs[0]->topLeft;
		vertices[currVert++] = _glyphs[0]->bottomLeft;
		vertices[currVert++] = _glyphs[0]->bottomRight;
		vertices[currVert++] = _glyphs[0]->topLeft;
		vertices[currVert++] = _glyphs[0]->topRight;
		vertices[currVert++] = _glyphs[0]->bottomRight;
		offset += 6;

		for (size_t currGlyph = 1; currGlyph < _glyphs.size(); currGlyph++)
		{
			if (_glyphs[currGlyph]->texture != _glyphs[currGlyph - 1]->texture)
			{
				_renderBatches.emplace_back(offset, 6, _glyphs[currGlyph]->texture);
			}
			else
			{
				_renderBatches.back().numVertices += 6;
			}

			_renderBatches.emplace_back(0, 6, _glyphs[currGlyph]->texture);
			vertices[currVert++] = _glyphs[currGlyph]->topLeft;
			vertices[currVert++] = _glyphs[currGlyph]->bottomLeft;
			vertices[currVert++] = _glyphs[currGlyph]->bottomRight;
			vertices[currVert++] = _glyphs[currGlyph]->bottomRight;
			vertices[currVert++] = _glyphs[currGlyph]->topRight;
			vertices[currVert++] = _glyphs[currGlyph]->topLeft;
			offset += 6;
		}

		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		//orphan the buffer
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

		//upload the data
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());

		//unbind the buffer
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}



	void SpriteBatch::_createVertexArray()
	{
		if (_vao == 0)
		{
			glGenVertexArrays(1, &_vao);
		}
		glBindVertexArray(_vao);

		if (_vbo == 0)
		{
			glGenBuffers(1, &_vbo);
		}
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);


		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		//Position attribute pointer
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

		//Color attribute pointer
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));

		//UV attribute pointer
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));


		glBindVertexArray(0);
	}

	void SpriteBatch::_sortGlyphs()
	{
		switch (_sortType)
		{
		case MexEngine::GlyphSortType::FRONT_TO_BACK:
			std::stable_sort(_glyphs.begin(), _glyphs.end(), _compareFrontToBack);
			break;

		case MexEngine::GlyphSortType::BACK_TO_FRONT:
			std::stable_sort(_glyphs.begin(), _glyphs.end(), _compareBackToBack);
			break;

		case MexEngine::GlyphSortType::TEXTURE:
			std::stable_sort(_glyphs.begin(), _glyphs.end(), _compareTexture);
			break;
		}

	}




	bool SpriteBatch::_compareFrontToBack(Glyph* a, Glyph* b)
	{
		return (a->depth < b->depth);
	}

	bool SpriteBatch::_compareBackToBack(Glyph* a, Glyph* b)
	{
		return (a->depth > b->depth);
	}

	bool SpriteBatch::_compareTexture(Glyph* a, Glyph* b)
	{
		return (a->texture < b->texture);
	}


}