#include <math.h>
#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define TEXT_SIZE 300.
#define TEXT_COLOR RAYWHITE

Vector2 centerTextPos(char* text, float textSize);
float textSizeFromLen(int textLen);
char* textHandler(char* text);

int main() {
    // SETUP
    const char* title = "funk-txt";

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
    SetTargetFPS(60);

    // TEXT
    char* text = malloc(sizeof(char) * 1);
    text[0] = '\0';
    float textSize = textSizeFromLen(strlen(text));
    Vector2 textPos = centerTextPos(text, textSize);

    // RENDER
    while(!WindowShouldClose()) {
        if(IsKeyPressed(KEY_ESCAPE)) return 0;

        char*changedText = textHandler(text);
        if(changedText != text) {
            free(text);
            text = changedText;
        }

        textSize = textSizeFromLen(strlen(text));
        textPos = centerTextPos(text, textSize);

        // DRAW
        BeginDrawing();

        ClearBackground(BLACK);

        DrawText(text, textPos.x, textPos.y, textSize, TEXT_COLOR);

        EndDrawing();
    }

    // TERMINATE

    free(text);

    return 0;
}

Vector2 centerTextPos(char* text, float textSize) {
    Vector2 size = MeasureTextEx(GetFontDefault(), text, textSize, textSize / 10);

    char* lastChar = malloc(sizeof(char));
    lastChar[0] = text[strlen(text) - 1];

    size.x = ((float) SCREEN_WIDTH + MeasureTextEx(GetFontDefault(), lastChar, textSize, textSize / 10).x) / 2 - size.x;
    size.y = ((float) SCREEN_HEIGHT - size.y) / 2;

    free(lastChar);
    return size;
}

float textSizeFromLen(int textLen) {
    return TEXT_SIZE * ((float)1 / log2f((float) (textLen + 1)));
}

char* textHandler(char* text) {
    int textLen = strlen(text);
    if(IsKeyPressed(KEY_BACKSPACE) && textLen > 0) {
        char* changedText = malloc(sizeof(char) * (textLen));
        memcpy(changedText, text, textLen - 1);
        changedText[textLen - 1] = '\0';

        printf("\n\n %s \n\n", changedText);

        return changedText;
    }

    if (GetCharPressed() != 0) {
        char* changedText = malloc(sizeof(char) * (textLen + 2));
        memcpy(changedText, text, textLen);

        changedText[textLen] = GetKeyPressed();
        changedText[textLen + 1] = '\0';

        printf("\n\n %s \n\n", changedText);

        return changedText;
    }

    return text;
}
