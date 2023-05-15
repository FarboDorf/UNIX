#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEVICE_PATH "/dev/chardev"
#define FILE_PATH "output.txt"
#define BUFFER_SIZE 21

int main()
{
    int fd;
    char buffer[BUFFER_SIZE];

    while (1)
    {
        fd = open(DEVICE_PATH, O_RDWR);
        if (fd < 0)
        {
            perror("Failed to open device");
            return -1;
        }

        int file_fd = open(FILE_PATH, O_WRONLY | O_CREAT | O_TRUNC);
        if (file_fd < 0)
        {
            perror("Failed to open file for write");
            close(fd);
            return -1;
        }

        ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read < 0)
        {
            perror("Failed to read from device");
            close(fd);
            return -1;
        }

        buffer[bytes_read] = '\0';

        write(file_fd, buffer, bytes_read);

        printf("Input: >>> %s <<<\n",buffer);

        close(fd);
        close(file_fd);

        sleep(5);
    }

    return 0;
}
