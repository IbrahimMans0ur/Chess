// ### Include necessary libraries for graphics, sound, and utilities ###
#include <GL/glew.h>            // GLEW library for managing OpenGL extensions
#include <GL/freeglut.h>        // FreeGLUT for windowing and OpenGL context
#include <stdio.h>              // Standard I/O for printing messages
#include <stdbool.h>            // For using bool type
#include <math.h>               // Math functions (e.g., abs)
#include <stdlib.h>             // Standard library (e.g., memory allocation)
#include <windows.h>            // Windows API (for PlaySound, etc.)
#include <mmsystem.h>           // Multimedia functions (for PlaySound)
#define STB_IMAGE_IMPLEMENTATION // Include the implementation of stb_image for loading images
#include "stb_image.h"          // stb_image for loading PNG textures
#pragma comment(lib, "user32.lib") // Link user32 library for Windows GUI
#pragma comment(lib, "winmm.lib")  // Link winmm library for sound

// Constants for board and colors
#define BOARD_SIZE 8            // Number of squares per side on the chessboard
#define SQUARE_SIZE 87          // Pixel size of each square (700px / 8 squares = 87.5px, rounded to 87px , thats why we can see the black line at the right edge it is about 4px wide, but it is not a problem for the game logic)

// Data structures and global variables
typedef struct {
    int row, col;               // Structure to hold board coordinates
} T_Coordinates;

int board[8][8];                // The chessboard: each cell encodes piece and color
bool availableMoves[8][8] = {0}; // Highlight legal moves for selected piece
int selectedRow = -1, selectedCol = -1; // Currently selected square (-1 means none), to track which piece is selected, it is equal to the index of the row and column of the selected square in the board array
int currentPlayer = 0;          // 0 for white's turn, 1 for black's turn

// Texture handles for each chess piece,
// textures are images (often PNG, JPG) that are mapped onto the surfaces of shapes or 3D models to give them detailed appearance, color, or patterns.
//GLuint is a data type defined by OpenGL. It stands for "Unsigned Integer for OpenGL" and is typically used to store non-negative integer values.
//When you create a texture in OpenGL (using glGenTextures), OpenGL gives you a unique number (called a "texture handle" or "texture ID") that you use to refer to that texture later. This number is of type GLuint.
GLuint whitePawnTex, whiteKingTex, whiteQueenTex, whiteRookTex, whiteBishopTex, whiteKnightTex;
GLuint blackPawnTex, blackKingTex, blackQueenTex, blackRookTex, blackBishopTex, blackKnightTex;
GLuint menuBackgroundTex; // Add this near your other texture globals
GLuint theCreatorTex; // Add texture for the creator's image

// These variables is meant to handle the movements of some pieces for casteling reasons 
bool whiteKingMoved = false, blackKingMoved = false;
bool whiteKingsideRookMoved = false, whiteQueensideRookMoved = false;
bool blackKingsideRookMoved = false, blackQueensideRookMoved = false;

// Game state management
int gameState = 0; // 0 = menu, 1 = game, 2 = credits
int starter = 0;   // 0 = white, 1 = black

// Check if mouse is inside a rectangle function to help with button clicks
bool inRect(int mx, int my, int x, int y, int w, int h) {
    return mx >= x && mx <= x + w && my >= y && my <= y + h;
}

// ### Function prototypes ###
bool isLegalMove(int board[8][8], int fromRow, int fromCol, int toRow, int toCol, int currentPlayer); // Checks if a move is legal
bool isInCheck(int board[8][8], int player);      // Checks if a player is in check
bool isCheckmate(int board[8][8], int player);    // Checks if a player is in checkmate
void display(void);                               // Draws the board and pieces
void mouse(int button, int state, int x, int y);  // Handles mouse clicks
void boardInitializer(int board[8][8]);           // Sets up the initial board
void reshape(int w, int h);                       // Handles window resizing and fixing the window size to 700x700
void updateAvailableMoves();                  // Updates the available moves for the selected piece
GLuint loadTexture(const char* filename);         // Loads a PNG texture which is a common format for images with transparency, suitable for chess pieces

// ### Draw the chessboard and pieces ###
void display(void) {
    if (gameState == 0) {
        glClear(GL_COLOR_BUFFER_BIT); // Clear the screen before drawing the menu

        // Draw menu background
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, menuBackgroundTex);
        glColor3f(1, 1, 1);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2i(0, 0);
        glTexCoord2f(1, 0); glVertex2i(700, 0);
        glTexCoord2f(1, 1); glVertex2i(700, 700);
        glTexCoord2f(0, 1); glVertex2i(0, 700);
        glEnd();
        glDisable(GL_TEXTURE_2D);

        // Draw Start Game button
        glColor3f(0.2f, 0.6f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2i(220, 240); glVertex2i(520, 240); glVertex2i(520, 280); glVertex2i(220, 280);
        glEnd();
        glColor3f(1, 1, 1);
        glRasterPos2i(330, 265);
        const char* startMsg = "Start Game";
        for (const char* p = startMsg; *p; p++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);

        // Draw Credits button
        glColor3f(0.2f, 0.2f, 0.6f);
        glBegin(GL_QUADS);
        glVertex2i(220, 300); glVertex2i(520, 300); glVertex2i(520, 340); glVertex2i(220, 340);
        glEnd();
        glColor3f(1, 1, 1);
        glRasterPos2i(340, 325);
        const char* creditsMsg = "Credits";
        for (const char* p = creditsMsg; *p; p++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);

        // Draw Change Starter button
        glColor3f(0.6f, 0.4f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2i(220, 360); glVertex2i(520, 360); glVertex2i(520, 400); glVertex2i(220, 400);
        glEnd();
        glColor3f(1, 1, 1);
        glRasterPos2i(320, 385);
        char starterMsg[18];
        sprintf(starterMsg, "%s Starts", starter == 0 ? "White" : "Black"); // To store the starter message in the starterMsg array
        for (const char* p = starterMsg; *p; p++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);

        glutSwapBuffers();
        return;
    } else if (gameState == 2) { // This is the credits screen
        // If you don't want my credit to appear in the credit screen for any reason, you can comment the following blockof code
        glClear(GL_COLOR_BUFFER_BIT); // Clear the buffer first

        // Draw the creator image
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, theCreatorTex);
        glColor3f(1, 1, 1);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2i(300, 100);
        glTexCoord2f(1, 0); glVertex2i(475, 100);
        glTexCoord2f(1, 1); glVertex2i(475, 275);
        glTexCoord2f(0, 1); glVertex2i(300, 275);
        glEnd();
        glDisable(GL_TEXTURE_2D);

        // Credits text
        glColor3f(1, 1, 1);
        glRasterPos2i(320, 300);
        const char* credits1 = "Ibrahim Mansour";
        for (const char* p = credits1; *p; p++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
        glRasterPos2i(70, 350);
        const char* credits1v = "HI! I am Ibrahim Mansour, the creator of this game.";
        for (const char* p = credits1v; *p; p++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *p);
        glRasterPos2i(70, 370);
        const char* credits2 = "This game was made in the process of developing a C project for my university course.";
        for (const char* p = credits2; *p; p++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *p);
        glRasterPos2i(70, 390);
        const char* credits3 = "This game is made without using Cmake to make it easier and more like download and run.";
        for (const char* p = credits3; *p; p++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *p);
        glRasterPos2i(70, 410);
        const char* credits4 = "Also this game had passed on different stages of development.";
        for (const char* p = credits4; *p; p++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *p);
        glRasterPos2i(70, 430);
        const char* credits5 = "A Huge Shoutout to all my colleagues who helped in the early stages!";
        for (const char* p = credits5; *p; p++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *p);
        glRasterPos2i(70, 450);
        const char* credits6 = "DISCLAIMER: If you are using this game for university projects please read the code carefully.";
        for (const char* p = credits6; *p; p++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *p);
        glRasterPos2i(300, 600);
        const char* backMsg = "Click to go back";
        for (const char* p = backMsg; *p; p++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);

        glutSwapBuffers();
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT); // Clear the screen before drawing, so we start with a clean slate

    // Loop through each square on the board (8x8 grid)
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            // Set color for the square (alternating light/dark)
            glColor3f((i + j) % 2 == 0 ? 0.95f : 0.5f, (i + j) % 2 == 0 ? 0.85f : 0.3f, (i + j) % 2 == 0 ? 0.7f : 0.1f);// if the square (i+j%2==0) is even, use light color,(i+j%2!=0) else dark color
            glBegin(GL_QUADS); // Draw the square, to read 4 vertices in counter-clockwise order and connect them to form a square
            // Define the four corners of the square starting from the bottmom left corner
            // The coordinates are calculated based on the square size and position in the grid
            glVertex2f(j * SQUARE_SIZE, i * SQUARE_SIZE);
            glVertex2f((j + 1) * SQUARE_SIZE, i * SQUARE_SIZE);
            glVertex2f((j + 1) * SQUARE_SIZE, (i + 1) * SQUARE_SIZE);
            glVertex2f(j * SQUARE_SIZE, (i + 1) * SQUARE_SIZE);
            glEnd();

            // Highlight the selected square
            if (i == selectedRow && j == selectedCol) { // Checks if the current square is selected or not
                // If the square is selected, draw a yellow outline around it
                glColor3f(0.8f, 0.8f, 0.2f); // Yellow highlight color setting
                // Draw a line loop to create an outline to the selected square, starting from the bottom left corner and going counter-clockwise
                glLineWidth(4); // Line width is set for 4px
                glBegin(GL_LINE_LOOP);
                glVertex2f(j * SQUARE_SIZE, i * SQUARE_SIZE);
                glVertex2f((j + 1) * SQUARE_SIZE, i * SQUARE_SIZE);
                glVertex2f((j + 1) * SQUARE_SIZE, (i + 1) * SQUARE_SIZE);
                glVertex2f(j * SQUARE_SIZE, (i + 1) * SQUARE_SIZE);
                glEnd();
            }

            // Highlight available moves
            if (availableMoves[i][j]) { // When glutDisplayFunc() refreshes we check if there is a true value in a spicific square if the value of it is set to true that means that it was changed by updateAvailableMove() so we green the square
                glColor4f(0.2f, 0.8f, 0.2f, 0.4f); // Green with some transparency
                glBegin(GL_QUADS);
                glVertex2f(j * SQUARE_SIZE, i * SQUARE_SIZE);
                glVertex2f((j + 1) * SQUARE_SIZE, i * SQUARE_SIZE);
                glVertex2f((j + 1) * SQUARE_SIZE, (i + 1) * SQUARE_SIZE);
                glVertex2f(j * SQUARE_SIZE, (i + 1) * SQUARE_SIZE);
                glEnd();
            }

            int val = board[i][j];           // Get the value at this square
            int color = val / 10;            // Color: 0 for white, 1 for black
            int piece = val % 10;            // Piece type (1=pawn, 2=king, etc.)
            if (piece) {                     // If there is a piece
                GLuint tex = 0;              // Intialize temporary texture to handle the piece texture ID
                // Load the correct texture based on the piece type and color
                // Select the correct texture based on color and piece type
                if (color == 0) { // The square has a white piece
                    // White pieces: 1=pawn, 2=king, 3=queen, 4=rook, 5=bishop, 6=knight
                    if (piece == 1) tex = whitePawnTex;
                    else if (piece == 2) tex = whiteKingTex;
                    else if (piece == 3) tex = whiteQueenTex;
                    else if (piece == 4) tex = whiteRookTex;
                    else if (piece == 5) tex = whiteBishopTex;
                    else if (piece == 6) tex = whiteKnightTex;
                } else { // The square has a black pieace
                    // Black pieces: 1=pawn, 2=king, 3=queen, 4=rook, 5=bishop, 6=knight
                    if (piece == 1) tex = blackPawnTex;
                    else if (piece == 2) tex = blackKingTex;
                    else if (piece == 3) tex = blackQueenTex;
                    else if (piece == 4) tex = blackRookTex;
                    else if (piece == 5) tex = blackBishopTex;
                    else if (piece == 6) tex = blackKnightTex;
                }
                if (tex) {                   // If a texture is found so it is not 0
                    // Draw the piece using the texture
                    glEnable(GL_TEXTURE_2D); // Enable 2D texturing
                    glBindTexture(GL_TEXTURE_2D, tex); // Bind the texture, so OpenGL knows which texture to use
                    // Set color to white (no tint), so the texture color is used as is
                    glColor3f(1, 1, 1);      // Set color to white (no tint), ensuring the texture’s original colors are shown
                    float x0 = j * SQUARE_SIZE, y0 = i * SQUARE_SIZE;//Calculates the bottom-left corner of the square on the board
                    float x1 = x0 + SQUARE_SIZE, y1 = y0 + SQUARE_SIZE;//Calculates the top-right corner of the squareon the board
                    // Draw the textured square (the piece)
                    // The texture coordinates (0,0) to (1,1) map the entire texture to the square
                    glBegin(GL_QUADS);       // Draw the textured quad
                    // glTexCoord2f sets the texture coordinates for each vertex
                    // glVertex2f sets the vertex position in the square
                    // The texture coordinates map the entire texture to the square
                    glTexCoord2f(0, 0); glVertex2f(x0, y0); // Maps the texture’s bottom-left to the square’s bottom-left
                    glTexCoord2f(1, 0); glVertex2f(x1, y0); // Maps the texture’s bottom-right to the square’s bottom-right
                    glTexCoord2f(1, 1); glVertex2f(x1, y1); // Maps the texture’s top-right to the square’s top-right
                    glTexCoord2f(0, 1); glVertex2f(x0, y1); // Maps the texture’s top-left to the square’s top-left
                    // The vertices are defined in counter-clockwise order to ensure correct texture mapping
                    glEnd();
                    glDisable(GL_TEXTURE_2D); // Disable texturing for next draw
                }
            }
        }
    }
    glutSwapBuffers(); // Swap the front and back buffers (double buffering)
}

// ### Check if the current player's king is under attack ###
bool isInCheck(int board[8][8], int player) {
    int kingRow = -1, kingCol = -1; // Variables to store the king's position
    // Find the king's position for the given player
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if (board[i][j] == (player ? 12 : 2)) { // If the player is White(0) then the check is on a black king (12=white king, 2=black king)
                                                   // The logic is a bit reversed because the check happens in the opponent's turn
                kingRow = i;
                kingCol = j;
            }
    // Check if any opponent piece can move to the king's square
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++) {
            int val = board[i][j];
            int color = val / 10;
            // Only consider opponent's pieces
            if (color != player) // If the color is not equal to the player, then it is an opponent's piece
                // Check if the opponent's piece can legally move to the king's position
                if (isLegalMove(board, i, j, kingRow, kingCol, 1 - player))
                    return true; // King is attacked
        }
    return false; // King is safe
}

// ### Check if a move is legal for the current player ###
bool isLegalMove(int board[8][8], int fromRow, int fromCol, int toRow, int toCol, int currentPlayer) {
    int val = board[fromRow][fromCol], color = val / 10, piece = val % 10; // Get the piece type and color from the value at the starting square
    int target = board[toRow][toCol], targetColor = target / 10; // Get the target piece's color (0 for white, 1 for black) if there is a piece at the target square
    // Only allow moving your own pieces
    if (color != currentPlayer) return false; // If the piece color does not match the current player, the move is illegal
    // Can't capture your own piece
    if (target && color == targetColor) return false; // If the target square has a piece and its color matches the current player's color, the move is illegal, it also handles picking the same square the piece is on
    int dr = toRow - fromRow, dc = toCol - fromCol; // Calculate the row and column difference between the starting and target squares
    // Piece-specific movement rules
    switch (piece) {
        case 1: // Pawn
            if (color == 1) { // Black
                if (fromRow == 1 && dr == 2 && dc == 0 && board[fromRow+1][fromCol] == 0 && target == 0) return true; // Double move from start
                if (dr == 1 && dc == 0 && target == 0) return true; // Forward
                if (dr == 1 && abs(dc) == 1 && target && targetColor == 0) return true; // Capture
            } else { // White
                if (fromRow == 6 && dr == -2 && dc == 0 && board[fromRow-1][fromCol] == 0 && target == 0) return true; // Double move from start
                if (dr == -1 && dc == 0 && target == 0) return true; // Forward
                if (dr == -1 && abs(dc) == 1 && target && targetColor == 1) return true; // Capture
            }
            break;
        case 2: // King
            if (abs(dr) <= 1 && abs(dc) <= 1) return true; // King can move one square in any direction


            // ### Castling logic ###
            if (color == 0 && fromRow == 7 && fromCol == 4 && dr == 0) { // White king
                // Kingside castling
                if (dc == 2 && !whiteKingMoved && !whiteKingsideRookMoved &&
                    board[7][5] == 0 && board[7][6] == 0) {
                    // Check that king is not in check, does not pass through check, and does not end in check
                    bool safe = true;
                    // Check e1 (7,4), f1 (7,5), g1 (7,6)
                    int squares[3][2] = { {7,4}, {7,5}, {7,6} };
                    for (int k = 0; k < 3; ++k) {
                        int backup = board[squares[k][0]][squares[k][1]];
                        board[squares[k][0]][squares[k][1]] = 2; // Temporarily place king
                        if (isInCheck(board, color)) safe = false;
                        board[squares[k][0]][squares[k][1]] = backup;
                        if (!safe) break;
                    }
                    if (safe) return true;
                }
                // Queenside castling
                if (dc == -2 && !whiteKingMoved && !whiteQueensideRookMoved &&
                    board[7][3] == 0 && board[7][2] == 0 && board[7][1] == 0) {
                    // Check e1 (7,4), d1 (7,3), c1 (7,2)
                    bool safe = true;
                    int squares[3][2] = { {7,4}, {7,3}, {7,2} };
                    for (int k = 0; k < 3; ++k) {
                        int backup = board[squares[k][0]][squares[k][1]];
                        board[squares[k][0]][squares[k][1]] = 2;
                        if (isInCheck(board, color)) safe = false;
                        board[squares[k][0]][squares[k][1]] = backup;
                        if (!safe) break;
                    }
                    if (safe) return true;
                }
            }
            if (color == 1 && fromRow == 0 && fromCol == 4 && dr == 0) { // Black king
                // Kingside castling
                if (dc == 2 && !blackKingMoved && !blackKingsideRookMoved &&
                    board[0][5] == 0 && board[0][6] == 0) {
                    // Check e8 (0,4), f8 (0,5), g8 (0,6)
                    bool safe = true;
                    int squares[3][2] = { {0,4}, {0,5}, {0,6} };
                    for (int k = 0; k < 3; ++k) {
                        int backup = board[squares[k][0]][squares[k][1]];
                        board[squares[k][0]][squares[k][1]] = 12;
                        if (isInCheck(board, color)) safe = false;
                        board[squares[k][0]][squares[k][1]] = backup;
                        if (!safe) break;
                    }
                    if (safe) return true;
                }
                // Queenside castling
                if (dc == -2 && !blackKingMoved && !blackQueensideRookMoved &&
                    board[0][3] == 0 && board[0][2] == 0 && board[0][1] == 0) {
                    // Check e8 (0,4), d8 (0,3), c8 (0,2)
                    bool safe = true;
                    int squares[3][2] = { {0,4}, {0,3}, {0,2} };
                    for (int k = 0; k < 3; ++k) {
                        int backup = board[squares[k][0]][squares[k][1]];
                        board[squares[k][0]][squares[k][1]] = 12;
                        if (isInCheck(board, color)) safe = false;
                        board[squares[k][0]][squares[k][1]] = backup;
                        if (!safe) break;
                    }
                    if (safe) return true;
                }
            }
            break;
        case 3: // Queen
            if (abs(dr) == abs(dc) || dr == 0 || dc == 0) { // Queen can move diagonally or straight
                int stepR = (dr == 0) ? 0 : dr / abs(dr), stepC = (dc == 0) ? 0 : dc / abs(dc); // Calculate step direction, by dividing by the absolute value of dr and dc This ensures we move in the correct direction (up, down, left, right, or diagonally)
                int r = fromRow + stepR, c = fromCol + stepC; // Start from the next square in the direction of the move
                // Check if all squares between the start and end are empty
                while (r != toRow || c != toCol) {
                    if (board[r][c] != 0) return false;// If any square in the path is occupied, the move is illegal
                    // Move to the next square in the direction of the move
                    r += stepR; c += stepC;
                }
                return true; // If we reach here, the move is legal
            }
            break;
        case 4: // Rook
            if (dr == 0 || dc == 0) { // Rook can move straight (horizontal or vertical)
                int stepR = (dr == 0) ? 0 : dr / abs(dr), stepC = (dc == 0) ? 0 : dc / abs(dc); // Calculate step direction, by dividing by the absolute value of dr and dc This ensures we move in the correct direction (up, down, left, right)
                int r = fromRow + stepR, c = fromCol + stepC;
                // Check if all squares between the start and end are empty
                while (r != toRow || c != toCol) {
                    if (board[r][c] != 0) return false; // If any square in the path is occupied, the move is illegal
                    // Move to the next square in the direction of the move
                    r += stepR; c += stepC;
                }
                return true; // If we reach here, the move is legal
            }
            break;
        case 5: // Bishop
            if (abs(dr) == abs(dc)) { // Bishop can move diagonally
                int stepR = dr / abs(dr), stepC = dc / abs(dc); // Calculate step direction, by dividing by the absolute value of dr and dc This ensures we move in the correct direction (diagonally)
                int r = fromRow + stepR, c = fromCol + stepC;
                // Check if all squares between the start and end are empty
                while (r != toRow || c != toCol) {
                    if (board[r][c] != 0) return false; // If any square in the path is occupied, the move is illegal
                    // Move to the next square in the direction of the move
                    r += stepR; c += stepC;
                }
                return true; // If we reach here, the move is legal
            }
            break;
        case 6: // Knight
            if ((abs(dr) == 2 && abs(dc) == 1) || (abs(dr) == 1 && abs(dc) == 2)) return true; // Knight can jump in an "L" shape (2 squares in one direction and 1 square perpendicular)
            break;
    }
    return false; // If none of the piece-specific rules matched, the move is illegal
}

// ### Check if the current player is in checkmate ###
bool isCheckmate(int board[8][8], int player) {
    if (!isInCheck(board, player)) return false; // If the player is not in check, it cannot be checkmate
    // If the player is in check, we need to check if they have any legal moves to escape check
    // Try every possible move for the player; if any legal move gets out of check, not checkmate
    for (int fromRow = 0; fromRow < BOARD_SIZE; fromRow++)
        for (int fromCol = 0; fromCol < BOARD_SIZE; fromCol++) {
            int val = board[fromRow][fromCol], color = val / 10; // Get the piece color at the starting square
            // Only consider the player's own pieces
            if (player == color) // If the piece color matches the current player, then it is a piece of the current player
                // Try moving the piece to every possible square on the board
                for (int toRow = 0; toRow < BOARD_SIZE; toRow++)
                    for (int toCol = 0; toCol < BOARD_SIZE; toCol++)
                        if (isLegalMove(board, fromRow, fromCol, toRow, toCol, player)) {// Check if the move is legal
                            // Temporarily make the move to check if it escapes check
                            int backup = board[toRow][toCol];// Backup the piece at the target square
                            board[toRow][toCol] = board[fromRow][fromCol]; // Move the piece to the target square
                            board[fromRow][fromCol] = 0; // Empty the starting square
                            bool stillInCheck = isInCheck(board, player); // Check if the player is still in check after the move
                            board[fromRow][fromCol] = board[toRow][toCol]; // Restore the piece to the starting square
                            board[toRow][toCol] = backup; // Restore the piece at the target square
                            if (!stillInCheck) return false; // If the player can escape check with this move, it's not checkmate (stillInCheck is false)
                        }
        }
    return true; // No legal moves to escape check
}

// ### Handle mouse clicks for selecting and moving pieces ###
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Next block will handle the menu buttons
        if (gameState == 0) {
            // GLUT y is from top, our ortho is from top, so y is fine

            // HUSH!!
            if (inRect(x, y, 0, 0, 10, 10)) {
                PlaySound("Fun.wav", NULL, SND_FILENAME | SND_ASYNC);
                glutPostRedisplay();
                return;
            }
            // Start Game button
            if (inRect(x, y, 220, 240, 300, 40)) {
                currentPlayer = starter;
                gameState = 1;
                glutPostRedisplay();
                return;
            }
            // Credits button
            if (inRect(x, y, 220, 300, 300, 40)) {
                gameState = 2;
                glutPostRedisplay();
                return;
            }
            // Change Starter button
            if (inRect(x, y, 220, 360, 300, 40)) {
                starter = 1 - starter;
                glutPostRedisplay();
                return;
            }
            // Next will handle the credits screen
        } else if (gameState == 2) {
            // Any click returns to menu
            gameState = 0;
            glutPostRedisplay();
            return;
        }
    }
    // If we are in the game state, handle piece selection and movement
    if (gameState == 1) {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) { // Only on left click
            int col = x / SQUARE_SIZE, row = y / SQUARE_SIZE; // Convert pixel to board coordinates
            if (row >= 0 && row < 8 && col >= 0 && col < 8) {
                if (selectedRow == -1) { // No piece selected yet
                    int val = board[row][col], color = val / 10; // Get the piece color at the clicked square
                    // Only select your own piece
                    if (val != 0 && currentPlayer == color) { // If the square is not empty and the piece color matches the current player, then store the selected piece coordinates
                        selectedRow = row;
                        selectedCol = col;
                        updateAvailableMoves(); // Update available moves for the selected piece
                    }
                } else { // Piece already selected, try to move
                    if (!(selectedRow == row && selectedCol == col)) { // If the clicked square is not the same as the selected square, attempt to move the selected piece to the clicked square
                        if (isLegalMove(board, selectedRow, selectedCol, row, col, currentPlayer)) { // Check if the move is legal
                            // Make the move if it is legal
                            int backup = board[row][col]; // Backup the piece at the target square
                            int movedPiece = board[selectedRow][selectedCol]; // Get the piece being moved
                            board[row][col] = movedPiece; // Move the piece to the target square
                            board[selectedRow][selectedCol] = 0; // Empty the starting square

                            // ### Update castling rights for any king or rook move ###
                            // White king moves from e1
                            if (movedPiece == 2 && selectedRow == 7 && selectedCol == 4)
                                whiteKingMoved = true;
                            // Black king moves from e8
                            if (movedPiece == 12 && selectedRow == 0 && selectedCol == 4)
                                blackKingMoved = true;
                            // White kingside rook moves from h1
                            if (movedPiece == 4 && selectedRow == 7 && selectedCol == 7)
                                whiteKingsideRookMoved = true;
                            // White queenside rook moves from a1
                            if (movedPiece == 4 && selectedRow == 7 && selectedCol == 0)
                                whiteQueensideRookMoved = true;
                            // Black kingside rook moves from h8
                            if (movedPiece == 14 && selectedRow == 0 && selectedCol == 7)
                                blackKingsideRookMoved = true;
                            // Black queenside rook moves from a8
                            if (movedPiece == 14 && selectedRow == 0 && selectedCol == 0)
                                blackQueensideRookMoved = true;

                            // ### Castling rook move logic ###
                            if (movedPiece % 10 == 2 && !isInCheck(board,currentPlayer)) { // King
                                // White kingside castling
                                if (movedPiece == 2 && selectedRow == 7 && selectedCol == 4 && row == 7 && col == 6) {
                                    board[7][5] = board[7][7]; // Move rook
                                    board[7][7] = 0;
                                }
                                // White queenside castling
                                if (movedPiece == 2 && selectedRow == 7 && selectedCol == 4 && row == 7 && col == 2) {
                                    board[7][3] = board[7][0];
                                    board[7][0] = 0;
                                }
                                // Black kingside castling
                                if (movedPiece == 12 && selectedRow == 0 && selectedCol == 4 && row == 0 && col == 6) {
                                    board[0][5] = board[0][7];
                                    board[0][7] = 0;
                                }
                                // Black queenside castling
                                if (movedPiece == 12 && selectedRow == 0 && selectedCol == 4 && row == 0 && col == 2) {
                                    board[0][3] = board[0][0];
                                    board[0][0] = 0;
                                }

                                // ### Update castling rights ###
                                if (movedPiece == 2) whiteKingMoved = true;
                                if (movedPiece == 12) blackKingMoved = true;
                                if (movedPiece == 4 && selectedRow == 7 && selectedCol == 0) whiteQueensideRookMoved = true;
                                if (movedPiece == 4 && selectedRow == 7 && selectedCol == 7) whiteKingsideRookMoved = true;
                                if (movedPiece == 14 && selectedRow == 0 && selectedCol == 0) blackQueensideRookMoved = true;
                                if (movedPiece == 14 && selectedRow == 0 && selectedCol == 7) blackKingsideRookMoved = true;
                            }

                           
                            // ### Pawn Promotion ###
                            int piece = movedPiece % 10;
                            int color = movedPiece / 10;
                            if (piece == 1) { // If the moved piece is a pawn
                                if ((color == 0 && row == 0) || (color == 1 && row == 7)) {
                                    // Promote to queen (3)
                                    board[row][col] = color * 10 + 3;
                                    printf("Pawn promoted to Queen!\n");
                                }
                            }

                            // Check if the move puts the player in check
                            // If the move does not put the player in check, then switch players and play sound
                            if (!isInCheck(board, currentPlayer)) {
                                (backup==0)? PlaySound("Move.wav", NULL, SND_FILENAME | SND_SYNC) /* Play move sound */ : PlaySound("Capture.wav", NULL, SND_FILENAME | SND_SYNC); /* Play capture sound */
                                PlaySound("sound.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP); // Play background music
                                currentPlayer = 1 - currentPlayer; // Switch player
                                if (isInCheck(board, currentPlayer)) printf("Check!\n");
                                if (isCheckmate(board, currentPlayer)) {
                                    printf("Checkmate!\n");
                                    PlaySound("gameEnd.wav", NULL, SND_FILENAME | SND_SYNC); // Play end sound
                                    printf("%s wins!\n", currentPlayer? "White":"Black"); // Print the winning player (the logic of currentPlayer is reversed because it came after switching players)
                                }
                            } else {
                                // Undo move if it puts player in check
                                board[selectedRow][selectedCol] = movedPiece;
                                board[row][col] = backup;
                            }
                        }
                    }
                    selectedRow = -1;
                    selectedCol = -1;
                    updateAvailableMoves(); // Resetting the AvailableMoves array to be ready for the next selection
                }
                glutPostRedisplay(); // Redraw the board
            }
        }
    }
}

// ### Set up the initial chessboard position ###
void boardInitializer(int board[8][8]) {
    // Standard chess starting position, encoded as integers
    int chessBoard[8][8] = {
        {14, 16, 15, 13, 12, 15, 16, 14}, // Black major pieces
        {11, 11, 11, 11, 11, 11, 11, 11}, // Black pawns
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {01, 01, 01, 01, 01, 01, 01, 01}, // White pawns
        {04, 06, 05, 03, 02, 05, 06, 04}  // White major pieces
    };
    // Copy the starting position into the board array
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            board[i][j] = chessBoard[i][j];
}

// ### Force window size back to 700x700 ###
void reshape(int w, int h) {
    glutReshapeWindow(700, 700); // Always keep window square and fixed size
}

// ### Load texture from file ###
GLuint loadTexture(const char* filename) { // Load a PNG texture from file
    // Use stb_image to load the PNG file
    int width, height, channels; // Variables to hold image dimensions and channels
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4); // Load PNG as RGBA
    if (!data) {
        printf("Failed to load texture: %s\n", filename); // Print error if file not found
        return 0;
    }
    GLuint tex; // Texture handle to be returned
    // Generate and set up the texture
    glGenTextures(1, &tex); // 1 is the number of textures to be generated and to store the texture ID in the variable tex
    glBindTexture(GL_TEXTURE_2D, tex); // Bind it for setup (2D setup), which means making it the current texture for 2D rendering
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); // Upload pixel data
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear filtering for minification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear magnification
    stbi_image_free(data); // Free image memory
    return tex; // Return texture handle
}

// ### Update available moves for the selected piece ###
void updateAvailableMoves() {
    // Clear previous highlights
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            availableMoves[i][j] = false;

    if (selectedRow == -1 || selectedCol == -1) // Checks if there is a selected piece
        return;

    int val = board[selectedRow][selectedCol];
    int color = val / 10;
    if (val == 0 || color != currentPlayer) // Checks if there is a piece selected and not just an empty square and the color selected matches the player color to validate players turn
        return;

    // check all squares on the board for available moves for the selected piece
    // Loop through all squares on the board to find legal moves for the selected piece
    for (int r = 0; r < BOARD_SIZE; r++)
        for (int c = 0; c < BOARD_SIZE; c++)
            if (isLegalMove(board, selectedRow, selectedCol, r, c, currentPlayer)) { // Check if the move is legal for the selected piece
            // If the move is legal, mark it as available
            // Simulate the move
            int backup = board[r][c];   // Backup the piece at the target square
            // Move the piece to the target square
            int movedPiece = board[selectedRow][selectedCol];
            board[r][c] = movedPiece; // Place the selected piece at the target square
            // Empty the selected square
            board[selectedRow][selectedCol] = 0;
            bool leavesInCheck = isInCheck(board, currentPlayer);  // Check if the move leaves the player in check
            if (!leavesInCheck) // If the move does not leave the player in check
                availableMoves[r][c] = true; // Mark the target square as available for the selected piece
            // Undo the move
            board[selectedRow][selectedCol] = movedPiece;
            board[r][c] = backup;
            if (!leavesInCheck)
                availableMoves[r][c] = true;
        }
}

// ### Main function: initializes everything and starts the game loop ###
int main(int argc, char **argv) {
    glutInit(&argc, argv); // Initialize GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // Double buffering, RGBA color
    glutInitWindowSize(700, 700); // Set window size
    glutCreateWindow("Chess Game"); // Create window with title
    glewInit(); // Initialize GLEW (must be after window creation)

    // Enable alpha blending for PNG transparency, to show only the chess piece without a background
    glEnable(GL_BLEND); // Enable blending for transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Set blending function for transparency

    // Matching the coordinates of the mouse and input drawing to the chessboard
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black
    glMatrixMode(GL_PROJECTION); // Set projection matrix
    glLoadIdentity(); // Load identity matrix to reset projection
    gluOrtho2D(0, 700, 700, 0); // Set orthographic projection (top-left origin)
    
    glutReshapeFunc(reshape); // Set reshape callback

    PlaySound("sound.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP); // Play background music

    // Load all chess piece textures before even starting the game
    whitePawnTex   = loadTexture("White_Pawn.png");
    whiteKingTex   = loadTexture("White_King.png");
    whiteQueenTex  = loadTexture("White_Queen.png");
    whiteRookTex   = loadTexture("White_Rook.png");
    whiteBishopTex = loadTexture("White_Bishop.png");
    whiteKnightTex = loadTexture("White_Knight.png");
    blackPawnTex   = loadTexture("Black_Pawn.png");
    blackKingTex   = loadTexture("Black_King.png");
    blackQueenTex  = loadTexture("Black_Queen.png");
    blackRookTex   = loadTexture("Black_Rook.png");
    blackBishopTex = loadTexture("Black_Bishop.png");
    blackKnightTex = loadTexture("Black_Knight.png");
    menuBackgroundTex = loadTexture("Background_Main_Menu.jpg");
    theCreatorTex = loadTexture("TheCreator.jpg");

    boardInitializer(board); // Set up the initial board

    glutDisplayFunc(display); // Set display callback
    glutMouseFunc(mouse);     // Set mouse callback
    glutReshapeFunc(reshape); // Set reshape callback

    glutMainLoop(); // Start the main event loop
    return 0; // Program should never reach here
}
