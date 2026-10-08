#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <termios.h>
#include "header.h"
#include <sodium.h>

char *USERS = "./data/users.txt";
static int migratePlaintextUser(int targetId,
                                const char *targetName,
                                const char *plainPassword)
{
    char hash[crypto_pwhash_STRBYTES];

    if (crypto_pwhash_str(hash, plainPassword, strlen(plainPassword),
                          crypto_pwhash_OPSLIMIT_INTERACTIVE,
                          crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0)
        return 0;

    FILE *source = fopen(USERS, "r");
    if (source == NULL)
        return 0;

    char tempPath[] = "./data/users.tmp.XXXXXX";
    int fd = mkstemp(tempPath);
    if (fd == -1)
    {
        fclose(source);
        return 0;
    }

    FILE *target = fdopen(fd, "w");
    if (target == NULL)
    {
        close(fd);
        remove(tempPath);
        fclose(source);
        return 0;
    }

    struct User row = {0};
    char savedPassword[crypto_pwhash_STRBYTES];
    int fieldsRead;
    int found = 0;
    int failed = 0;

    while ((fieldsRead = fscanf(source, "%d %49s %127s",
                                &row.id, row.name, savedPassword)) == 3)
    {
        const char *passwordToWrite = savedPassword;

        if (row.id == targetId && strcmp(row.name, targetName) == 0)
        {
            if (found || strcmp(savedPassword, plainPassword) != 0)
            {
                failed = 1;
                break;
            }

            passwordToWrite = hash;
            found = 1;
        }

        if (fprintf(target, "%d %s %s\n",
                    row.id, row.name, passwordToWrite) < 0)
        {
            failed = 1;
            break;
        }
    }

    if (fieldsRead != EOF || !feof(source) || ferror(source))
        failed = 1;

    if (fclose(source) != 0)
        failed = 1;

    if (fclose(target) != 0)
        failed = 1;

    if (failed || !found)
    {
        remove(tempPath);
        return 0;
    }

    if (rename(tempPath, USERS) != 0)
    {
        remove(tempPath);
        return 0;
    }

    return 1;
}

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
    char storedPassword[crypto_pwhash_STRBYTES];

    if (fp == NULL)
    {
        perror("users.txt");
        return 0;
    }

    while (fscanf(fp, "%d %49s %127s",
                  &userChecker.id,
                  userChecker.name,
                  storedPassword) == 3)
    {
        if (strcmp(userChecker.name, u->name) != 0)
            continue;

        int passwordMatches;

        if (strncmp(storedPassword, crypto_pwhash_STRPREFIX,
                    strlen(crypto_pwhash_STRPREFIX)) == 0)
        {
            passwordMatches =
                crypto_pwhash_str_verify(storedPassword,
                                         u->password,
                                         strlen(u->password)) == 0;
        }
        else
        {
            /* Existing plaintext entry; migration comes later. */
            passwordMatches = strcmp(storedPassword, u->password) == 0;
        }

        if (passwordMatches)
        {
            int wasPlaintext =
                strncmp(storedPassword, crypto_pwhash_STRPREFIX,
                        strlen(crypto_pwhash_STRPREFIX)) != 0;

            u->id = userChecker.id;
            fclose(fp);

            if (wasPlaintext &&
                !migratePlaintextUser(u->id, u->name, u->password))
            {
                fprintf(stderr,
                        "Login succeeded, but the password upgrade was not saved.\n");
            }

            sodium_memzero(u->password, sizeof u->password);
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
    char storedPassword[crypto_pwhash_STRBYTES];
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

    while (fscanf(fp, "%d %49s %127s",
                  &existingUser.id,
                  existingUser.name,
                  storedPassword) == 3)
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
    char passwordHash[crypto_pwhash_STRBYTES];

    if (crypto_pwhash_str(passwordHash,
                          newUser.password,
                          strlen(newUser.password),
                          crypto_pwhash_OPSLIMIT_INTERACTIVE,
                          crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0)
    {
        sodium_memzero(newUser.password, sizeof newUser.password);
        fprintf(stderr, "Could not hash password.\n");
        fclose(fp);
        return;
    }

    sodium_memzero(newUser.password, sizeof newUser.password);

    newUser.id = maxId + 1;

    if (fseek(fp, 0, SEEK_END) != 0 ||
        fprintf(fp, "%d %s %s\n",
                newUser.id,
                newUser.name,
                passwordHash) < 0)
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