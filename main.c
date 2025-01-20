#include <math.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define TEXT_SIZE 300.
#define SPACING 0.05

#define CURSOR_RATE 30

#define BACKGROUND_COLOR (Color) {0x1e,0x1e,0x1e,0xff}
#define TEXT_COLOR (Color) RAYWHITE
#define CURSOR_COLOR (Color) {0x6d,0x9d,0x97,0xff}

#define BUF_SIZE 100
#define FILE_NAME "txt"

int count = 0;
bool show = true;

Vector2 centerTextLastCharPos(char* text, float textSize, Font font);
Vector2 centerTextPos(char* text, float textSize, Font font);
float textSizeFromLen(int textLen);
char* textHandler(char* text);
void showCursor(char* cursor, Vector2 cursorPos, float textSize, Font font);
char* readTextFromFile(FILE* f);

int main() {
    // SETUP
    const char* title = "funk-txt";

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
    SetTargetFPS(60);

    // TEXT
    char* text = malloc(sizeof(char) * 1);
    text[0] = '\0';
    float textSize = textSizeFromLen(strlen(text));
    Font font = LoadFontEx("VictorMono-Medium.ttf", 2 * textSize, 0, 250);
    Vector2 textPos = centerTextLastCharPos(text, textSize, font);


    // CURSOR
    char* cursor = "|";
    Vector2 cursorPos = centerTextPos(cursor, textSize, font);

    // FILE
    FILE* f = fopen(FILE_NAME, "r+");

    if (f == NULL) {
        FILE* aux = fopen(FILE_NAME, "w");
        fclose(aux);

        f = fopen(FILE_NAME, "r+");
    }

    text = readTextFromFile(f);

    // RENDER
    while(!WindowShouldClose()) {
        if(IsKeyPressed(KEY_ESCAPE)) return 0;

        // TODO: File System is Terrible
        if(IsKeyDown(KEY_LEFT_CONTROL)) {
            char* fileText;
            if(IsKeyDown(KEY_S) && strcmp(fileText = readTextFromFile(f), text) != 0) {
                free(fileText);
                FILE* aux = fopen(FILE_NAME, "w");
                if (aux != NULL){
                    fprintf(aux, "%s", text);
                    fclose(aux);
                }
            }
            else if(IsKeyDown(KEY_X)) {
                text = realloc(text, sizeof(char));
                text[0] = '\0';
            }
        }

        char*changedText = textHandler(text);
        if(changedText != text) {
            free(text);
            text = changedText;
            show = true;
            count = 0;
        }

        textSize = textSizeFromLen(strlen(text));
        textPos = centerTextLastCharPos(text, textSize, font);

        cursorPos = centerTextPos(cursor, textSize, font);

        // DRAW
        BeginDrawing();

        ClearBackground(BACKGROUND_COLOR);

        DrawTextEx(font, text, (Vector2) {textPos.x, textPos.y}, textSize, textSize * SPACING, TEXT_COLOR);

        showCursor(cursor, cursorPos, textSize, font);

        EndDrawing();
    }

    // TERMINATE

    CloseWindow();
    free(text);
    if (f) fclose(f);
    UnloadFont(font);

    return 0;
}

Vector2 centerTextLastCharPos(char* text, float textSize, Font font) {
    Vector2 size = MeasureTextEx(font, text, textSize, textSize * SPACING);

    size.x = ((float) SCREEN_WIDTH) / 2 - size.x - textSize * SPACING;
    size.y = ((float) SCREEN_HEIGHT - size.y) / 2;

    return size;
}

Vector2 centerTextPos(char* text, float textSize , Font font) {
    Vector2 size = MeasureTextEx(font, text, textSize, textSize * SPACING);

    size.x = ((float) SCREEN_WIDTH - size.x) / 2;
    size.y = ((float) SCREEN_HEIGHT - size.y) / 2;

    return size;
}

float textSizeFromLen(int textLen) {
    if (textLen <= 0) return TEXT_SIZE;
    return TEXT_SIZE * ((float)1 / log2f((float) (textLen + 1)));
}

char* textHandler(char* text) {
    int textLen = strlen(text);
    if(IsKeyPressed(KEY_BACKSPACE) && textLen > 0) {
        char* changedText = malloc(sizeof(char) * (textLen));
        memcpy(changedText, text, textLen - 1);

        changedText[textLen - 1] = '\0';

        return changedText;
    }

    if(IsKeyPressed(KEY_SPACE)) {
        char* changedText = malloc(sizeof(char) * (textLen + 2));
        memcpy(changedText, text, textLen);

        changedText[textLen] = ' ';
        changedText[textLen + 1] = '\0';

        return changedText;
    }

    int key;

    while ((key = GetCharPressed()) != 0) { // wait for char to read the continue!!
        char* changedText = malloc(sizeof(char) * (textLen + 2));
        memcpy(changedText, text, textLen);

        changedText[textLen] = key;

        return changedText;
    }

    return text;
}

void showCursor(char* cursor, Vector2 cursorPos, float textSize, Font font) {
    if (count < CURSOR_RATE && show) {
        DrawTextEx(font, cursor, cursorPos, textSize, textSize * SPACING, CURSOR_COLOR);
        count++;
        return;
    }
    else if (show) {
        show = false;
        count = 0;
        return;
    }
    else if (count < CURSOR_RATE && !show) {
        count++;
        return;
    }
    else {
        show = true;
        count = 0;
        return;
    }
}

char* readTextFromFile(FILE* f) {
    rewind(f); // so it starts from the beggining!!

    char* fileText = NULL;
    int ch;
    int size = 0;
    while((ch = fgetc(f)) != EOF) {
        size++;
        fileText = realloc(fileText, sizeof(char) * size);
        fileText[size - 1] = ch;
    }

    fileText = realloc(fileText, sizeof(char) * (size + 1));
    fileText[size] = '\0';

    return fileText;
}
