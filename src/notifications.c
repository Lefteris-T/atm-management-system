#define _POSIX_C_SOURCE 200809L
#include "header.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/file.h>

static pid_t listenerPid = -1;
static char activeFifo[64];
static int notificationLockFd = -1;

static void stopNotificationListener(void)
{
    if (listenerPid == -1)
        return;

    /* Keep the per-user lock until teardown is complete so a new session
       cannot replace the FIFO while this session is still removing it. */
    kill(listenerPid, SIGTERM);

    while (waitpid(listenerPid, NULL, 0) == -1 && errno == EINTR)
    {
    }

    unlink(activeFifo);
    listenerPid = -1;

    if (notificationLockFd != -1)
    {
        close(notificationLockFd);
        notificationLockFd = -1;
    }
}

int notificationPath(int userId, char *path, size_t size)
{
    if (userId < 0 || path == NULL || size == 0)
        return 0;

    int written = snprintf(path, size, "./data/notify-%d.fifo", userId);

    return written >= 0 && (size_t)written < size;
}
int createNotificationFifo(int userId)
{
    char path[64];

    if (!notificationPath(userId, path, sizeof path))
        return 0;

    if (mkfifo(path, 0600) == -1)
    {
        perror("mkfifo");
        return 0;
    }

    return 1;
}
static void listenForNotifications(const char *path)
{
    for (;;)
    {
        int fd = open(path, O_RDONLY);
        if (fd == -1)
            _exit(1);

        char message[256];
        ssize_t bytesRead;

        while ((bytesRead = read(fd, message, sizeof message)) > 0)
        {
            printf("\n[Notification] %.*s\n",
                   (int)bytesRead, message);
            fflush(stdout);
        }

        close(fd);

        if (bytesRead < 0)
            _exit(1);
    }
}

int startNotificationListener(int userId)
{
    char path[64];
    char lockPath[80];

    if (listenerPid != -1 ||
        !notificationPath(userId, path, sizeof path))
        return 0;

    int length = snprintf(lockPath, sizeof lockPath,
                          "./data/notify-%d.lock", userId);
    if (length < 0 || (size_t)length >= sizeof lockPath)
        return 0;

    int lockFd = open(lockPath, O_CREAT | O_RDWR, 0600);
    if (lockFd == -1)
    {
        perror("Opening notification lock");
        return 0;
    }

    if (flock(lockFd, LOCK_EX | LOCK_NB) == -1)
    {
        if (errno == EWOULDBLOCK)
            fprintf(stderr, "This user already has an active listener.\n");
        else
            perror("Locking notifications");

        close(lockFd);
        return 0;
    }

    /* The per-user lock makes it safe to recover a stale FIFO from a prior
       session without interfering with another active session. */
    struct stat info;

    if (lstat(path, &info) == 0)
    {
        if (!S_ISFIFO(info.st_mode) || unlink(path) == -1)
        {
            fprintf(stderr, "Could not remove old notification FIFO.\n");
            close(lockFd);
            return 0;
        }
    }
    else if (errno != ENOENT)
    {
        perror("Checking notification FIFO");
        close(lockFd);
        return 0;
    }

    if (!createNotificationFifo(userId))
    {
        close(lockFd);
        return 0;
    }

    fflush(stdout);
    pid_t child = fork();

    if (child == -1)
    {
        perror("fork");
        unlink(path);
        close(lockFd);
        return 0;
    }

    if (child == 0)
    {
        close(lockFd);
        /* The child owns only the read side; the parent keeps the lock. */
        listenForNotifications(path);
        _exit(1);
    }

    listenerPid = child;
    notificationLockFd = lockFd;
    strcpy(activeFifo, path);

    if (atexit(stopNotificationListener) != 0)
    {
        stopNotificationListener();
        return 0;
    }

    return 1;
}

int sendTransferNotification(int recipientId,
                             const char *senderName,
                             int accountNbr)
{
    char path[64];
    char message[160];

    if (!notificationPath(recipientId, path, sizeof path))
        return 0;

    int length = snprintf(message, sizeof message,
                          "%s transferred account %d to you.",
                          senderName, accountNbr);

    if (length < 0 || (size_t)length >= sizeof message)
        return 0;

    int fd = open(path, O_WRONLY | O_NONBLOCK);
    if (fd == -1)
        return 0;

    /* A short, nonblocking write keeps an offline recipient from delaying
       the ownership transfer. */
    ssize_t written = write(fd, message, (size_t)length);
    close(fd);

    return written == length;
}
