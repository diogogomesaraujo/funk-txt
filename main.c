#include <math.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#define FILE_NAME "funk.txt"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define TEXT_SIZE 400.
#define SPACING 0.05
#define CURSOR_RATE 30
#define SMOOTHING 0.95f
#define BACKGROUND_COLOR (Color) {0x28,0x28,0x28,0xff}
#define TEXT_COLOR (Color) {0xef,0xf1,0xf5,0xff}
#define CURSOR_COLOR (Color) {0x6d,0x9d,0x97,0xff}

typedef struct {
    int width;
    int height;
    Font font;
} Editor;

typedef struct {
    char* str;
    int len;
    Vector2 pos;
    float font_size;
    int line_count;
}Text;

typedef struct {
    // int pos; // 0 is the last line;
    // char* start;
    // char* end;
    int len;
} Line;

typedef struct {
    const char* str;
    int abs_pos;
    Vector2 rel_pos;
    int count;
    bool show;
} Cursor;

Text text;
Cursor cursor;
Editor editor;
Line currentLine;

void* handleLinesAndCount();
char* textHandler(Text text);
void* readTextFromFile(void* f);
float textSizeFromLen(int textLen);
void* controlOperations(void* fileName);
float lerp(float a, float b, float f);
float smoothing(float a, float b, float s);
void showCursorAndUpdate(float textSize, Font font);
Vector2 centerTextPos(Text text, Font font, int lastLineCount);
Vector2 centerCursorPos(char* cursor, float textSize, Font font, float offsetX, float offsetY);

int main(int argc, char** argv) {
    // ARGS
    const char* fileName = FILE_NAME;

    if (argc > 1) {
        fileName = argv[1];
    }

    // WINDOW CONFIG
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

    pthread_t fileRead;

    pthread_create(&fileRead, NULL, readTextFromFile, f);
    pthread_join(fileRead, NULL);

    // TEXT
    editor.font = LoadFontEx("VictorMono-Regular.ttf", TEXT_SIZE, 0, 250);

    text.font_size = textSizeFromLen(text.len);
    text.line_count = 1;
    text.pos = centerTextPos(text, editor.font, 0);

    // CURSOR
    cursor.str = "|";
    cursor.abs_pos = 0;
    cursor.rel_pos = centerCursorPos((char*) cursor.str, text.font_size, editor.font, 0, 0);
    cursor.count = 0;
    cursor.show = true;

    // EDITOR
    editor.width = SCREEN_WIDTH;
    editor.height = SCREEN_HEIGHT;

    // RENDER
    while(!WindowShouldClose()) {
        if(IsKeyPressed(KEY_ESCAPE)) return 0;

        if (editor.width != GetScreenWidth() || editor.height != GetScreenHeight()) {
            editor.width = GetScreenWidth();
            editor.height = GetScreenHeight();
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
            cursor.show = true;
            cursor.count = 0;
        }

        pthread_t handleText;

        pthread_create(&handleText, NULL, handleLinesAndCount, NULL);
        pthread_join(handleText, NULL);

        text.len = strlen(text.str);

        if(currentLine.len == 0) currentLine.len = text.len + 1;

        if(IsKeyPressed(KEY_LEFT) && currentLine.len - cursor.abs_pos - 1 > 0) {
            cursor.abs_pos++;
            cursor.show = true;
            cursor.count = 0;
        }
        if(IsKeyPressed(KEY_RIGHT) && cursor.abs_pos > 0) {
            cursor.abs_pos--;
            cursor.show = true;
            cursor.count = 0;
        }

        text.font_size = smoothing(textSizeFromLen(text.len), text.font_size, SMOOTHING);
        Vector2 auxText = centerTextPos(text, editor.font, currentLine.len);
        Vector2 auxCursor1 = centerCursorPos((char*)cursor.str, text.font_size, editor.font, 0, 0);

        if(text.len == 0) {
            text.pos = (Vector2){
                smoothing(203.000275f, text.pos.x, SMOOTHING),
                smoothing(100.000206f, text.pos.y, SMOOTHING)
            };
            cursor.rel_pos = (Vector2){
                smoothing(auxCursor1.x, cursor.rel_pos.x, SMOOTHING),
                smoothing(auxCursor1.y, cursor.rel_pos.y, SMOOTHING)
            };

        }
        else {
            text.pos = (Vector2){
                smoothing(auxText.x, text.pos.x, SMOOTHING),
                smoothing(auxText.y, text.pos.y, SMOOTHING)
            };
            cursor.rel_pos = centerCursorPos((char*) cursor.str, text.font_size, editor.font, auxText.x - text.pos.x, auxText.y - text.pos.y);

        }

        // DRAW
        BeginDrawing();

        ClearBackground(BACKGROUND_COLOR);

        DrawTextEx(editor.font, text.str, (Vector2) {
            text.pos.x,
            text.pos.y
        }, text.font_size, text.font_size * SPACING, TEXT_COLOR);

        showCursorAndUpdate(text.font_size, editor.font);

        EndDrawing();
    }

    // TERMINATE

    CloseWindow();
    free(text.str);
    if (f) fclose(f);
    UnloadFont(editor.font);

    return 0;
}

float lerp(float a, float b, float f) {
    return a * (1.0 - f) + (b * f);
}

float smoothing(float a, float b, float s) {
    return a + (b - a) * s;
}

Vector2 centerTextPos(Text text, Font font, int lastLineCount) {
    Vector2 size = MeasureTextEx(font, text.str, text.font_size, text.font_size * SPACING);
    Vector2 charSize = MeasureTextEx(font, &text.str[text.len - 1], text.font_size, 0);

    float lineWidth = 0;
    float lineHeight = size.y / text.line_count;

    if (charSize.y == size.y) lineWidth = size.x;
    if(lastLineCount != 0) lineWidth = (lastLineCount - 1 - cursor.abs_pos) * charSize.x + (lastLineCount - 2 - cursor.abs_pos) * text.font_size * SPACING;

    size.x = ((float) editor.width) / 2 - lineWidth - text.font_size * SPACING;
    size.y = ((float) editor.height - lineHeight) / 2 - lineHeight * (text.line_count - 1);

    return size;
}

Vector2 centerCursorPos(char* cursor, float textSize , Font font, float offsetX, float offsetY) {
    Vector2 size = MeasureTextEx(font, cursor, textSize, textSize * SPACING);

    size.x = ((float) editor.width - size.x) / 2 - offsetX;
    size.y = ((float) editor.height - size.y) / 2 - offsetY;

    return size;
}

float textSizeFromLen(int textLen) {
    if (textLen <= 0) return TEXT_SIZE;
    return TEXT_SIZE * ((float)1 / log2f((float) (textLen + 1)));
}

char* textHandler(Text text) {
    if(IsKeyPressed(KEY_BACKSPACE) && text.len - cursor.abs_pos > 0) {
        if(cursor.abs_pos == 0) {
            cursor.abs_pos = 0;

            char* changedText = malloc(sizeof(char) * (text.len));
            memcpy(changedText, text.str, text.len - 1);

            changedText[text.len - 1] = '\0';

            return changedText;
        } else {
            char* changedText = malloc(sizeof(char) * (text.len));
            strncpy(changedText, text.str, text.len - cursor.abs_pos - 1);

            strncpy(&changedText[text.len - (int) cursor.abs_pos - 1], &text.str[text.len - (int) cursor.abs_pos], cursor.abs_pos + 1);

            return changedText;
        }
    }

    if(IsKeyPressed(KEY_SPACE)) {
        if(cursor.abs_pos == 0) {
            char* changedText = malloc(sizeof(char) * (text.len + 2));
            strncpy(changedText, text.str, text.len);

            changedText[text.len] = ' ';

            cursor.abs_pos = 0;

            return changedText;
        } else {
            char* changedText = malloc(sizeof(char) * (text.len + 2));
            strncpy(changedText, text.str, text.len - cursor.abs_pos);

            changedText[text.len - (int) cursor.abs_pos] = ' ';

            strncpy(&changedText[text.len - (int) cursor.abs_pos + 1], &text.str[text.len - (int) cursor.abs_pos], cursor.abs_pos + 1);

            return changedText;
        }
    }

    if(IsKeyPressed(KEY_TAB)) {
        if(cursor.abs_pos == 0) {
            char* changedText = malloc(sizeof(char) * (text.len + 5));
            strncpy(changedText, text.str, text.len);

            for(int i = text.len; i < text.len + 4; i++) {
                changedText[i] = ' ';
            }

            cursor.abs_pos = 0;

            return changedText;
        } else {
            char* changedText = malloc(sizeof(char) * (text.len + 5));
            strncpy(changedText, text.str, text.len - cursor.abs_pos);

            for(int i = text.len - cursor.abs_pos; i < text.len + 4; i++) {
                changedText[i] = ' ';
            }

            strncpy(&changedText[text.len - (int) cursor.abs_pos + 4], &text.str[text.len - (int) cursor.abs_pos], cursor.abs_pos + 1);

            return changedText;
        }
    }

    if(IsKeyPressed(KEY_ENTER)) {
        if(cursor.abs_pos == 0) {
            char* changedText = malloc(sizeof(char) * (text.len + 2));
            strncpy(changedText, text.str, text.len);

            changedText[text.len] = '\n';

            cursor.abs_pos = 0;

            return changedText;
        } else {
            char* changedText = malloc(sizeof(char) * (text.len + 2));
            strncpy(changedText, text.str, text.len - cursor.abs_pos);

            changedText[text.len - (int) cursor.abs_pos] = '\n';

            strncpy(&changedText[text.len - (int) cursor.abs_pos + 1], &text.str[text.len - (int) cursor.abs_pos], cursor.abs_pos + 1);

            return changedText;
        }
    }

    int key;

    while ((key = GetCharPressed()) != 0) { // wait for char to read the continue!!
        if(cursor.abs_pos == 0) {
            char* changedText = malloc(sizeof(char) * (text.len + 2));
            strncpy(changedText, text.str, text.len);

            changedText[text.len] = key;
            changedText[text.len + 1] = '\0';

            cursor.abs_pos = 0;

            return changedText;
        } else {
            char* changedText = malloc(sizeof(char) * (text.len + 2));
            strncpy(changedText, text.str, text.len - cursor.abs_pos);

            changedText[text.len - (int) cursor.abs_pos] = key;

            strncpy(&changedText[text.len - (int) cursor.abs_pos + 1], &text.str[text.len - (int) cursor.abs_pos], cursor.abs_pos + 1);
            changedText[strlen(changedText)] = '\0';

            return changedText;
        }
    }

    return text.str;
}

void showCursorAndUpdate(float textSize, Font font) {
    if (cursor.count < CURSOR_RATE && cursor.show) {
        DrawTextEx(font, cursor.str, cursor.rel_pos, textSize, textSize * SPACING, CURSOR_COLOR);
        cursor.count++;
        return;
    }
    else if (cursor.show) {
        cursor.show = false;
        cursor.count = 0;
        return;
    }
    else if (cursor.count < CURSOR_RATE && !cursor.show) {
        cursor.count++;
        return;
    }
    else {
        cursor.show = true;
        cursor.count = 0;
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

    pthread_exit(NULL);
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
        cursor.abs_pos = 0;
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

    currentLine.len = 0;

    while(lastLinePointer != NULL && *lastLinePointer != '\0') {
        lastLinePointer++;
        currentLine.len++;
    }

    pthread_exit(NULL);
}
