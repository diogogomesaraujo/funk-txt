#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    float abs_pos;
    Vector2 rel_pos;
    int current_ln_id;
    bool show;
    const char* txt;
} Cursor;

typedef struct {
    char* str;
    int str_len;
    int ln_count;
    int font_size;
} Text;

typedef struct {
    char* begin;
    char* end;
    int len;
} Line;

const float spacing = .05f;
const float smoothing = .95f;
const int cursor_rate = 30;

const Color bg_color = {0x28,0x28,0x28,0xff};
const Color text_color = {0xef,0xf1,0xf5,0xff};
const Color cursor_color = {0x6d,0x9d,0x97,0xff};

int screen_width = 800;
int screen_height = 600;

char* file_name = "funk.txt";

Text text;
Line current_line;
Cursor cursor;
Font font;
FILE* text_file;
Camera2D camera;

char* read_text_from_file(FILE* f, char* file_name);
int ln_count(char* str);
Line update_current_line(Cursor cursor, Text text);
Vector2 get_cursor_pos(Text text, Line current_line);

void setup(int argc, char** argv) {
    // FILE
    if (argc > 1) {
        file_name = argv[1];
    }

    text_file = fopen(file_name, "r+");

    // TEXT
    char* str = read_text_from_file(text_file, file_name);

    text = (Text) {
        str,
        strlen(str),
        ln_count(str),
        400
    };

    font = LoadFontEx("fonts/VictorMono-Regular.ttf", text.font_size, 0, 250);

    cursor = (Cursor) {
        0,
        get_cursor_pos(text, current_line),
        1,
        true,
        "|"
    };

    current_line = update_current_line(cursor, text);
}

void destroy() {
    free(text.str);
    fclose(text_file);
    UnloadFont(font);
}

int main(int argc, char** argv) {
    // SETUP
    InitWindow(screen_width, screen_height, file_name);

    setup(argc, argv);

    //
    while(!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(bg_color);

        DrawTextEx(font, text.str, (Vector2){0,0}, text.font_size, text.font_size * spacing, text_color);

        EndDrawing();

    }

    destroy();
    CloseWindow();
    return 0;
}

char* read_text_from_file(FILE* f, char* file_name) {
    if (f == NULL) {
        printf("hey trying to create new file!");
        FILE* aux = fopen(file_name, "w");
        fclose(aux);

        f = fopen(file_name, "r+");
    }

    rewind(f); // so it starts from the beggining!!

    char* file_text = NULL;
    int ch;
    int size = 0;
    while((ch = fgetc(f)) != EOF) {
        size++;
        file_text = realloc(file_text, sizeof(char) * size);
        file_text[size - 1] = ch;
    }

    file_text = realloc(file_text, sizeof(char) * (size + 1));
    file_text[size] = '\0';

    return file_text;
}

int ln_count(char* str) {
    int count = 1;
    char *line_pointer = str;

    while ((line_pointer = strchr(line_pointer, '\n')) != NULL && line_pointer) {
        count++;
        line_pointer++;
    }

    return count;
}

Line update_current_line(Cursor cursor, Text text) { // to be tested
    int count = 0;
    char *line_pointer = text.str;

    // go to the current line

    while ((line_pointer = strchr(line_pointer, '\n')) != NULL && line_pointer && count <= cursor.current_ln_id) {
        count++;
        line_pointer++;
    }

    int ln_size = 1;
    char *last_line_pointer = line_pointer + sizeof(char);
    //get the length of the line

    while(last_line_pointer != NULL && *last_line_pointer != '\0' && *last_line_pointer != '\n') {
        last_line_pointer++;
        ln_size++;
    }

    Line line = current_line = (Line) {
        line_pointer++,
        last_line_pointer--,
        ln_size
    };

    printf("\n\nCurrent Line: %c %c %d\n\n", *line_pointer, *last_line_pointer, ln_size);

    return line;
}

Vector2 get_cursor_pos(Text text, Line current_line) {
    Vector2 ch_size = MeasureTextEx(font, cursor.txt, text.font_size, 0);

    Vector2 cursor_pos = (Vector2) {
        ch_size.x * (float) current_line.len * (1 + spacing) - (ch_size.x / 2),
        ch_size.y * text.ln_count - (ch_size.y / 2)
    };

    printf("\nCursor Pos: %f %f\n", cursor_pos.x, cursor_pos.y);

    return cursor_pos;
}

/*Vector2 centerTextLastCharPos(char* text, float textSize, Font font, int lines, int lastLineCount) {
    Vector2 size = MeasureTextEx(font, text, textSize, textSize * SPACING);
    Vector2 charSize = MeasureTextEx(font, &text[strlen(text) - 1], textSize, 0);

    float lineWidth = 0;
    float lineHeight = size.y / lines;

    if (charSize.y == size.y) lineWidth = size.x;
    if(lastLineCount != 0) lineWidth = (lastLineCount - 1 - atChar) * charSize.x + (lastLineCount - 2 - atChar) * textSize * SPACING;

    size.x = ((float) screenWidth) / 2 - lineWidth - textSize * SPACING;
    size.y = ((float) screenHeight - lineHeight) / 2 - lineHeight * (lines - 1);

    return size;
    }*/
