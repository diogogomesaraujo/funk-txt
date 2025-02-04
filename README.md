# funk-txt

`funk-txt` is a minimal text editor built using Raylib in C. It displays and edits text files in a custom font, with a blinking cursor and dynamic text resizing.

![image](demo.gif)

## Features

- Open and edit text files.
- Dynamic view of the text.
- Keyboard support:
  - **Arrow Right**: Move cursor right.
  - **Arrow Left**: Move cursor left.
  - **Backspace**: Delete text.
  - **Enter**: Add a new line.
  - **Tab**: Add spaces.
  - **Ctrl + S**: Save file.
  - **Ctrl + X**: Clear text.

## Usage

1. **Run the program**:
   ```bash
   ./funk [filename]
   ```
   If no filename is provided, it defaults to `funk.txt`.

2. **Edit and save your text file (Ctrl S)**.

## Documentation
1. **Run to get permission.**:
  ```bash
  chmod +x docs.sh
  ```

2. **Generate documentation**:
  ```bash
  ./docs.sh
  ```

## Requirements

- Raylib library installed.
- A C compiler like GCC.
- (Optional) Doxygen installed for documentation.

## Future Work

- Glyphs
- Up and Down Movement
- Words per Second
- Config File for font, font-size, theme
- File Selection
