#include <math.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define TEXT_SIZE 400.
#define SPACING 0.05

#define CURSOR_RATE 30

#define BACKGROUND_COLOR (Color) {0x28,0x28,0x28,0xff}
#define TEXT_COLOR (Color) {0xef,0xf1,0xf5,0xff}
#define CURSOR_COLOR (Color) {0x6d,0x9d,0x97,0xff}

#define BUF_SIZE 100
#define FILE_NAME "txt"

#define SMOOTHING 0.95f

char* text;
int count = 0;
bool show = true;
float atChar = 0; //FROM LAST CHAR
int lines;
int lastLineCount;
int screenWidth = SCREEN_WIDTH;
int screenHeight = SCREEN_HEIGHT;

Vector2 centerTextLastCharPos(char* text, float textSize, Font font, int lines, int lastLineCount);
Vector2 centerCursorPos(char* cursor, float textSize, Font font, float offsetX, float offsetY);
float textSizeFromLen(int textLen);
char* textHandler(char* text);
void showCursor(char* cursor, Vector2 cursorPos, float textSize, Font font);
void* readTextFromFile(void* f);
void* controlOperations(void* fileName);
void* handleLinesAndCount();
float lerp(float a, float b, float f);
float smoothing(float a, float b, float s);

int main(int argc, char** argv) {
    // SETUP
    const char* fileName = FILE_NAME;

    if (argc > 1) {
        fileName = argv[1];
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, fileName);
    SetTargetFPS(60);

    // FILE
    FILE* f = fopen(fileName, "r+");

    if (f == NULL) {
        FILE* aux = fopen(fileName, "w");
        fclose(aux);

        f = fopen(fileName, "r+");
    }

    pthread_t readFile;

    pthread_create(&readFile, NULL, readTextFromFile, f);
    pthread_join(readFile, NULL);

    // TEXT
    float textSize = textSizeFromLen(strlen(text));

    Font font = LoadFontEx("VictorMono-Regular.ttf", TEXT_SIZE, 0, 250);
    Vector2 textPos = centerTextLastCharPos(text, textSize, font, 1, 0);

    // CURSOR
    char* cursor = "|";
    Vector2 cursorPos = centerCursorPos(cursor, textSize, font, 0, 0);

    // RENDER
    while(!WindowShouldClose()) {
        if(IsKeyPressed(KEY_ESCAPE)) return 0;

        if (screenWidth != GetScreenWidth() || screenHeight != GetScreenHeight()) {
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight();
        }

        if(IsKeyDown(KEY_LEFT_CONTROL)) {
            pthread_t saveFile;
            pthread_create(&saveFile, NULL, controlOperations, (char*)fileName);
            pthread_join(saveFile, NULL);
        }

        char* changedText = textHandler(text);
        if(changedText != text) {
            free(text);
            text = changedText;
            show = true;
            count = 0;
        }

        pthread_t handleText;

        pthread_create(&handleText, NULL, handleLinesAndCount, NULL);
        pthread_join(handleText, NULL);

        int textLen = strlen(text);

        if(lastLineCount == 0) lastLineCount = textLen + 1;

        if(IsKeyPressed(KEY_LEFT) && lastLineCount - atChar - 1 > 0) atChar++;
        if(IsKeyPressed(KEY_RIGHT) && atChar > 0) atChar--;

        textSize = smoothing(textSizeFromLen(textLen), textSize, SMOOTHING);
        Vector2 auxText = centerTextLastCharPos(text, textSize, font, lines, lastLineCount);
        Vector2 auxCursor1 = centerCursorPos(cursor, textSize, font, 0, 0);

        if(textLen == 0) {
            textPos = (Vector2){smoothing(203.000275f, textPos.x, SMOOTHING),smoothing(100.000206f, textPos.y, SMOOTHING)};
            cursorPos = (Vector2){smoothing(auxCursor1.x, cursorPos.x, SMOOTHING), smoothing(auxCursor1.y, cursorPos.y, SMOOTHING)};

        }
        else {
            textPos = (Vector2){smoothing(auxText.x, textPos.x, SMOOTHING), smoothing(auxText.y, textPos.y, SMOOTHING)};
            cursorPos = centerCursorPos(cursor, textSize, font, auxText.x - textPos.x, auxText.y - textPos.y);

        }

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

float lerp(float a, float b, float f) {
    return a * (1.0 - f) + (b * f);
}

float smoothing(float a, float b, float s) {
    return a + (b - a) * s;
}

Vector2 centerTextLastCharPos(char* text, float textSize, Font font, int lines, int lastLineCount) {
    Vector2 size = MeasureTextEx(font, text, textSize, textSize * SPACING);
    Vector2 charSize = MeasureTextEx(font, &text[strlen(text) - 1], textSize, 0);

    float lineWidth = 0;
    float lineHeight = size.y / lines;

    if (charSize.y == size.y) lineWidth = size.x;
    if(lastLineCount != 0) lineWidth = (lastLineCount - 1 - atChar) * charSize.x + (lastLineCount - 2 - atChar) * textSize * SPACING;

    size.x = ((float) screenWidth) / 2 - lineWidth - textSize * SPACING;
    size.y = ((float) screenHeight - lineHeight) / 2 - lineHeight * (lines - 1);

    return size;
}

Vector2 centerCursorPos(char* cursor, float textSize , Font font, float offsetX, float offsetY) {
    Vector2 size = MeasureTextEx(font, cursor, textSize, textSize * SPACING);

    size.x = ((float) screenWidth - size.x) / 2 - offsetX;
    size.y = ((float) screenHeight - size.y) / 2 - offsetY;

    return size;
}

float textSizeFromLen(int textLen) {
    if (textLen <= 0) return TEXT_SIZE;
    return TEXT_SIZE * ((float)1 / log2f((float) (textLen + 1)));
}

char* textHandler(char* text) {
    int textLen = strlen(text);
    if(IsKeyPressed(KEY_BACKSPACE) && textLen - atChar > 0) {
        if(atChar == 0) {
            atChar = 0;

            char* changedText = malloc(sizeof(char) * (textLen));
            memcpy(changedText, text, textLen - 1);

            changedText[textLen - 1] = '\0';

            return changedText;
        } else {
            char* changedText = malloc(sizeof(char) * (textLen));
            strncpy(changedText, text, textLen - atChar - 1);

            strncpy(&changedText[textLen - (int) atChar - 1], &text[textLen - (int) atChar], atChar + 1);

            return changedText;
        }
    }

    if(IsKeyPressed(KEY_SPACE)) {
        if(atChar == 0) {
            char* changedText = malloc(sizeof(char) * (textLen + 2));
            strncpy(changedText, text, textLen);

            changedText[textLen] = ' ';

            atChar = 0;

            return changedText;
        } else {
            char* changedText = malloc(sizeof(char) * (textLen + 2));
            strncpy(changedText, text, textLen - atChar);

            changedText[textLen - (int) atChar] = ' ';

            strncpy(&changedText[textLen - (int) atChar + 1], &text[textLen - (int) atChar], atChar + 1);

            return changedText;
        }
    }

    if(IsKeyPressed(KEY_TAB)) {
        if(atChar == 0) {
            char* changedText = malloc(sizeof(char) * (textLen + 5));
            strncpy(changedText, text, textLen);

            for(int i = textLen; i < textLen + 4; i++) {
                changedText[i] = ' ';
            }

            atChar = 0;

            return changedText;
        } else {
            char* changedText = malloc(sizeof(char) * (textLen + 5));
            strncpy(changedText, text, textLen - atChar);

            for(int i = textLen - atChar; i < textLen + 4; i++) {
                changedText[i] = ' ';
            }

            strncpy(&changedText[textLen - (int) atChar + 4], &text[textLen - (int) atChar], atChar + 1);

            return changedText;
        }
    }

    if(IsKeyPressed(KEY_ENTER)) {
        if(atChar == 0) {
            char* changedText = malloc(sizeof(char) * (textLen + 2));
            strncpy(changedText, text, textLen);

            changedText[textLen] = '\n';

            atChar = 0;

            return changedText;
        } else {
            char* changedText = malloc(sizeof(char) * (textLen + 2));
            strncpy(changedText, text, textLen - atChar);

            changedText[textLen - (int) atChar] = '\n';

            strncpy(&changedText[textLen - (int) atChar + 1], &text[textLen - (int) atChar], atChar + 1);

            return changedText;
        }
    }

    int key;

    while ((key = GetCharPressed()) != 0) { // wait for char to read the continue!!
        if(atChar == 0) {
            char* changedText = malloc(sizeof(char) * (textLen + 2));
            strncpy(changedText, text, textLen);

            changedText[textLen] = key;
            changedText[textLen + 1] = '\0';

            atChar = 0;

            return changedText;
        } else {
            char* changedText = malloc(sizeof(char) * (textLen + 2));
            strncpy(changedText, text, textLen - atChar);

            changedText[textLen - (int) atChar] = key;

            strncpy(&changedText[textLen - (int) atChar + 1], &text[textLen - (int) atChar], atChar + 1);
            changedText[strlen(changedText)] = '\0';

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

void* readTextFromFile(void* f) {
    f = (FILE*) f;
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

    text = fileText;

    pthread_exit(NULL);
}

void* controlOperations(void* fileName) {
    fileName = (char*) fileName;
    if(IsKeyPressed(KEY_S)) {
        FILE* aux = fopen(fileName, "w");
        if (aux != NULL){
            fprintf(aux, "%s", text);
            fclose(aux);
        }
    }
    else if(IsKeyDown(KEY_X)) {
        text = realloc(text, sizeof(char));
        text[0] = '\0';
        atChar = 0;
    }

    pthread_exit(NULL);
}

void* handleLinesAndCount() {
    lines = 1;
    char *linePointer = text;
    char *lastLinePointer = NULL;

    while ((linePointer = strchr(linePointer, '\n')) != NULL) {
        lastLinePointer = linePointer;

        lines++;
        linePointer++;
    }

    lastLineCount = 0;

    while(lastLinePointer != NULL && *lastLinePointer != '\0') {
        lastLinePointer++;
        lastLineCount++;
    }

    pthread_exit(NULL);
}
