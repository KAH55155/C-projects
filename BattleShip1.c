#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

void Initialization() {
    printf("Welcome to Battleship!\n");
}

void Teardown() {
    printf("Thanks for playing!\n");
}

void AcceptInput(char *letter, int *number) {
    char input[10];

    printf("Please pick a letter A-J and a number 0-9 as your move!, OR type Q to exit the game!: ");
    fgets(input, sizeof(input), stdin);

    
    input[0] = toupper(input[0]);
    if(input[0] == 'Q') {
        *letter = input[0];
        return;
    }
    //Ensures input is 2 characters long and that a digit is the second character
    if (strlen(input) > 3 || !isdigit(input[1]) || input[0] >= 'K') {
        printf("INVALID MOVE BUDDY, try again :) .\n");
        AcceptInput(letter, number);
        return;
    }
    else {
        *number = input[1] - '0';
        *letter = input[0];
    }
}

//If the number is divisible by 2 it is a hit, if not its a miss 
bool UpdateState(char letter, int number) {
    if (number % 2 == 0) {
        return true;
    } else {
        return false;
    }
}

void DisplayWorld(bool Hit) {
    if (Hit) {
        printf("It is a HIT!\n");
    } else {
        printf("It is a MISS!\n");
    }
}

int main() {
    
    Initialization();
    
    bool gameOver = false;
    char letter;
    int number;

    //This loops the question and also gives the user to exit the game
     while (!gameOver) {
        AcceptInput(&letter, &number);
        if (letter == 'Q') { 
            gameOver = true;
            break;
        }
        bool Hit = UpdateState(letter, number);
        DisplayWorld(Hit);
    }
    Teardown();
    return 0;
}
