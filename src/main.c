#include "header.h"

void mainMenu(struct User u)
{
    int option;
    system("clear");

    while (1)
    {
        printf("\n\n\t\t======= ATM =======\n\n");
        printf("\n\t\t-->> Feel free to choose one of the options below <<--\n");
        printf("\n\t\t[1]- Create a new account\n");
        printf("\n\t\t[2]- Update account information\n");
        printf("\n\t\t[3]- Check accounts\n");
        printf("\n\t\t[4]- Check list of owned account\n");
        printf("\n\t\t[5]- Make Transaction\n");
        printf("\n\t\t[6]- Remove existing account\n");
        printf("\n\t\t[7]- Transfer ownership\n");
        printf("\n\t\t[8]- Exit\n");
        scanf("%d", &option);

        switch (option)
        {
        case 1:
            createNewAcc(u);
            break;
        case 2:
            // student TODO : add your **Update account information** function
            // here
            break;
        case 3:
        {
            int requestedAccountNbr;
            struct Record found;

            printf("\nEnter the account number: ");

            int inputResult = scanf("%d", &requestedAccountNbr);

            if (inputResult == EOF)
            {
                return;
            }

            if (inputResult != 1)
            {
                int ch;
                while ((ch = getchar()) != '\n' && ch != EOF)
                {
                }

                printf("Invalid account number.\n");
            }
            else if (findOwnedAccount(u, requestedAccountNbr, &found))
            {
                printf("\nAccount number: %d\n"
                       "Owner: %s\n"
                       "Deposit date: %02d/%02d/%04d\n"
                       "Country: %s\n"
                       "Phone: %d\n"
                       "Balance: $%.2f\n"
                       "Account type: %s\n",
                       found.accountNbr,
                       found.name,
                       found.deposit.month,
                       found.deposit.day,
                       found.deposit.year,
                       found.country,
                       found.phone,
                       found.amount,
                       found.accountType);

                showAccountInterest(found);
            }
            else
            {
                printf("Account not found or not owned by you.\n");
            }

            while (1)
            {
                int nextOption;

                printf("\nEnter 1 for the main menu or 0 to exit: ");

                int nextInput = scanf("%d", &nextOption);

                if (nextInput == EOF)
                {
                    return;
                }

                if (nextInput != 1)
                {
                    int ch;
                    while ((ch = getchar()) != '\n' && ch != EOF)
                    {
                    }

                    printf("Invalid operation!\n");
                    continue;
                }

                if (nextOption == 1)
                {
                    break;
                }

                if (nextOption == 0)
                {
                    return;
                }

                printf("Invalid operation!\n");
            }

            break;
        }
        break;
        case 4:
            checkAllAccounts(u);
            break;
        case 5:
            // student TODO : add your **Make transaction** function
            // here
            break;
        case 6:
            // student TODO : add your **Remove existing account** function
            // here
            break;
        case 7:
            // student TODO : add your **Transfer owner** function
            // here
            break;
        case 8:
            exit(1);
            break;
        default:
            printf("Invalid operation!\n");
        }
    }
};

void initMenu(struct User *u)
{
    int r = 0;
    int option;

    system("clear");
    printf("\n\n\t\t======= ATM =======\n");
    printf("\n\t\t-->> Feel free to login / register :\n");

    while (!r)
    {
        printf("\n\t\t[1]- login\n");
        printf("\n\t\t[2]- register\n");
        printf("\n\t\t[3]- exit\n");
        int inputResult = scanf("%d", &option);

        if (inputResult == EOF)
        {
            exit(0);
        }

        if (inputResult != 1)
        {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF)
            {
            }
            printf("Insert a valid operation!\n");
            continue;
        }

        switch (option)
        {
        case 1:
            if (!loginMenu(u->name, u->password))
            {
                printf("\nLogin input was not completed.\n");
                exit(1);
            }

            if (authenticateUser(u))
            {
                printf("\n\nPassword Match!");
                r = 1;
            }
            else
            {
                printf("\nWrong password!! or User Name\n");
                exit(1);
            }
            break;

        case 2:
            registerMenu();
            break;

        case 3:
            exit(0);

        default:
            printf("Insert a valid operation!\n");
            break;
        }
    }
}
int main(void)
{
    struct User u;

    initMenu(&u);
    mainMenu(u);
    return 0;
}