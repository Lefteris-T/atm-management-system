#include <termios.h>
#include "header.h"

char *USERS = "./data/users.txt";

int loginMenu(char a[50], char pass[50])
{
    struct termios oflags, nflags;
    int passwordRead;

    system("clear");
    printf("\n\n\n\t\t\t\t   Bank Management System\n\t\t\t\t\t User Login:");
    if (scanf("%49s", a) != 1)
        return 0;

    if (tcgetattr(fileno(stdin), &oflags) != 0)
    {
        perror("tcgetattr");
        return 0;
    }
    nflags = oflags;
    nflags.c_lflag &= ~ECHO;
    nflags.c_lflag |= ECHONL;

    if (tcsetattr(fileno(stdin), TCSANOW, &nflags) != 0)
    {
        perror("tcsetattr");
        return 0;
    }
    printf("\n\n\n\n\n\t\t\t\tEnter the password to login:");
    passwordRead = scanf("%49s", pass);

    // Restore echo even when password input ends early.
    if (tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0)
    {
        perror("tcsetattr");
        return 0;
    }
    return passwordRead == 1;
}

int authenticateUser(struct User *u)
{
    FILE *fp = fopen("./data/users.txt", "r");
    struct User userChecker;

    if (fp == NULL)
    {
        perror("users.txt");
        return 0;
    }

    while (fscanf(fp, "%d %49s %49s",
                  &userChecker.id,
                  userChecker.name,
                  userChecker.password) == 3)
    {
        if (strcmp(userChecker.name, u->name) == 0 &&
            strcmp(userChecker.password, u->password) == 0)
        {
            u->id = userChecker.id;
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}