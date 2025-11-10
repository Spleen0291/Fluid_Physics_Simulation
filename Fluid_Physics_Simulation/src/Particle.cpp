#include "../HeaderFiles/Particle.h"

#define M_PI 3.1415926535897932384626433832f

//Defining static members
std::vector <float> Particle::positions;
std::vector <unsigned int> Particle::indices;
std::vector <Particle> Particle::particles;
int size = 2.0f / Particle::s_Radius;
std::vector <std::vector <std::unordered_map<int, bool>>> Particle::cells(size, std::vector <std::unordered_map<int, bool>> (size));
unsigned int Particle::vao = 0;
unsigned int Particle::vbo = 0;
unsigned int Particle::ibo = 0;

void checkBoundary(Particle& p) {
    float r = Particle::radius;
    // check left
    if (p.pos.x < -0.9f + r) p.pos.x = -0.9f + r, p.velocity.x = -p.velocity.x * 0.5f;

    // check right
    if (p.pos.x > 0.9f - r) p.pos.x = 0.9f  - r, p.velocity.x = -p.velocity.x * 0.5f;

    // check top
    if (p.pos.y > 0.9f - r) p.pos.y = 0.9f - r, p.velocity.y = -p.velocity.y * 0.5f;

    //check bottom
    if (p.pos.y < -0.9f + r) p.pos.y = -0.9f + r, p.velocity.y = -p.velocity.y * 0.5f;
}

void Particle::generateRandomCenters() {
    for (int i = 0; i < numOfParticles; i++) {
        Particle::centers.push_back(glm::linearRand(-0.9f + Particle::radius, 0.9f - Particle::radius));
        Particle::centers.push_back(glm::linearRand(-0.9f + Particle::radius, 0.9f - Particle::radius));
    }
}

void Particle::generateGridCenters(int rows, int cols) {
    float left = 0.0f - (2 * Particle::radius + Particle::spacing) * cols / 2.0f;
    float top = 0.9f - (Particle::spacing + Particle::radius);
    for (int i = 0; i < rows; i++) {
        
        for (int j = 0; j < cols; j++) {
            Particle::centers.push_back(left + j * (2 * Particle::radius + Particle::spacing));
            Particle::centers.push_back(top);
        }
        top -= 2 * Particle::radius + Particle::spacing;
    }
}

void Particle::generateParticle(float aspectRatio) {

    positions.push_back(0.0f);
    positions.push_back(0.0f);

    int startingIndex = (int)positions.size() / 2;

    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * M_PI * (float)i / (float)segments;
        float x = radius * cosf(theta);
        float y = radius * sinf(theta);
        positions.push_back((x) / aspectRatio);
        positions.push_back(y);

        if (i == 0) continue;

        indices.push_back(startingIndex);
        indices.push_back(startingIndex + i);
        indices.push_back(startingIndex + i + 1);
    }
}

void Particle::populate(float aspectRatio) {
    // generating Centers
    for (int i = 0; i < centers.size(); i += 2) {
        Particle p;
        p.velocity = glm::vec3(0.0f);
        p.acceleration = glm::vec3(0.0f);
        p.pos = glm::vec3(centers[i], centers[i + 1], 0.0f);
        p.density = 0.0f;
        p.generateParticle(aspectRatio);
        particles.push_back(p);

        // populating cells
        int x = (p.pos.x + 1.0f) / s_Radius;
        int y = (p.pos.y + 1.0f) / s_Radius;
        cells[x][y][i/2] = true;
    }
}

void Particle::updateCell(int idx, int prevX, int prevY) {
    cells[prevX][prevY][idx] = false;
    int x = (particles[idx].pos.x + 1.0f) / s_Radius;
    int y = (particles[idx].pos.y + 1.0f) / s_Radius;
    cells[x][y][idx] = true;
}

std::vector<Particle> Particle::findNeighbors(int idx) {
    Particle& p = particles[idx];
    int cellX = (p.pos.x + 1.0f) / s_Radius;
    int cellY = (p.pos.y + 1.0f) / s_Radius;
    std::vector <Particle> neighborsOut;
    for (int i = -1; i <= 1; i++) {
        if (cellX + i < 0 || cellX + i > size - 1) continue;
        for (int j = -1; j <= 1; j++) {
            if (cellY + j < 0 || cellY + j > size - 1) continue;
            for (std::pair<int, bool> neighbor : cells[cellX + i][cellY + j]) {
                if (neighbor.first != idx && neighbor.second) neighborsOut.push_back(particles[neighbor.first]);
            }
        }
    }
    return neighborsOut;
}

float Particle::densityKernel(float dst) {
    if (dst >= s_Radius) return 0;
    float scale = 4.0f / (M_PI * std::powf(s_Radius, 8.0f));
    float val = s_Radius * s_Radius - dst * dst;
    return val * val * val * scale;
}

float Particle::nearDensityKernel(float dst) {
    if (dst >= s_Radius) return 0;
    float val = 1 - dst / s_Radius;
    return val * val * val;
}

float Particle::pressureKernel(float dst) {
    if (dst >= s_Radius) return 0;
    float scale = -30.0f / (M_PI * std::powf(s_Radius, 5.0f));
    float val = s_Radius - dst;
    return val * val * scale;
}

float Particle::nearPressureKernel(float dst) {
    if (dst >= s_Radius) return 0;
    float scale = -3.0f / s_Radius;
    float val = 1 - dst / s_Radius;
    return val * val * scale;
}

float Particle::viscosityKernel(float dst) {
    if (dst >= s_Radius) return 0;
    float scale = 40.0f / (M_PI * std::powf(s_Radius, 5.0f));
    float val = s_Radius - dst;
    return val * scale;
}

glm::vec3 Particle::pressure(int idx) {
    glm::vec3 force = glm::vec3(0.0f);
    std::vector <Particle> neighbors = findNeighbors(idx);
    for (int i = 0; i < neighbors.size(); ++i) {
        float dst = glm::length(neighbors[i].pos - particles[idx].pos);
        if (dst < 1e-6f) continue;
        glm::vec3 dir = (neighbors[i].pos - particles[idx].pos) / dst;
        float dens = std::max(neighbors[i].density, 1e-4f);

        float influence = pressureKernel(dst);
        float nearInfluence = nearPressureKernel(dst);

        float pressureA = (neighbors[i].density - targetDensity) * pressureMultiplier;
        float pressureB = (particles[idx].density - targetDensity) * pressureMultiplier;

        float nearPressure = neighbors[i].nearDensity * nearPressureMultiplier;

        float sharedPressure = influence * (pressureA + pressureB) / (2.0f * dens);
        sharedPressure += nearInfluence * nearPressure;
        force += dir * sharedPressure;
    }

    return force + viscosity(idx, neighbors);
}

void Particle::calcuateDensities(int idx) {
    float density = 0.0f;
    float nearDensity = 0.0f;
    Particle& p = particles[idx];
    std::vector <Particle> neighbors = findNeighbors(idx);
    for (int i = 0; i < neighbors.size(); i++) {
        if (i == idx) continue;
        float dst = glm::length(neighbors[i].predictedPos - particles[idx].predictedPos);
        density += densityKernel(dst);
        nearDensity += nearDensityKernel(dst);
    }
    p.density = density;
    p.nearDensity = nearDensity;
}

glm::vec3 Particle::viscosity(int idx, std::vector<Particle> neighbors) {
    glm::vec3 force = glm::vec3(0.0f);
    for (int i = 0; i < neighbors.size(); ++i) {
        float dst = glm::length(neighbors[i].pos - particles[idx].pos);
        if (dst < 1e-6f) continue;
        glm::vec3 dir = (neighbors[i].pos - particles[idx].pos) / dst;
        float influence = viscosityKernel(dst);
        force += (neighbors[i].velocity - particles[idx].velocity) * influence;
    }
    return force * viscosityMultiplier * particles[idx].density;
}

glm::vec3 velToColor(Particle p) {
    float speed = glm::length(p.velocity);
    float scale = speed / 15.0f;
    glm::vec3 color = glm::vec3(0.0f);
    color.r = scale;
    color.g = 1.0f - std::abs(scale - 0.5f);
    color.b = 1.0f - scale;
    
    return color;
}

void Particle::drawElements(Window window, int object_Location, int color_Location, bool bDraw) {
    if (bDraw)
    {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 2 * centers.size() / 2 * (segments + 2) * sizeof(float), positions.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * centers.size() / 2 * (segments + 2) * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
        glEnableVertexAttribArray(0);

        // Draw Loop
        for (int i = 0; i < particles.size(); ++i) {
            Particle& p = particles[i];
            glm::vec3 translate = glm::vec3(0.0f);
            glm::vec3 color = velToColor(p);

            glUniform4f(object_Location, p.pos.x+translate.x ,p.pos.y+translate.y , translate.z, 0.0f);
            glUniform3f(color_Location, color.r, color.g, color.b);

            glDrawElements(GL_TRIANGLES, 3 * segments, GL_UNSIGNED_INT, (void*)(i * 3 * segments * sizeof(unsigned int)));
        }
    }

    // change position and cell
    for (int i = 0; i < particles.size(); ++i) {
        Particle& p = particles[i];
        int x = (p.pos.x + 1.0f) / s_Radius;
        int y = (p.pos.y + 1.0f) / s_Radius;
        p.pos += stepSize * p.velocity;
        checkBoundary(p);
        updateCell(i, x, y);
    }

    // predict positions for density calculations
    for (int i = 0; i < particles.size(); ++i) {
        Particle& p = particles[i];
        p.predictedPos = p.pos + stepSize * p.velocity;
    }
    
    // calculate densities
    for (int i = 0; i < particles.size(); ++i) calcuateDensities(i);

    // apply pressure force
    for (int i = 0; i < particles.size(); ++i) {
        Particle& p = particles[i];
        float dens = std::max(particles[i].density, 1e-4f);
        p.acceleration = pressure(i) / dens;
        p.acceleration.y -= 200.0f;
        p.velocity += stepSize * p.acceleration;
        float velMag = glm::length(p.velocity);
        // velocity clamp
        if (velMag > 15.0f) p.velocity = 15.0f * p.velocity / velMag;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

