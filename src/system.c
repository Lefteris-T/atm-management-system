#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include "header.h"
#include <math.h>

const char *RECORDS = "./data/records.txt";

int getAccountFromFile(FILE *ptr, struct Record *r)
{
    int fieldsRead = fscanf(ptr, "%d %d %99s %d %d/%d/%d %99s %d %lf %9s",
                            &r->id,
                            &r->userId,
                            r->name,
                            &r->accountNbr,
                            &r->deposit.month,
                            &r->deposit.day,
                            &r->deposit.year,
                            r->country,
                            &r->phone,
                            &r->amount,
                            r->accountType);

    if (fieldsRead == 11)
    {
        return 1;
    }

    /* Distinguish a clean end of file from a malformed or truncated row. */
    if (fieldsRead == EOF && feof(ptr) && !ferror(ptr))
    {
        return 0;
    }

    return -1;
}

int saveAccountToFile(FILE *ptr, const struct Record *r)
{
    return fprintf(ptr, "%d %d %s %d %d/%d/%d %s %d %.2f %s\n",
                   r->id,
                   r->userId,
                   r->name,
                   r->accountNbr,
                   r->deposit.month,
                   r->deposit.day,
                   r->deposit.year,
                   r->country,
                   r->phone,
                   r->amount,
                   r->accountType) >= 0;
}
int rewriteOwnedAccount(int ownerId, int accountId,
                        const struct Record *replacement)
{
    FILE *source = fopen(RECORDS, "r");
    if (source == NULL)
    {
        perror("Opening records.txt");
        return -1;
    }

    char tempPath[] = "./data/records.tmp.XXXXXX";
    int fd = mkstemp(tempPath);
    if (fd == -1)
    {
        perror("Creating temporary file");
        fclose(source);
        return -1;
    }

    FILE *target = fdopen(fd, "w");
    if (target == NULL)
    {
        perror("Opening temporary file");
        close(fd);
        remove(tempPath);
        fclose(source);
        return -1;
    }

    struct Record current = {0};
    int found = 0;
    int failed = 0;

    int readStatus;
    while ((readStatus = getAccountFromFile(source, &current)) == 1)
    {
        if (current.userId == ownerId &&
            current.id == accountId)
        {
            if (found)
            {
                failed = 1; /* Ambiguous duplicate: leave the source intact. */
                break;
            }
            found = 1;

            if (replacement == NULL)
            {
                continue; /* Delete: do not copy this record. */
            }

            if (replacement->id != current.id ||
                replacement->accountNbr != current.accountNbr ||
                replacement->userId != current.userId ||
                strcmp(replacement->name, current.name) != 0 ||
                !saveAccountToFile(target, replacement))
            {
                failed = 1;
                break;
            }
        }
        else if (!saveAccountToFile(target, &current))
        {
            failed = 1;
            break;
        }
    }

    /* A malformed record must not result in a shortened file. */
    if (readStatus != 0)
    {
        failed = 1;
    }

    if (fclose(source) != 0)
    {
        failed = 1;
    }

    if (fclose(target) != 0)
    {
        failed = 1;
    }

    if (failed || !found)
    {
        remove(tempPath);
        return failed ? -1 : 0;
    }

    if (rename(tempPath, RECORDS) != 0)
    {
        perror("Replacing records.txt");
        remove(tempPath);
        return -1;
    }

    return 1;
}

void stayOrReturn(int notGood, void f(struct User u), struct User u)
{
    int option;
    if (notGood == 0)
    {
        system("clear");
        printf("\n✖ Record not found!!\n");
    invalid:
        printf("\nEnter 0 to try again, 1 to return to main menu and 2 to exit:");
        scanf("%d", &option);
        if (option == 0)
            f(u);
        else if (option == 1)
            mainMenu(u);
        else if (option == 2)
            exit(0);
        else
        {
            printf("Insert a valid operation!\n");
            goto invalid;
        }
    }
    else
    {
        printf("\nEnter 1 to go to the main menu and 0 to exit:");
        scanf("%d", &option);
    }
    if (option == 1)
    {
        system("clear");
        mainMenu(u);
    }
    else
    {
        system("clear");
        exit(1);
    }
}

void success(struct User u)
{
    int option;
    printf("\n✔ Success!\n\n");
invalid:
    printf("Enter 1 to go to the main menu and 0 to exit!\n");
    scanf("%d", &option);
    system("clear");
    if (option == 1)
    {
        return;
    }
    else if (option == 0)
    {
        exit(1);
    }
    else
    {
        printf("Insert a valid operation!\n");
        goto invalid;
    }
}

void createNewAcc(struct User u)
{
    struct Record r = {0};
    struct Record cr = {0};
    FILE *pf = fopen(RECORDS, "a+");

    if (pf == NULL)
    {
        perror("records.txt");
        return;
    }

    system("clear");
    printf("\t\t\t===== New record =====\n");

noAccount:
    printf("\nEnter today's date(mm/dd/yyyy):");
    if (scanf("%d/%d/%d",
              &r.deposit.month,
              &r.deposit.day,
              &r.deposit.year) != 3 ||
        r.deposit.month < 1 || r.deposit.month > 12 ||
        r.deposit.day < 1 || r.deposit.day > 31 ||
        r.deposit.year < 1)
    {
        printf("Invalid date.\n");
        fclose(pf);
        return;
    }

    printf("\nEnter the account number:");
    if (scanf("%d", &r.accountNbr) != 1 || r.accountNbr < 0)
    {
        printf("Invalid account number.\n");
        fclose(pf);
        return;
    }

    int maxId = -1;
    rewind(pf);

    int readStatus;
    while ((readStatus = getAccountFromFile(pf, &cr)) == 1)
    {
        if (cr.id > maxId)
        {
            maxId = cr.id;
        }

        if (cr.userId == u.id && cr.accountNbr == r.accountNbr)
        {
            printf("✖ This Account already exists for this user\n\n");
            goto noAccount;
        }
    }

    if (readStatus < 0)
    {
        fprintf(stderr, "Invalid record in records.txt; account was not saved.\n");
        fclose(pf);
        return;
    }

    r.id = maxId + 1;
    r.userId = u.id;
    snprintf(r.name, sizeof r.name, "%s", u.name);

    printf("\nEnter the country:");
    if (scanf("%99s", r.country) != 1)
    {
        printf("Invalid country.\n");
        fclose(pf);
        return;
    }

    printf("\nEnter the phone number:");
    if (scanf("%d", &r.phone) != 1 || r.phone <= 0)
    {
        printf("Invalid phone number.\n");
        fclose(pf);
        return;
    }

    printf("\nEnter amount to deposit: $");
    if (scanf("%lf", &r.amount) != 1 || r.amount < 0.0)
    {
        printf("Invalid deposit amount.\n");
        fclose(pf);
        return;
    }

    printf("\nChoose the type of account:\n"
           "\t-> savings\n"
           "\t-> current\n"
           "\t-> fixed01(for 1 year)\n"
           "\t-> fixed02(for 2 years)\n"
           "\t-> fixed03(for 3 years)\n\n"
           "\tEnter your choice:");

    if (scanf("%9s", r.accountType) != 1)
    {
        printf("Invalid account type input.\n");
        fclose(pf);
        return;
    }

    if (strcmp(r.accountType, "savings") != 0 &&
        strcmp(r.accountType, "current") != 0 &&
        strcmp(r.accountType, "fixed01") != 0 &&
        strcmp(r.accountType, "fixed02") != 0 &&
        strcmp(r.accountType, "fixed03") != 0)
    {
        printf("Invalid account type.\n");
        fclose(pf);
        return;
    }

    if (fseek(pf, 0, SEEK_END) != 0 ||
        !saveAccountToFile(pf, &r))
    {
        perror("Saving account");
        fclose(pf);
        return;
    }

    if (fclose(pf) != 0)
    {
        perror("Closing records.txt");
        return;
    }

    success(u);
}

void checkAllAccounts(struct User u)
{
    struct Record r;

    FILE *pf = fopen(RECORDS, "r");
    if (pf == NULL)
    {
        perror("records.txt");
        return;
    }

    system("clear");
    printf("\t\t====== All accounts from user, %s =====\n\n", u.name);
    int readStatus;
    while ((readStatus = getAccountFromFile(pf, &r)) == 1)
    {
        if (r.userId == u.id)
        {
            printf("_____________________\n");
            printf("\nAccount number:%d\nDeposit Date:%d/%d/%d \ncountry:%s \nPhone number:%d \nAmount deposited: $%.2f \nType Of Account:%s\n",
                   r.accountNbr,
                   r.deposit.month,
                   r.deposit.day,
                   r.deposit.year,
                   r.country,
                   r.phone,
                   r.amount,
                   r.accountType);
        }
    }
    if (readStatus < 0)
    {
        fprintf(stderr, "Invalid record in records.txt; listing stopped.\n");
    }
    fclose(pf);
    success(u);
}
int findOwnedAccount(struct User u, int accountId, struct Record *out)
{
    FILE *pf = fopen(RECORDS, "r");
    struct Record current = {0};

    if (pf == NULL)
    {
        perror("records.txt");
        return -1;
    }

    int readStatus;
    while ((readStatus = getAccountFromFile(pf, &current)) == 1)
    {
        if (current.userId == u.id &&
            current.id == accountId)
        {
            *out = current;
            fclose(pf);
            return 1;
        }
    }

    fclose(pf);
    return readStatus == 0 ? 0 : -1;
}
void showAccountInterest(struct Record r)
{
    if (strcmp(r.accountType, "current") == 0)
    {
        printf("You will not get interests because the account is of type current\n");
        return;
    }

    if (strcmp(r.accountType, "savings") == 0 ||
        strcmp(r.accountType, "saving") == 0)
    {
        double interest = r.amount * 0.07 / 12.0;

        printf("You will get $%.2f as interest on day %d of every month\n",
               interest, r.deposit.day);
        return;
    }
    double rate = 0.0;
    int years = 0;

    if (strcmp(r.accountType, "fixed01") == 0)
    {
        rate = 0.04;
        years = 1;
    }
    else if (strcmp(r.accountType, "fixed02") == 0)
    {
        rate = 0.05;
        years = 2;
    }
    else if (strcmp(r.accountType, "fixed03") == 0)
    {
        rate = 0.08;
        years = 3;
    }

    if (years > 0)
    {
        double interest = r.amount * rate * years;

        printf("You will get $%.2f as interest on %02d/%02d/%04d\n",
               interest,
               r.deposit.month,
               r.deposit.day,
               r.deposit.year + years);
    }
}
void updateAccount(struct User u)
{
    int accountId;
    struct Record record;

    printf("Enter the account ID to update: ");
    if (scanf("%d", &accountId) != 1)
    {
        printf("Invalid account ID.\n");
        return;
    }

    if (!findOwnedAccount(u, accountId, &record))
    {
        printf("Account not found or it does not belong to you.\n");
        return;
    }

    int choice;

    printf("What do you want to update?\n");
    printf("1. Country\n");
    printf("2. Phone number\n");
    printf("Choice: ");

    if (scanf("%d", &choice) != 1)
    {
        printf("Invalid choice.\n");
        return;
    }

    if (choice == 1)
    {
        printf("New country: ");
        if (scanf("%99s", record.country) != 1)
        {
            printf("Invalid country.\n");
            return;
        }
    }
    else if (choice == 2)
    {
        printf("New phone number: ");
        if (scanf("%d", &record.phone) != 1 || record.phone <= 0)
        {
            printf("Invalid phone number.\n");
            return;
        }
    }
    else
    {
        printf("Invalid choice. Nothing was changed.\n");
        return;
    }

    int result = rewriteOwnedAccount(u.id, accountId, &record);

    if (result == 1)
        printf("Account updated successfully.\n");
    else
        printf("Could not update the account.\n");
}

static void clearInputLine(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
    {
    }
}
void makeTransaction(struct User u)
{
    int accountId;
    struct Record record;

    printf("Enter the account ID: ");
    if (scanf("%d", &accountId) != 1)
    {
        clearInputLine();
        printf("Invalid account ID.\n");
        return;
    }

    if (findOwnedAccount(u, accountId, &record) != 1)
    {
        printf("Account not found or it does not belong to you.\n");
        return;
    }

    if (strcmp(record.accountType, "fixed01") == 0 ||
        strcmp(record.accountType, "fixed02") == 0 ||
        strcmp(record.accountType, "fixed03") == 0)
    {
        printf("Transactions are not allowed for fixed accounts.\n");
        return;
    }

    int choice;
    double amount;

    printf("Balance: $%.2f\n", record.amount);
    printf("1. Deposit\n2. Withdraw\nChoice: ");

    if (scanf("%d", &choice) != 1)
    {
        clearInputLine();
        printf("Invalid transaction choice.\n");
        return;
    }

    if (choice != 1 && choice != 2)
    {
        printf("Invalid transaction choice.\n");
        return;
    }

    printf("Amount: $");
    if (scanf("%lf", &amount) != 1)
    {
        clearInputLine();
        printf("Invalid amount.\n");
        return;
    }

    if (!isfinite(amount) || amount <= 0)
    {
        printf("Invalid amount.\n");
        return;
    }

    if (choice == 2 && amount > record.amount)
    {
        printf("Insufficient funds.\n");
        return;
    }

    double newBalance = choice == 1
                            ? record.amount + amount
                            : record.amount - amount;

    if (!isfinite(newBalance))
    {
        printf("Invalid resulting balance.\n");
        return;
    }

    record.amount = newBalance;

    if (rewriteOwnedAccount(u.id, accountId, &record) != 1)
    {
        printf("Transaction could not be saved.\n");
        return;
    }

    printf("Transaction saved. New balance: $%.2f\n", record.amount);
}
