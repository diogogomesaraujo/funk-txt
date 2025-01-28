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

typedef struct {
    char* str;
    int len;
    Vector2 pos;
    float font_size;
    int line_count;
}Text;

typedef struct {
    const char* str;
    int abs_pos;
} Cursor;

typedef struct {
    char* start;
    char* end;
    int len;
} Line;

Text text;
int count = 0;
bool show = true;
float atChar = 0; //FROM LAST CHAR
int lastLineCount;
int screenWidth = SCREEN_WIDTH;
int screenHeight = SCREEN_HEIGHT;

Vector2 centerTextLastCharPos(Text text, Font font, int lastLineCount);
Vector2 centerCursorPos(char* cursor, float textSize, Font font, float offsetX, float offsetY);
float textSizeFromLen(int textLen);
char* textHandler(Text text);
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

    readTextFromFile(f);

    // TEXT
    Font font = LoadFontEx("VictorMono-Regular.ttf", TEXT_SIZE, 0, 250);

    text.font_size = textSizeFromLen(text.len);
    text.line_count = 1;
    text.pos = centerTextLastCharPos(text, font, 0);

    // CURSOR
    char* cursor = "|";
    Vector2 cursorPos = centerCursorPos(cursor, text.font_size, font, 0, 0);

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
        if(changedText != text.str) {
            free(text.str);
            text.str = changedText;
            show = true;
            count = 0;
        }

        pthread_t handleText;

        pthread_create(&handleText, NULL, handleLinesAndCount, NULL);
        pthread_join(handleText, NULL);

        text.len = strlen(text.str);

        if(lastLineCount == 0) lastLineCount = text.len + 1;

        if(IsKeyPressed(KEY_LEFT) && lastLineCount - atChar - 1 > 0) atChar++;
        if(IsKeyPressed(KEY_RIGHT) && atChar > 0) atChar--;

        text.font_size = smoothing(textSizeFromLen(text.len), text.font_size, SMOOTHING);
        Vector2 auxText = centerTextLastCharPos(text, font, lastLineCount);
        Vector2 auxCursor1 = centerCursorPos(cursor, text.font_size, font, 0, 0);

        if(text.len == 0) {
            text.pos = (Vector2){smoothing(203.000275f, text.pos.x, SMOOTHING),smoothing(100.000206f, text.pos.y, SMOOTHING)};
            cursorPos = (Vector2){smoothing(auxCursor1.x, cursorPos.x, SMOOTHING), smoothing(auxCursor1.y, cursorPos.y, SMOOTHING)};

        }
        else {
            text.pos = (Vector2){smoothing(auxText.x, text.pos.x, SMOOTHING), smoothing(auxText.y, text.pos.y, SMOOTHING)};
            cursorPos = centerCursorPos(cursor, text.font_size, font, auxText.x - text.pos.x, auxText.y - text.pos.y);

        }

        // DRAW
        BeginDrawing();

        ClearBackground(BACKGROUND_COLOR);

        DrawTextEx(font, text.str, (Vector2) {text.pos.x, text.pos.y}, text.font_size, text.font_size * SPACING, TEXT_COLOR);

        showCursor(cursor, cursorPos, text.font_size, font);

        EndDrawing();
    }

    // TERMINATE

    CloseWindow();
    free(text.str);
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

Vector2 centerTextLastCharPos(Text text, Font font, int lastLineCount) {
    Vector2 size = MeasureTextEx(font, text.str, text.font_size, text.font_size * SPACING);
    Vector2 charSize = MeasureTextEx(font, &text.str[text.len - 1], text.font_size, 0);

    float lineWidth = 0;
    float lineHeight = size.y / text.line_count;

    if (charSize.y == size.y) lineWidth = size.x;
    if(lastLineCount != 0) lineWidth = (lastLineCount - 1 - atChar) * charSize.x + (lastLineCount - 2 - atChar) * text.font_size * SPACING;

    size.x = ((float) screenWidth) / 2 - lineWidth - text.font_size * SPACING;
    size.y = ((float) screenHeight - lineHeight) / 2 - lineHeight * (text.line_count - 1);

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

char* textHandler(Text text) {
    if(IsKeyPressed(KEY_BACKSPACE) && text.len - atChar > 0) {
        if(atChar == 0) {
            atChar = 0;

            char* changedText = malloc(sizeof(char) * (text.len));
            memcpy(changedText, text.str, text.len - 1);

            changedText[text.len - 1] = '\0';

            return changedText;
        } else {
            char* changedText = malloc(sizeof(char) * (text.len));
            strncpy(changedText, text.str, text.len - atChar - 1);

            strncpy(&changedText[text.len - (int) atChar - 1], &text.str[text.len - (int) atChar], atChar + 1);

            return changedText;
        }
    }

    if(IsKeyPressed(KEY_SPACE)) {
        if(atChar == 0) {
            char* changedText = malloc(sizeof(char) * (text.len + 2));
            strncpy(changedText, text.str, text.len);

            changedText[text.len] = ' ';

            atChar = 0;

            return changedText;
        } else {
            char* changedText = malloc(sizeof(char) * (text.len + 2));
            strncpy(changedText, text.str, text.len - atChar);

            changedText[text.len - (int) atChar] = ' ';

            strncpy(&changedText[text.len - (int) atChar + 1], &text.str[text.len - (int) atChar], atChar + 1);

            return changedText;
        }
    }

    if(IsKeyPressed(KEY_TAB)) {
        if(atChar == 0) {
            char* changedText = malloc(sizeof(char) * (text.len + 5));
            strncpy(changedText, text.str, text.len);

            for(int i = text.len; i < text.len + 4; i++) {
                changedText[i] = ' ';
            }

            atChar = 0;

            return changedText;
        } else {
            char* changedText = malloc(sizeof(char) * (text.len + 5));
            strncpy(changedText, text.str, text.len - atChar);

            for(int i = text.len - atChar; i < text.len + 4; i++) {
                changedText[i] = ' ';
            }

            strncpy(&changedText[text.len - (int) atChar + 4], &text.str[text.len - (int) atChar], atChar + 1);

            return changedText;
        }
    }

    if(IsKeyPressed(KEY_ENTER)) {
        if(atChar == 0) {
            char* changedText = malloc(sizeof(char) * (text.len + 2));
            strncpy(changedText, text.str, text.len);

            changedText[text.len] = '\n';

            atChar = 0;

            return changedText;
        } else {
            char* changedText = malloc(sizeof(char) * (text.len + 2));
            strncpy(changedText, text.str, text.len - atChar);

            changedText[text.len - (int) atChar] = '\n';

            strncpy(&changedText[text.len - (int) atChar + 1], &text.str[text.len - (int) atChar], atChar + 1);

            return changedText;
        }
    }

    int key;

    while ((key = GetCharPressed()) != 0) { // wait for char to read the continue!!
        if(atChar == 0) {
            char* changedText = malloc(sizeof(char) * (text.len + 2));
            strncpy(changedText, text.str, text.len);

            changedText[text.len] = key;
            changedText[text.len + 1] = '\0';

            atChar = 0;

            return changedText;
        } else {
            char* changedText = malloc(sizeof(char) * (text.len + 2));
            strncpy(changedText, text.str, text.len - atChar);

            changedText[text.len - (int) atChar] = key;

            strncpy(&changedText[text.len - (int) atChar + 1], &text.str[text.len - (int) atChar], atChar + 1);
            changedText[strlen(changedText)] = '\0';

            return changedText;
        }
    }

    return text.str;
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

    text.str = fileText;
    text.len = strlen(fileText);

    return NULL;
}

void* controlOperations(void* fileName) {
    fileName = (char*) fileName;
    if(IsKeyPressed(KEY_S)) {
        FILE* aux = fopen(fileName, "w");
        if (aux != NULL){
            fprintf(aux, "%s", text.str);
            fclose(aux);
        }
    }
    else if(IsKeyDown(KEY_X)) {
        text.str = realloc(text.str, sizeof(char));
        text.str[0] = '\0';
        atChar = 0;
    }

    pthread_exit(NULL);
}

void* handleLinesAndCount() {
    text.line_count = 1;
    char *linePointer = text.str;
    char *lastLinePointer = NULL;

    while ((linePointer = strchr(linePointer, '\n')) != NULL) {
        lastLinePointer = linePointer;

        text.line_count++;
        linePointer++;
    }

    lastLineCount = 0;

    while(lastLinePointer != NULL && *lastLinePointer != '\0') {
        lastLinePointer++;
        lastLineCount++;
    }

    pthread_exit(NULL);
}
