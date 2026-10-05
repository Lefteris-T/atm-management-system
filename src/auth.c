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
void registerMenu(void)
{
    struct User newUser;
    struct User existingUser;
    int maxId = -1;
    FILE *fp = fopen(USERS, "a+");

    if (fp == NULL)
    {
        perror("users.txt");
        return;
    }

    printf("\nChoose a user name: ");
    if (scanf("%49s", newUser.name) != 1)
    {
        fclose(fp);
        return;
    }

    rewind(fp);

    while (fscanf(fp, "%d %49s %49s",
                  &existingUser.id,
                  existingUser.name,
                  existingUser.password) == 3)
    {
        if (existingUser.id > maxId)
        {
            maxId = existingUser.id;
        }

        if (strcmp(existingUser.name, newUser.name) == 0)
        {
            printf("This user name already exists.\n");
            fclose(fp);
            return;
        }
    }

    printf("Choose a password: ");
    if (scanf("%49s", newUser.password) != 1)
    {
        fclose(fp);
        return;
    }

    newUser.id = maxId + 1;

    if (fseek(fp, 0, SEEK_END) != 0 ||
        fprintf(fp, "%d %s %s\n",
                newUser.id,
                newUser.name,
                newUser.password) < 0)
    {
        perror("Saving user");
        fclose(fp);
        return;
    }

    if (fclose(fp) != 0)
    {
        perror("Closing users.txt");
        return;
    }

    printf("Registration successful. Please log in.\n");
}