#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <conio.h>

#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define RESET   "\033[0m"

typedef struct {
    int acc_no;
    int pin;
    float balance;
} Account;

Account accounts[100];
int total_accounts = 0, current_index = -1;

void sleepMs(int ms) { Sleep(ms); }

void showLoading(const char *message) {
    printf(YELLOW "\n%s" RESET, message);
    for (int i = 0; i < 3; i++) {
        printf(".");
        fflush(stdout);
        sleepMs(500);
    }
    printf("\n");
}

int getMaskedPIN() {
    int pin = 0;
    char ch;
    while ((ch = getch()) != '\r') {
        if (ch >= '0' && ch <= '9') {
            pin = pin * 10 + (ch - '0');
            printf("*");
        }
    }
    printf("\n");
    return pin;
}

void loadAccounts() {
    FILE *fp = fopen("accounts.txt", "r");
    if (fp == NULL) {
        fp = fopen("accounts.txt", "w");
        fclose(fp);
        return;
    }
    while (fscanf(fp, "%d %d %f", &accounts[total_accounts].acc_no,
                  &accounts[total_accounts].pin, &accounts[total_accounts].balance) != EOF) {
        total_accounts++;
    }
    fclose(fp);
}

void saveAccounts() {
    FILE *fp = fopen("accounts.txt", "w");
    for (int i = 0; i < total_accounts; i++) {
        fprintf(fp, "%d %d %.2f\n", accounts[i].acc_no, accounts[i].pin, accounts[i].balance);
    }
    fclose(fp);
}

int login(int acc, int pin) {
    for (int i = 0; i < total_accounts; i++) {
        if (accounts[i].acc_no == acc && accounts[i].pin == pin) {
            current_index = i;
            return 1;
        }
    }
    return 0;
}

void showDenomination(float amount) {
    int notes[] = {2000, 500, 200, 100, 50, 20, 10, 5, 1};
    int count;
    printf(YELLOW "Currency Denomination:\n" RESET);
    for (int i = 0; i < 9; i++) {
        count = amount / notes[i];
        if (count > 0) {
            printf("₹%d x %d\n", notes[i], count);
            amount -= count * notes[i];
        }
    }
}

void createAccount() {
    int new_acc, new_pin;
    float new_bal;
    printf("Enter new Account Number: ");
    scanf("%d", &new_acc);
    for (int i = 0; i < total_accounts; i++) {
        if (accounts[i].acc_no == new_acc) {
            printf(RED "Account already exists!\n" RESET);
            return;
        }
    }
    printf("Enter PIN: ");
    new_pin = getMaskedPIN();
    printf("Enter Opening Balance (min ₹500): ₹");
    scanf("%f", &new_bal);
    if (new_bal < 500) {
        printf(RED "Minimum ₹500 required to open account.\n" RESET);
        return;
    }

    accounts[total_accounts].acc_no = new_acc;
    accounts[total_accounts].pin = new_pin;
    accounts[total_accounts].balance = new_bal;
    total_accounts++;
    saveAccounts();
    printf(GREEN "Account created successfully!\n" RESET);
}

void deleteAccount() {
    int acc, found = 0;
    printf("Enter account number to delete: ");
    scanf("%d", &acc);

    for (int i = 0; i < total_accounts; i++) {
        if (accounts[i].acc_no == acc) {
            found = 1;
            for (int j = i; j < total_accounts - 1; j++) {
                accounts[j] = accounts[j + 1];
            }
            total_accounts--;
            saveAccounts();
            printf(GREEN "Account deleted successfully.\n" RESET);
            break;
        }
    }
    if (!found) {
        printf(RED "Account not found.\n" RESET);
    }
}

int adminMenu() {
    int choice;
    do {
        printf("\n--- ADMIN MENU ---\n");
        printf("1. Create New Account\n");
        printf("2. View All Accounts\n");
        printf("3. Delete Account\n");
        printf("4. Exit to Login\n");
        printf("Choose: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: createAccount(); break;
            case 2:
                printf("\n%-12s %-6s %-10s\n", "Account No", "PIN", "Balance");
                for (int i = 0; i < total_accounts; i++) {
                    printf("%-12d %-6d ₹%.2f\n", accounts[i].acc_no, accounts[i].pin, accounts[i].balance);
                }
                break;
            case 3: deleteAccount(); break;
            case 4:
                printf(GREEN "Returning to login...\n" RESET);
                return 0;
            default: printf(RED "Invalid choice.\n" RESET);
        }
    } while (1);
}

int atmMenu() {
    int choice;
    float amount;

    do {
        printf("\n--- ATM MENU ---\n");
        printf("1. Check Balance\n");
        printf("2. Deposit\n");
        printf("3. Withdraw\n");
        printf("4. Change PIN\n");
        printf("5. Calculate Interest\n");
        printf("6. Exit to Login\n");
        printf("Choose: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf(YELLOW "Your balance: ₹%.2f\n" RESET, accounts[current_index].balance);
                break;
            case 2:
                printf("Amount to deposit: ₹");
                scanf("%f", &amount);
                if (amount > 0) {
                    accounts[current_index].balance += amount;
                    printf(GREEN "Deposit successful.\n" RESET);
                } else {
                    printf(RED "Invalid amount.\n" RESET);
                }
                break;
            case 3:
                printf("Amount to withdraw: ₹");
                scanf("%f", &amount);
                if (amount > 0 && amount <= accounts[current_index].balance - 500) {
                    accounts[current_index].balance -= amount;
                    printf(GREEN "Withdrawal successful.\n" RESET);
                    showDenomination(amount);
                } else {
                    printf(RED "Withdrawal denied: Minimum ₹500 balance must remain.\n" RESET);
                }
                break;
            case 4: {
                int old_pin, new_pin;
                while (1) {
                    printf("Enter current PIN: ");
                    old_pin = getMaskedPIN();
                    if (old_pin == accounts[current_index].pin) {
                        printf("Enter new PIN: ");
                        new_pin = getMaskedPIN();
                        accounts[current_index].pin = new_pin;
                        printf(GREEN "PIN changed successfully.\n" RESET);
                        break;
                    } else {
                        printf(RED "Incorrect PIN. Please try again.\n" RESET);
                    }
                }
                break;
            }
            case 5: {
                float rate = 4.0;
                float principal = accounts[current_index].balance;
                int years;
                printf("Enter number of years: ");
                scanf("%d", &years);
                float interest = (principal * rate * years) / 100.0;
                printf(YELLOW "Projected balance after %d year(s): ₹%.2f\n" RESET, years, principal + interest);
                break;
            }
            case 6:
                printf(GREEN "Returning to login...\n" RESET);
                saveAccounts();
                return 0;
            default: printf(RED "Invalid choice.\n" RESET);
        }
    } while (1);
}

int main() {
    char input[20];
    int acc_no, pin;

    loadAccounts();

    while (1) {
        printf(BLUE "\n===== Welcome to ATM System =====\n" RESET);
        printf("Enter Account Number (or 0 for Admin, or press Enter to Exit): ");

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;

        if (strlen(input) == 0) {
            printf(GREEN "Exiting ATM. Goodbye!\n" RESET);
            break;
        }

        acc_no = atoi(input);

        if (acc_no == 0) {
            printf("Enter Admin Password: ");
            int admin_pass = getMaskedPIN();
            if (admin_pass == 9999) {
                showLoading("Loading admin panel");
                adminMenu();
            } else {
                printf(RED "Invalid admin password.\n" RESET);
            }
            continue;
        }

        printf("Enter PIN: ");
        pin = getMaskedPIN();

        if (login(acc_no, pin)) {
            printf(GREEN "Login successful!\n" RESET);
            showLoading("Loading your account");
            atmMenu();
        } else {
            printf(RED "Invalid credentials.\n" RESET);
        }
    }

    return 0;
}

