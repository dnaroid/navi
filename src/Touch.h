#ifndef TOUCH_H
#define TOUCH_H

#include "NS2009.h"

struct Pos {
    int x;
    int y;
    int dx;
    int dy;
};


class Touch {
public:
    Touch() : lastTouch({-1, -1}),
              currentTouch({-1, -1}),
              onClickCallback(nullptr),
              onDragCallback(nullptr),
              touched(false),
              dragging(false) {
    }

    bool init() {
        return initTouch();
    }

    void update() {
        int touch_pos[2] = {0, 0};
        if (ns2009_pos(touch_pos)) {
            // Touch detected
            currentTouch = {touch_pos[0], touch_pos[1], touch_pos[0] - lastTouch.x, touch_pos[1] - lastTouch.y};

            if (!touched) { // First touch detected
                touched = true;
                lastTouch = currentTouch;
            } else { // Continuing touch

                if (abs(currentTouch.dx) > DRAG_THRESHOLD || abs(currentTouch.dy) > DRAG_THRESHOLD) { // Detected dragging motion
                    dragging = true;
                    if (onDragCallback) onDragCallback(currentTouch);
                } else {
                    delay(100);
                }
                lastTouch = currentTouch;
            }
        } else { // No touch detected
            if (touched) { // Touch release detected
                if (!dragging && onClickCallback) onClickCallback(lastTouch);
                touched = false;
            }
            dragging = false;
        }
    }

    void onClick(const std::function<void(Pos&)>& callback) {
        onClickCallback = callback;
    }

    void onDrag(const std::function<void(Pos&)>& callback) {
        onDragCallback = callback;
    }

private:
    Pos lastTouch;
    Pos currentTouch;
    std::function<void(Pos&)> onClickCallback;
    std::function<void(Pos&)> onDragCallback;
    bool touched;
    bool dragging;

    static constexpr int DRAG_THRESHOLD = 20;
};
#endif //TOUCH_H
