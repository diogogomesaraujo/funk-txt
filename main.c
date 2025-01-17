#include <raylib.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define TEXT_SIZE 40.
#define TEXT_COLOR BLACK

Vector2 centerTextPos(char* text);

int main() {
    // SETUP
    const char* title = "funk-txt";

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
    SetTargetFPS(60);

    // TEXT
    char* text = "funk";
    Vector2 textPos = centerTextPos(text);

    // RENDER
    while(!WindowShouldClose()) {
        if(IsKeyPressed(KEY_ESCAPE)) return 0;

        // DRAW
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText(text, textPos.x, textPos.y, TEXT_SIZE, TEXT_COLOR);

        EndDrawing();
    }

    // TERMINATE

    return 0;
}

Vector2 centerTextPos(char* text) {
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, TEXT_SIZE, (float)TEXT_SIZE / 10);

    textSize.x = ((float) SCREEN_WIDTH - textSize.x) / 2;
    textSize.y = ((float) SCREEN_HEIGHT - textSize.y) / 2;

    return textSize;
}
