#pragma once

struct Anim {
    int start = 0;
    int count = 1;
    float fps = 8.f;
    float t = 0.f;

    void update(float dt) { t += dt; }

    int frameOffset() const {
        if (count <= 0) return 0;
        int f = int(t * fps) % count;
        if (f < 0) f += count;
        return f;
    }
};
