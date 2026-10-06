#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Date
{
    int month, day, year;
};

// all fields for each record of an account
struct Record
{
    int id;
    int userId;
    char name[100];
    char country[100];
    int phone;
    char accountType[10];
    int accountNbr;
    double amount;
    struct Date deposit;
    struct Date withdraw;
};

struct User
{
    int id;
    char name[50];
    char password[50];
};

// authentication functions
int loginMenu(char a[50], char pass[50]);
void registerMenu(void);
int authenticateUser(struct User *u);
int findOwnedAccount(struct User u, int accountNbr, struct Record *out);

// system function
void createNewAcc(struct User u);
void mainMenu(struct User u);
void checkAllAccounts(struct User u);
void showAccountInterest(struct Record r);
void makeTransaction(struct User u);
/* Update one owned account, or delete it when replacement is NULL.
 * Returns 1 on success, 0 when no such owned account exists,
 * and -1 when reading or rewriting storage fails. */
int rewriteOwnedAccount(int ownerId, int accountId,
                        const struct Record *replacement);
void updateAccount(struct User u);
void removeAccount(struct User u);
