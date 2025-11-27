#pragma once

struct Anim {
    int start = 0;     // frame inicial dentro do sheet
    int count = 1;     // quantos frames
    float fps = 8.f;   // quadros por segundo
    float t = 0.f;     // acumulador interno

    void update(float dt) { t += dt; }
    int frameOffset() const {
        if (count <= 0) return 0;
        int f = int(t * fps) % count;
        if (f < 0) f += count;
        return f;
    }
};
