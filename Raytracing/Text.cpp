#include "Text.h"
#include "main.h"

#include<glm/gtc/matrix_transform.hpp>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

Text::Text() {
    // Load TTF font
    stbtt_fontinfo font;

    std::vector<unsigned char> fontBuffer;
    std::ifstream file("font/arial.ttf", std::ios::binary);
    fontBuffer = std::vector<unsigned char>(std::istreambuf_iterator<char>(file), {});

    if (!stbtt_InitFont(&font, fontBuffer.data(), 0)) {
        std::cout << "Failed to load font\n";
        return;
    }

    // Generate font atlas
    std::vector<unsigned char> atlas(atlasWidth * atlasHeight);

    stbtt_BakeFontBitmap(fontBuffer.data(), 0, 32.0f, atlas.data(), atlasWidth, atlasHeight, 32, 96, cdata);

    // Upload atlas to OpenGL
    glGenTextures(1, &fontTex);
    glBindTexture(GL_TEXTURE_2D, fontTex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, atlas.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontTex);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Text::draw(Shader* shader, std::string text) {
    float x = 10;
    float y = Main::getScreenSize().y - 25;

    std::vector<float> verts;
    verts.reserve(text.size() * 6 * 4);
    for (char ch : text)
    {
        if (ch < 32 || ch > 126) continue;

        stbtt_bakedchar* b = &cdata[ch - 32];

        float scale = 0.65f;

        float x0 = x + b->xoff * scale;
        float y0 = y - b->yoff * scale;
        float x1 = x0 + (b->x1 - b->x0) * scale;
        float y1 = y0 - (b->y1 - b->y0) * scale;

        float s0 = b->x0 / (float)atlasWidth;
        float t0 = b->y0 / (float)atlasHeight;
        float s1 = b->x1 / (float)atlasWidth;
        float t1 = b->y1 / (float)atlasHeight;

        // Add a quad
        verts.insert(verts.end(), {
            x0, y0, s0, t0,
            x1, y0, s1, t0,
            x1, y1, s1, t1,

            x0, y0, s0, t0,
            x1, y1, s1, t1,
            x0, y1, s0, t1
        });

        x += b->xadvance * scale;
    }

    shader->Activate();
    shader->setInt("tex", 0);

    glm::mat4 proj = glm::ortho(0.f, Main::getScreenSize().x, 0.f, Main::getScreenSize().y, 0.f, 1000.f);
    //glm::mat4 proj = glm::ortho(-1.f, 1.f, -1.f, 1.f, 0.f, 1000.f);
    shader->setMat4("proj", proj);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontTex);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_DYNAMIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, verts.size() / 4);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}