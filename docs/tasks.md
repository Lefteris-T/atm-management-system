# ATM System — Implementation Tasks

Source: [exercise.md](exercise.md), [audit.md](audit.md), and the supplied starter code. This is a plan; unchecked items are not implemented.

## Working method

- Work through one small behavior at a time: understand it → try it yourself → check it → fix it → commit it.
- Back up `data/users.txt` and `data/records.txt`. Use separate copies for checks so sample data stays intact.
- For each feature, check success, rejection, and persistence after restarting. Commit only when `make` succeeds and that commit's behavior works.
- Keep commits focused. Use a `test:` commit only when it contains actual tests; record manual checks with the feature commit.
- `Record.id` and `accountNbr` are different. The audit's entered “account ID” is `accountNbr`; use it for account prompts and lookups. Keep `Record.id` unique in storage.
- Use the logged-in `User.id` for ownership. A name or account number alone is insufficient.
- Complete the required work and its audit before starting bonus features.

## Required: make the starter usable

### Phase 0 — Compile the starter

- [ ] Fix the compilation errors in `saveAccountToFile`: struct values versus pointers, missing separators, and `fprintf` arguments that match its format.
- [ ] Run `make`. Account creation cannot be checked reliably until login and user loading are repaired.

**Learn:** `.` versus `->`, `&`, and matching format specifiers to arguments.  
**Commit:** `fix: build starter application`

### Phase 1 — Read users and log in correctly

- [ ] Read each user as `id name password` and accept a row only when all three fields were read. Check file-open failures.
- [ ] Replace the unsafe `getPassword` result with a login lookup that fills the logged-in `User`, including `id`; never return a pointer into a local variable.
- [ ] Bound login string input to the destination buffers and handle invalid numeric input on the initial menu.
- [ ] Check known user, wrong password, unknown user, and successful login followed by the main menu.

**Learn:** local-variable lifetime, `fscanf` results, buffer bounds, and complete user identity.  
**Commits:** `fix: read complete user records`; `fix: load authenticated user on login`

### Phase 2 — Read, create, and list valid accounts

- [ ] Read a record only when every expected field was parsed; check record-file open failures and bound string input.
- [ ] Give each new record a unique `Record.id`, even if old IDs have gaps. Save the logged-in `User.id` as `userId` and keep the expected field order.
- [ ] Check account-number uniqueness for the same owner while allowing the same number for different owners. Validate creation inputs needed to save a usable account.
- [ ] Create an account, inspect its saved line, list it, and confirm it remains after restarting.

**Learn:** record parsing, unique IDs, append mode, and record ID versus account number.  
**Commits:** `fix: read complete account records`; `fix: save valid owned accounts`

## Required: new features

### Phase 3 — Register users

- [ ] Allocate a unique user ID even if older IDs have gaps.
- [ ] Reject a duplicate name; write `id name password` to `users.txt`.
- [ ] Connect registration to the initial menu.
- [ ] Check new user, duplicate user, login with new credentials, and persistence after restart. Confirm names remain unique in storage.

**Learn:** file search, uniqueness, append, and user record format.  
**Commit:** `feat: register unique users`

### Phase 4 — Find an owned account

- [ ] Write a reusable lookup using `accountNbr` and the logged-in `User.id`.
- [ ] Check owned, missing, and other users' accounts; also check the same `accountNbr` under two different owners.
- [ ] Make sure create and list follow the same ownership rule.

**Learn:** an account number by itself does not prove ownership.  
**Commit:** `feat: find accounts owned by signed-in user`

### Phase 5 — Show one account and interest

- [ ] Ask for one account number and show only that owned account, or an error if it is missing.
- [ ] Show details and interest for savings at 7% per year, `fixed01` at 4% for one year, `fixed02` at 5% for two years, and `fixed03` at 8% for three years. Show the required no-interest message for `current`.
- [ ] Support the starter data's `saving` spelling and the exercise's `savings` spelling. Choose one consistent spelling for newly created accounts.
- [ ] Match the audit's $1001.20 examples dated 10/10/2012: $5.84 on day 10 each month; $40.05 on 10/10/2013; $100.12 on 10/10/2014; $240.29 on 10/10/2015. Also check the exercise's $1023.20 savings example ($5.97 monthly).

**Learn:** annual rates, monthly amounts, term dates, and two-decimal output.  
**Commit:** `feat: show one account with interest`

### Phase 6 — Save changes to existing accounts

- [ ] Add a small, reusable way to update or remove one owned record: read records, write a temporary file, check errors, then replace the original only after a successful write.
- [ ] Preserve all other records, IDs, and field order. A missing or unowned account must leave storage unchanged.
- [ ] Check a successful rewrite and a rejected rewrite with multiple users' records present.

**Learn:** append mode cannot update or delete an existing record.  
**Commit:** `feat: safely rewrite account records`

### Phase 7 — Update country or phone

- [ ] Ask for an account number, then offer only `country` and `phone` as fields to change.
- [ ] Change only the selected field on an owned account and save it.
- [ ] Check both choices, a nonexistent or unowned account, and persistence after restart in the application and `records.txt`.

**Learn:** change one field without changing other records.  
**Commit:** `feat: update owned account country or phone`

### Phase 8 — Deposit and withdraw

- [ ] Ask for an owned account, transaction type, and positive amount. Reject invalid amounts and withdrawals above the balance.
- [ ] Reject deposits and withdrawals on `fixed01`, `fixed02`, and `fixed03` with an error; leave storage unchanged.
- [ ] Save the balance of eligible accounts and check both transaction types after restart.

**Learn:** validate before changing data.  
**Commit:** `feat: deposit and withdraw from eligible accounts`

### Phase 9 — Delete an account

- [ ] Delete only an owned account from `records.txt`.
- [ ] Check nonexistent and unowned accounts, confirm other records remain, and confirm deletion in the application and storage after restart.

**Learn:** rewrite all records except the selected one.  
**Commit:** `feat: remove owned account`

### Phase 10 — Transfer ownership

- [ ] Ask for an owned account and an existing recipient. Reject unknown users and self-transfer.
- [ ] Save both the recipient's `userId` and `name`. Reject an `accountNbr` collision if the recipient already has that number.
- [ ] Check that the sender loses access, the recipient gains access after login, and other users cannot transfer the account. Confirm the stored record changed.

**Learn:** update all fields that represent the same relationship.  
**Commit:** `feat: transfer account to existing user`

### Phase 11 — Required-feature audit

- [ ] Run `make clean && make` and any automated tests that exist.
- [ ] Follow [audit.md](audit.md) using separate test data: register Marcus; reject duplicate Alice; log in as Alice; create and list accounts; update phone and country; check each interest example; reject fixed-account transactions; withdraw, reject overdraft, and deposit; delete accounts; transfer the remaining account to Michel.
- [ ] At each relevant step, verify the application and storage agree. Restart to check persistence. Check nonexistent and unowned account paths, file format, and that test data or binaries are not committed.
- [ ] Record any audit discrepancy and fix it with a focused commit before considering the required work complete.

**Commit:** only if this phase adds tests or useful documentation; otherwise use it as a final verification checklist.

## Bonus — only after the required-feature audit passes

These are optional, independent tracks. Pick one at a time. After each bonus, rerun the relevant required flows so the bonus does not break audit behavior.

### Bonus B1 — Improve the Makefile

- [ ] Fix source/header dependencies so changing `src/header.h` rebuilds the affected objects.
- [ ] Add or verify `clean`; add a `test` target only if actual automated tests exist.
- [ ] Check a clean build, an incremental build, and cleanup. Keep generated files out of commits.

**Learn:** targets, prerequisites, and incremental builds.  
**Commit:** `build: correct Makefile dependencies` (add a separate commit for a real `test` target if needed)

### Bonus B2 — Improve the terminal interface

- [ ] Choose one small navigation improvement, such as returning to a menu after an invalid choice or showing a consistent error message.
- [ ] Apply it to the initial and main menus, then check normal and invalid input paths.
- [ ] Improve prompts and account displays for readability without changing the stored format or required behavior.
- [ ] Repeat the manual required-feature flow after the interface changes.

**Learn:** predictable control flow and input handling.  
**Commits:** `ui: make menu navigation consistent`; `ui: clarify account prompts and displays`

### Bonus B3 — Protect stored passwords

- [ ] Choose a maintained password-hashing library and document its build dependency and stored-hash format. Use its password-hashing API and per-password salts.
- [ ] Update registration to store hashes and login to verify them. Check correct and wrong passwords and restart persistence.
- [ ] Decide how existing plaintext `users.txt` entries will migrate without locking out sample users; implement and check that migration.
- [ ] Confirm newly stored passwords are not readable as plaintext, and rerun registration and login audit checks.

**Learn:** password verification, data migration, and compatibility.  
**Commits:** `feat: hash passwords for new users`; `feat: migrate existing plaintext passwords`

### Bonus B4 — Notify a recipient immediately after transfer

- [ ] Decide how two separately launched terminal processes will communicate, and how to identify the recipient's active session. A parent–child `pipe()` alone does not connect independent sessions.
- [ ] Build a small notification channel and listener; check that a message can reach the correct logged-in terminal.
- [ ] Send a notification only after the ownership change is saved successfully. If the recipient is offline, keep the transfer successful and define the notification behavior.
- [ ] Test with two terminals: transfer to the online recipient, verify an immediate alert, verify no unrelated user receives it, and verify account ownership still passes the required audit.

**Learn:** interprocess communication, sessions, and ordering a notification after persistence.  
**Commits:** `feat: deliver messages between active sessions`; `feat: notify recipient of account transfer`

### Bonus B5 — Store data in SQLite

- [ ] Design `users` and `accounts` tables with unique user names, stable IDs, ownership links, and appropriate constraints. Document the chosen database location.
- [ ] Add database initialization and user/account reads while preserving the current application behavior.
- [ ] Move creation and account changes into database operations, using transactions where a multi-step change must succeed together.
- [ ] Migrate existing text data or provide a documented import path. Check counts, IDs, balances, and ownership after import.
- [ ] Rerun the complete required audit against SQLite, including rejected operations and restart persistence.

**Learn:** schema design, SQL constraints, transactions, and migration.  
**Commits:** `feat: add SQLite schema and reads`; `feat: save ATM changes in SQLite`; `feat: import existing text data`

### Bonus B6 — Other improvements

- [ ] Describe one optional behavior and its expected result before implementing it.
- [ ] Add it in a focused commit, check its success and failure cases, then rerun affected required flows.

**Commit:** name the specific behavior rather than using a generic bonus commit.

## End-of-session note

Record: **Goal · Completed · Checks · Commits · Issues/Decisions · Next step**.
