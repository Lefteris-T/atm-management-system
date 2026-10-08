# ATM Management System

A terminal based ATM and account management program written in C. It stores user and account records under `data/` and supports account ownership transfers.

## Build and run

Requirements: a C compiler, `make`, and the libsodium development package.

```sh
make
./atm
```

The program reads and writes `data/users.txt` and `data/records.txt` relative to its current working directory, so run it from the project directory. Use `make clean` to remove compiled objects and the executable.

## B4: transfer notifications

When a user transfers an account, the program first saves the new owner in `data/records.txt`. If the recipient is currently logged in, their terminal receives a message such as:

```text
[Notification] Alice transferred account 3212 to you.
```

Each logged-in process listens on a named pipe (`data/notify-<user-id>.fifo`). The user ID in the filename routes the message to that user's session. A per-user lock file (`data/notify-<user-id>.lock`) prevents two sessions for the same user from claiming the same notification channel. These files are created at login and the FIFO is removed when the session exits.

Notifications are best effort and are not queued. If the recipient is offline or the notification channel cannot be reached, the ownership transfer remains saved and the sender is told that the notification was unavailable. A later login will not receive missed notifications.

To check live delivery, start two program instances from the project directory in separate terminals and log in as different users. From the sender, transfer an account to the logged-in recipient. The recipient should see the alert; the account owner should also reflect the transfer in the records file. Repeat with the recipient logged out to check that the transfer still completes without a delivered alert.

## Project notes

Optional project phases and their audit checklist are documented in [`docs/tasks.md`](docs/tasks.md). Account and user files in `data/` contain the local application data; keep a backup before manually editing them.
