/*
* Program Name: EECS 348 Assignment 4
*
* Description: A program that solves sudoku puzzles using a recursive object oriented searching method. It finds every solution showing puzzles that have multiple solutions and showing puzzles with no solution
*
* Inputs: Text files that contain 9x9 sudoku puzzles, a row per line that contain digits from 1-9 for filled pieces and _ for empty pieces. Uses the files puzzle1.txt, puzzle2.txt, puzzle3.txt, puzzle4.txt, puzzle5.txt
*
* Outputs: The file name, the puzzle read from the file, and then the solutions all written to the output
*
* Collaborators: Connor Wiley
*
* Other sources: Code generated via Claude Anthropic Sonnet 5.5
*
* Author: Connor Wiley, Claude Anthropic
*
* Creation Date: 10/15/2026
*/

// Sudoku brute-force solver (depth-first search with backtracking).
// Object-oriented design: all logic lives in classes; main() only creates
// an application object and runs it.
//
// Build:  g++ -std=c++17 -O2 -o sudoku sudoku.cpp
// Run:    ./sudoku puzzle1.txt puzzle2.txt puzzle3.txt puzzle4.txt puzzle5.txt
//         (with no arguments it looks for puzzle1.txt ... puzzle5.txt in the
//          current directory)

#include <array> // allows usage of std::array, used as a fixed size for the board
#include <cctype> // allows usage of character helper functions, used for std::isspace when reading puzzles
#include <fstream> // allows usage of ifstream, used to open the puzzle files
#include <iostream> // allows usage of cout and ostream, used for printing outputs like boards and messages
#include <string> // allows usage of the string type, used for filenames and for reading the puzzle line by line
#include <utility> // allows usage of std::move, used to hand the solutions back without copying them
#include <vector> // allows usage of vector type, used to hold solutions and filenames

// ---------------------------------------------------------------------------
// Board: a 9x9 grid of digits (0 = empty). Knows the Sudoku rules.
// ---------------------------------------------------------------------------
class Board { // creates a class Board that holds grids and defines sudoku
public: // makes it so everything below can be called by outside functions
    static const int SIZE = 9; // instantiates an int named size for the width and height of the board, static so it doesnt change
    static const int BLOCK = 3; // instantiates an int named block for the width and height of each small box 3x3 for example
    static const int ALL_DIGITS = 0x3FE; // instantiates an int named ALL_DIGITS that has the bits for digits 1 through 9 turned on (bit 0 is unused), used to build the candidate mask

    Board() { // creates a constructor that fills each cell with zero so we have an empty board
        cells_.fill(0); // fills every cell with 0
        rowMask_.fill(0); // clears the record of which digits are used in each row
        colMask_.fill(0); // clears the record of which digits are used in each column
        blockMask_.fill(0); // clears the record of which digits are used in each 3x3 block
    } // ends constructor

    int get(int row, int col) const { return cells_[row * SIZE + col]; } // creates the get function that returns the value at a row and column in the array
    void set(int row, int col, int value) { // creates the set function that puts a value at the specified row and column in the array and keeps the used digit masks up to date
        int old = cells_[row * SIZE + col]; // instantiates an int named old that holds the digit currently in the cell
        int b = (row / BLOCK) * BLOCK + col / BLOCK; // instantiates an int named b that holds the number 0 through 8 of the 3x3 block the cell is in
        if (old != 0) { // checks if the cell currently holds a digit
            rowMask_[row] &= ~(1 << old); // removes the old digit from the digits used in the row
            colMask_[col] &= ~(1 << old); // removes the old digit from the digits used in the column
            blockMask_[b] &= ~(1 << old); // removes the old digit from the digits used in the block
        } // ends if statement
        if (value != 0) { // checks if the new value is a digit and not a blank
            rowMask_[row] |= (1 << value); // marks the new digit as used in the row
            colMask_[col] |= (1 << value); // marks the new digit as used in the column
            blockMask_[b] |= (1 << value); // marks the new digit as used in the block
        } // ends if statement
        cells_[row * SIZE + col] = value; // stores the value in the cell
    } // ends set function

    // True if `value` can legally be placed at (row, col) given the other cells.
    bool isCandidate(int row, int col, int value) const { // creates the isCandiate function that checks if a spot is legal to enter
        for (int i = 0; i < SIZE; ++i) { // loops through all positions in the row and columns to check them
            if (i != col && get(row, i) == value) return false;  // row // checks the rows
            if (i != row && get(i, col) == value) return false;  // column // checks the columns
        } // ends the for loop for columns and rows
        int br = (row / BLOCK) * BLOCK, bc = (col / BLOCK) * BLOCK; // instantiates the top left corner of the 3x3 block the cells is in, rounding it down
        for (int r = br; r < br + BLOCK; ++r) // loops through the 3 rows of the block
            for (int c = bc; c < bc + BLOCK; ++c) // loops through the 3 columns of the block
                if ((r != row || c != col) && get(r, c) == value) return false;  // block // checks the block for exact value, skipping the original cell, if theres a match its invalid 
        return true; // combonation is correct so value is allowed
    } // ends isCanidate function

    // Bitmask of the digits that may legally go into the EMPTY cell (row, col), bit v is on if digit v is allowed.
    int candidateMask(int row, int col) const { // creates the candidateMask function that replaces the old candidates function, it returns a bitmask instead of building a vector
        int b = (row / BLOCK) * BLOCK + col / BLOCK; // instantiates an int named b that holds the number 0 through 8 of the 3x3 block the cell is in
        int used = rowMask_[row] | colMask_[col] | blockMask_[b]; // instantiates an int named used that combines every digit already used in the row, column, and block
        return ~used & ALL_DIGITS; // flips the used digits so the legal ones are on, then keeps only bits 1 through 9
    } // ends candidateMask function

    // Finds the next empty cell in row-major order starting at index `start`. Returns false if there is none.
    bool findEmpty(int& row, int& col, int start = 0) const { // creates the findEmpty function that finds the next empty cell, row and col are references so the answer gets sent back through them, start lets the search skip cells that are already known to be filled
        for (int i = start; i < SIZE * SIZE; ++i) { // loops through the cells in order beginning at start instead of always at cell 0
            if (cells_[i] == 0) { // checks if the cell is empty
                row = i / SIZE; // converts the index into a row number
                col = i % SIZE; // converts the index into a column number
                return true; // returns true since an empty cell was found
            } // ends if statement
        } // ends for loop
        return false; // returns false since no empty cells are left, meaning the board is full
    } // ends findEmpty function

    // True if no filled cell conflicts with another (used to vet the given puzzle).
    bool isConsistent() const { // creates the isConsistent function that checks if the numbers already on the board break any rules
        for (int r = 0; r < SIZE; ++r) // loops through each row
            for (int c = 0; c < SIZE; ++c) { // loops through each column
                int v = get(r, c); // instantiates an int named v that holds the value at that cell
                if (v != 0 && !isCandidate(r, c, v)) return false; // checks if the cell is filled and its value conflicts with another cell, if so returns false
            } // ends the for loop for columns
        return true; // returns true since nothing conflicted
    } // ends isConsistent function

    void print(std::ostream& out) const { // creates the print function that writes the board to whatever output stream it is given
        for (int r = 0; r < SIZE; ++r) { // loops through each row
            for (int c = 0; c < SIZE; ++c) { // loops through each column
                int v = get(r, c); // instantiates an int named v that holds the value at that cell
                out << (v == 0 ? '_' : static_cast<char>('0' + v)) << ' '; // prints an underscore if the cell is empty, otherwise prints the digit as a character, then a space
            } // ends the for loop for columns
            out << '\n'; // moves to the next line after each row
        } // ends the for loop for rows
    } // ends print function

private: // makes it so everything below can only be used inside the class
    std::array<int, SIZE * SIZE> cells_; // instantiates an array of 81 ints named cells_ that stores the whole board in one line
    std::array<int, SIZE> rowMask_; // instantiates an array of 9 ints named rowMask_ where each one has a bit on for every digit used in that row
    std::array<int, SIZE> colMask_; // instantiates an array of 9 ints named colMask_ where each one has a bit on for every digit used in that column
    std::array<int, SIZE> blockMask_; // instantiates an array of 9 ints named blockMask_ where each one has a bit on for every digit used in that 3x3 block
}; // ends Board class

// ---------------------------------------------------------------------------
// PuzzleReader: loads a Board from a text file ('_' = blank).
// ---------------------------------------------------------------------------
class PuzzleReader { // creates a class PuzzleReader that loads a board from a text file
public: // makes it so everything below can be called by outside functions
    // Returns true on success and fills `board`.
    bool read(const std::string& filename, Board& board) const { // creates the read function that fills a board from a file and returns whether it worked
        std::ifstream in(filename); // instantiates an ifstream named in that opens the file
        if (!in) return false; // checks if the file failed to open, if so returns false
        // Correctness fix: the file is now checked line by line, any character that isnt a digit, _, ., 0, or whitespace makes the read fail, every row must have exactly 9 cells, and extra content after the 9th row also makes the read fail instead of being accepted with no warning
        std::string line; // instantiates a string named line that holds one line of the file at a time
        int rows = 0; // instantiates an int named rows that tracks how many rows have been read
        bool firstLine = true; // instantiates a bool named firstLine that is true until the first line has been handled, used to remove a byte order mark
        while (std::getline(in, line)) { // loops reading one full line at a time until the file ends
            if (firstLine) { // checks if this is the first line of the file
                firstLine = false; // marks that the first line has been handled
                if (line.size() >= 3 && static_cast<unsigned char>(line[0]) == 0xEF && static_cast<unsigned char>(line[1]) == 0xBB && static_cast<unsigned char>(line[2]) == 0xBF) line.erase(0, 3); // removes the 3 byte order mark that some editors add to the start of a file
            } // ends if statement
            std::string cells; // instantiates a string named cells that holds the line with all whitespace removed
            for (char ch : line) // loops through each character of the line
                if (!std::isspace(static_cast<unsigned char>(ch))) cells += ch; // keeps the character only if it isnt whitespace, this also removes the carriage return from windows line endings
            if (cells.empty()) continue; // skips blank lines
            if (rows >= Board::SIZE) return false; // returns false since there is extra content after the 9th row
            if (static_cast<int>(cells.size()) != Board::SIZE) return false; // returns false since the row doesnt have exactly 9 cells
            for (int col = 0; col < Board::SIZE; ++col) { // loops through each cell in the row
                char ch = cells[col]; // instantiates a char named ch that holds the current character
                if (ch == '_' || ch == '.' || ch == '0') board.set(rows, col, 0); // sets the cell to 0 if the character represents a blank cell
                else if (ch >= '1' && ch <= '9') board.set(rows, col, ch - '0'); // sets the cell to the digit, subtracting '0' converts the character into an int
                else return false; // returns false since the character isnt allowed
            } // ends for loop
            ++rows; // adds 1 to rows since a full row was read
        } // ends while statement
        return rows == Board::SIZE; // returns true only if exactly 9 rows were read
    } // ends read function
}; // ends PuzzleReader class

// ---------------------------------------------------------------------------
// Solver: recursive depth-first search with backtracking. Collects EVERY
// solution (the puzzle may not have a unique one).
// ---------------------------------------------------------------------------
class Solver { // creates a class Solver that finds every solution using backtracking
public: // makes it so everything below can be called by outside functions
    static constexpr std::size_t MAX_SOLUTIONS = 1000; // instantiates a constant named MAX_SOLUTIONS that is the most solutions the solver will store before it stops searching

    // True if the last solveAll stopped early because it hit MAX_SOLUTIONS.
    bool limitReached() const { return limitReached_; } // creates the limitReached function that lets the application know the list of solutions was cut off

    // Solves `puzzle` and returns all solutions found (empty => no solution).
    std::vector<Board> solveAll(const Board& puzzle) { // creates the solveAll function that returns every solution to a puzzle
        solutions_.clear(); // clears any solutions left over from a previous puzzle
        limitReached_ = false; // resets the limit flag from a previous puzzle
        if (!puzzle.isConsistent()) return solutions_;  // given digits already clash // checks if the given numbers already break the rules, if so returns the empty list since theres no solution
        Board work = puzzle; // instantiates a Board named work that is a copy of the puzzle so the original doesnt change
        search(work, 0); // calls search on the copy, starting at cell 0, to start finding solutions
        return std::move(solutions_); // moves the solutions out to the caller instead of copying the whole vector
    } // ends solveAll function




private: // makes it so everything below can only be used inside the class
    // Space complexity fix: storing solutions is now capped at max_solutions (1000 boards is about 324 KB) so a nearly empty puzzle cant exhaust memory, and solveAll moves the vector out instead of copying it

    std::vector<Board> solutions_; // instantiates a vector of Boards named solutions_ that holds each finished board
    bool limitReached_ = false; // instantiates a bool named limitReached_ that becomes true once the solution cap is hit
    // Execution time fix: the worst case is still O(9^m) for m empty cells, but each node is much cheaper now. findEmpty starts scanning at the cell after the last one filled instead of rescanning from cell 0, and the legal digits come in one step instead, the search also stops as soon as the cap is hit.
    // Recursive step: fill the next empty cell with each candidate in turn.
    void search(Board& board, int start) { // creates the search function that fills the next empty cell and then calls itself again, start is the index to begin looking for an empty cell
        int row, col; // instantiates ints named row and col that hold the location of the next empty cell
        if (!board.findEmpty(row, col, start)) {  // checks if there are no empty cells left
            solutions_.push_back(board); // adds the finished board to the list of solutions
            if (solutions_.size() >= MAX_SOLUTIONS) limitReached_ = true; // checks if the cap was reached, if so sets the flag so the search stops
            return; // goes back to the previous call to try other options
        } // ends if statement
        int mask = board.candidateMask(row, col); // instantiates an int named mask that holds a bit for every legal digit in that cell
        for (int value = 1; value <= Board::SIZE; ++value) {  // none => dead end, backtrack // loops through the digits 1 through 9
            if (!(mask & (1 << value))) continue; // skips the digit if its bit isnt on in the mask, meaning it isnt legal there
            board.set(row, col, value); // puts the digit in the cell
            search(board, row * Board::SIZE + col + 1);          // recurse with one more cell filled // calls search again now that the cell is filled, starting just after this cell since everything before it is filled
            board.set(row, col, 0); // undo and try the next candidate // sets the cell back to empty so the next digit can be tried
            if (limitReached_) return; // stops trying digits once the cap was reached
        } // ends for loop
    } // ends search function
}; // ends Solver class

// ---------------------------------------------------------------------------
// Application: drives the whole program (reading, solving, reporting).
// ---------------------------------------------------------------------------
class SudokuApplication { // creates a class SudokuApplication that runs the whole program
public: // makes it so everything below can be called by outside functions
    int run(const std::vector<std::string>& files) { // creates the run function that goes through each puzzle file and returns 1 if any of them couldnt be read
        int status = 0; // instantiates an int named status set to 0, meaning everything has worked so far
        for (const std::string& name : files) { // loops through each filename
            std::cout << "==============================\n"; // prints a line to separate the puzzles
            std::cout << "Puzzle file: " << name << "\n\n"; // prints which file is being used

            Board puzzle; // instantiates an empty Board named puzzle to read the file into
            if (!reader_.read(name, puzzle)) { // checks if the file couldnt be read into a valid board
                std::cout << "Could not read a valid puzzle from this file.\n\n"; // prints an error message
                status = 1; // sets status to 1 so main knows something failed
                continue; // skips to the next file
            } // ends if statement

            std::cout << "Puzzle:\n"; // prints the label for the starting board
            puzzle.print(std::cout); // prints the starting board
            std::cout << '\n'; // prints a blank line after the board

            std::vector<Board> solutions = solver_.solveAll(puzzle); // instantiates a vector of Boards named solutions that holds everything the solver finds
            if (solutions.empty()) { // checks if no solutions were found
                std::cout << "No solution found\n\n"; // prints that there is no solution
            } else { // ends the if statement but runs if at least one solution was found
                std::cout << solutions.size()
                          << (solutions.size() == 1 ? " solution found:\n\n" : " solutions found:\n\n"); // prints how many solutions were found, using singular or plural depending on the count
                for (std::size_t i = 0; i < solutions.size(); ++i) { // loops through each solution
                    if (solutions.size() > 1) std::cout << "Solution " << (i + 1) << ":\n"; // checks if theres more than one solution, if so prints a numbered label
                    else std::cout << "Solution:\n"; // otherwise prints the plain label
                    solutions[i].print(std::cout); // prints the solved board
                    std::cout << '\n'; // prints a blank line after the board
                } // ends for loop
                if (solver_.limitReached()) std::cout << "Stopped after " << Solver::MAX_SOLUTIONS << " solutions, there may be more.\n\n"; // checks if the solver hit its cap, if so prints a note so the list isnt mistaken for every solution
            } // ends else statement
        } // ends for loop
        return status; // returns 0 if every file was read fine, otherwise 1
    } // ends run function

private: // makes it so everything below can only be used inside the class
    PuzzleReader reader_; // instantiates a PuzzleReader named reader_ that loads the puzzle files
    Solver solver_; // instantiates a Solver named solver_ that solves the puzzles
}; // ends SudokuApplication class

int main(int argc, char* argv[]) { // instantiates the main function for running the program
    std::vector<std::string> files; // instantiates a vector of strings named files that holds the filenames
    for (int i = 1; i < argc; ++i) files.push_back(argv[i]); // loops through the command line arguments and adds each one to files, starts at 1 since argv[0] is the program name
    if (files.empty()) // checks if no filenames were given
        for (int i = 1; i <= 5; ++i) files.push_back("puzzle" + std::to_string(i) + ".txt"); // defaults to puzzle1.txt through puzzle5.txt

    SudokuApplication app; // creates the application that does all the work
    return app.run(files); // runs the application and returns its status
} // ends main function