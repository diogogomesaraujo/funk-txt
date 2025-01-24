#include <math.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define TEXT_SIZE 400.
#define SPACING 0.05

#define CURSOR_RATE 30

#define BACKGROUND_COLOR (Color) {0x28,0x28,0x28,0xff}
#define TEXT_COLOR (Color) RAYWHITE
#define CURSOR_COLOR (Color) {0x6d,0x9d,0x97,0xff}

#define BUF_SIZE 100
#define FILE_NAME "txt"

int count = 0;
bool show = true;
Vector2 atChar = {0,0}; //FROM LAST CHAR

Vector2 centerTextLastCharPos(char* text, float textSize, Font font, int lines, int lastLineCount);
Vector2 centerTextPos(char* text, float textSize, Font font);
float textSizeFromLen(int textLen);
char* textHandler(char* text);
void showCursor(char* cursor, Vector2 cursorPos, float textSize, Font font);
char* readTextFromFile(FILE* f);

int main(int argc, char** argv) {
    // SETUP
    const char* fileName = FILE_NAME;

    if (argc > 1) {
        fileName = argv[1];
    }

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, fileName);
    SetTargetFPS(60);

    // TEXT
    char* text = malloc(sizeof(char) * 1);
    text[0] = '\0';
    float textSize = textSizeFromLen(strlen(text));
    Font font = LoadFontEx("VictorMono-Regular.ttf", 2 * textSize, 0, 250);
    Vector2 textPos = centerTextLastCharPos(text, textSize, font, 1, 0);

    // CURSOR
    char* cursor = "|";
    Vector2 cursorPos = centerTextPos(cursor, textSize, font);

    // FILE
    FILE* f = fopen(fileName, "r+");

    if (f == NULL) {
        FILE* aux = fopen(fileName, "w");
        fclose(aux);

        f = fopen(fileName, "r+");
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
                FILE* aux = fopen(fileName, "w");
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

        int lines = 1;
        char *linePointer = text;
        char *lastLinePointer = NULL;

        while ((linePointer = strchr(linePointer, '\n')) != NULL) {
            lastLinePointer = linePointer;

            lines++;
            linePointer++;
        }

        int lastLineCount = 0;

        while(lastLinePointer != NULL && *lastLinePointer != '\0') {
            lastLinePointer++;
            lastLineCount++;
        }

        int textLen = strlen(text);

        if(lastLineCount == 0) lastLineCount = textLen + 1;

        if(IsKeyPressed(KEY_LEFT) && lastLineCount - atChar.x - 1 > 0) atChar.x++;
        if(IsKeyPressed(KEY_RIGHT) && atChar.x > 0) atChar.x--;

        textSize = textSizeFromLen(textLen);
        textPos = centerTextLastCharPos(text, textSize, font, lines, lastLineCount);

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

Vector2 centerTextLastCharPos(char* text, float textSize, Font font, int lines, int lastLineCount) {
    Vector2 size = MeasureTextEx(font, text, textSize, textSize * SPACING);
    Vector2 charSize = MeasureTextEx(font, &text[strlen(text) - 1], textSize, 0);

    float lineWidth = 0;
    float lineHeight = size.y / lines;

    if (charSize.y == size.y) lineWidth = size.x;
    if(lastLineCount != 0) lineWidth = (lastLineCount - 1 - atChar.x) * charSize.x + (lastLineCount - 2 - atChar.x) * textSize * SPACING;

    size.x = ((float) SCREEN_WIDTH) / 2 - lineWidth - textSize * SPACING;
    size.y = ((float) SCREEN_HEIGHT - lineHeight) / 2 - lineHeight * (lines - 1 - atChar.y);

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
        atChar.x = 0;
        atChar.y = 0;

        char* changedText = malloc(sizeof(char) * (textLen));
        memcpy(changedText, text, textLen - 1);

        changedText[textLen - 1] = '\0';

        return changedText;
    }

    if(IsKeyPressed(KEY_SPACE)) {
        atChar.x = 0;
        atChar.y = 0;

        char* changedText = malloc(sizeof(char) * (textLen + 2));
        memcpy(changedText, text, textLen);

        changedText[textLen] = ' ';
        changedText[textLen + 1] = '\0';

        return changedText;
    }

    if(IsKeyPressed(KEY_TAB)) {
        atChar.x = 0;
        atChar.y = 0;

        char* changedText = malloc(sizeof(char) * (textLen + 5));
        memcpy(changedText, text, textLen);

        for(int i = textLen; i < textLen + 4; i++) {
            changedText[i] = ' ';
        }
        changedText[textLen] = ' ';
        changedText[textLen + 5] = '\0';

        return changedText;
    }

    if(IsKeyPressed(KEY_ENTER)) {
        atChar.x = 0;
        atChar.y = 0;

        char* changedText = malloc(sizeof(char) * (textLen + 2));
        memcpy(changedText, text, textLen);

        changedText[textLen] = '\n';
        changedText[textLen + 1] = '\0';

        return changedText;
    }

    int key;

    while ((key = GetCharPressed()) != 0) { // wait for char to read the continue!!
        if(atChar.x == 0) {
            char* changedText = malloc(sizeof(char) * (textLen + 2));
            strncpy(changedText, text, textLen);

            changedText[textLen] = key;

            atChar.x = 0;

            return changedText;
        } else {
            char* changedText = malloc(sizeof(char) * (textLen + 2));
            strncpy(changedText, text, textLen - atChar.x);

            changedText[textLen - (int) atChar.x] = key;

            strncpy(&changedText[textLen - (int) atChar.x + 1], &text[textLen - (int) atChar.x], atChar.x + 1);

            return changedText;
        }
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
