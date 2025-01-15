#include <raylib.h>
#include <stdlib.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

Vector2 textSize(char* text, float fontSize);

int main()
{
    // SETUP
    const char* title = "funk-txt";

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
    SetTargetFPS(60);

    // TEXT
    int textRowSize = 0;
    int textColSize = 0;
    char* text = malloc(sizeof(char) * 0); // to be cols * rows

    // RENDER
    while(!WindowShouldClose())
    {
        if(IsKeyPressed(KEY_ESCAPE)) return 0;

        // DRAW
        BeginDrawing();

        ClearBackground(RAYWHITE);

        EndDrawing();
    }

    // TERMINATE

    return 0;
}

Vector2 textSize(char* text, float fontSize) {
    return MeasureTextEx(GetFontDefault(), text, fontSize, fontSize / 10);
}
