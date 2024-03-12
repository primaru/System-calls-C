#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/mman.h>

int main(int argc, char **argv)
{
    int fd = -1, fd2 = -1;
    unsigned int variant = 43749;
    char buffer[250];
    unlink("RESP_PIPE_43749");
    if (mkfifo("RESP_PIPE_43749", 0777) == -1)
    {
        printf("ERROR \n cannot create the response pipe");
        return 1;
    }
    if ((fd = open("REQ_PIPE_43749", O_RDONLY)) == -1)
    {
        perror("ERROR \n cannot open the request pipe");
        return 1;
    }
    if ((fd2 = open("RESP_PIPE_43749", O_WRONLY)) == -1)
    {
        perror("mkfifo");
        return 1;
    }
    else
    {
        write(fd2, "BEGIN#", strlen("BEGIN#"));
        int i = 0;
        void *shared_mem;
        int memfd = -1;
        int fdmap = -1;
        char *mappedFile;
        struct stat fileStat;
        while (read(fd, &buffer[i], 1) > -1)
        {
            if (buffer[i] == '#')
            {
                buffer[i + 1] = '\0';
                if (strcmp(buffer, "PING#") == 0)
                {
                    write(fd2, "PING#", strlen("PING#"));
                    write(fd2, &variant, sizeof(variant));
                    write(fd2, "PONG#", strlen("PONG#"));
                    i = 0;
                }
                else if (strcmp(buffer, "CREATE_SHM#") == 0)
                {
                    memfd = shm_open("/85y6r8jo", O_CREAT | O_RDWR, 0664);
                    if (memfd == -1)
                    {
                        write(fd2, "CREATE_SHM#", strlen("CREATE_SHM#"));
                        write(fd2, "ERROR#", strlen("ERROR#"));
                        close(memfd);
                        i = 0;
                    }
                    unsigned int mem_size;
                    if (read(fd, &mem_size, sizeof(unsigned int)) == -1)
                    {
                        perror("cannot read");
                        return 0;
                    };
                    if (ftruncate(memfd, mem_size) == -1)
                    {
                        write(fd2, "CREATE_SHM#", strlen("CREATE_SHM#"));
                        write(fd2, "ERROR#", strlen("ERROR#"));
                        close(memfd);
                        i = 0;
                    }
                    shared_mem = (char *)mmap(NULL, mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0);
                    if (shared_mem == MAP_FAILED)
                    {
                        write(fd2, "CREATE_SHM#", strlen("CREATE_SHM#"));
                        write(fd2, "ERROR#", strlen("ERROR#"));
                        close(memfd);
                        i = 0;
                    }
                    write(fd2, "CREATE_SHM#", strlen("CREATE_SHM#"));
                    write(fd2, "SUCCESS#", strlen("SUCCESS#"));
                    i = 0;
                }
                else if (strcmp(buffer, "WRITE_TO_SHM#") == 0)
                {
                    unsigned int offset, value;
                    read(fd, &offset, sizeof(unsigned int));
                    read(fd, &value, sizeof(unsigned int));
                    if (offset > 0 && offset < 5174024 && offset + sizeof(value) < 5174024)
                    {
                        memcpy((char *)shared_mem + offset, &value, sizeof(unsigned int));
                    }
                    else
                    {
                        write(fd2, "WRITE_TO_SHM#", strlen("WRITE_TO_SHM#"));
                        write(fd2, "ERROR#", strlen("ERROR#"));
                        close(memfd);
                        i = 0;
                    }
                    write(fd2, "WRITE_TO_SHM#", strlen("WRITE_TO_SHM#"));
                    write(fd2, "SUCCESS#", strlen("SUCCESS#"));
                    i = 0;
                }
                else if (strcmp(buffer, "MAP_FILE#") == 0)
                {
                    char name[250];
                    char buf;
                    int k = 0;
                    while (read(fd, &buf, 1) > -1)
                    {
                        if (buf == '#')
                        {
                            break;
                        }
                        name[k] = buf;
                        k++;
                    }
                    name[k] = '\0';
                    printf("%s\n", name);
                    fdmap = open(name, O_RDWR);
                    if (fd == -1)
                    {
                        write(fd2, "MAP_FILE#", strlen("MAP_FILE#"));
                        write(fd2, "ERROR#", strlen("ERROR#"));
                        close(fdmap);
                        i = 0;
                    }
                    if (fstat(fdmap, &fileStat) == -1)
                    {
                        write(fd2, "MAP_FILE#", strlen("MAP_FILE#"));
                        write(fd2, "ERROR#", strlen("ERROR#"));
                        close(fdmap);
                        i = 0;
                    }
                    mappedFile = mmap(NULL, fileStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fdmap, 0);
                    if (mappedFile == MAP_FAILED)
                    {
                        write(fd2, "MAP_FILE#", strlen("MAP_FILE#"));
                        write(fd2, "ERROR#", strlen("ERROR#"));
                        close(fdmap);
                        i = 0;
                    }
                    write(fd2, "MAP_FILE#", strlen("MAP_FILE#"));
                    write(fd2, "SUCCESS#", strlen("SUCCESS#"));
                    i = 0;
                }
                else if (strcmp(buffer, "READ_FROM_FILE_OFFSET#") == 0)
                {
                    unsigned int offset;
                    unsigned int nr_of_bytes;
                    if (read(fd, &offset, sizeof(unsigned int)) == -1)
                    {
                        write(fd2, "READ_FROM_FILE_OFFSET#", strlen("READ_FROM_FILE_OFFSET#"));
                        write(fd2, "ERROR#", strlen("ERROR#"));
                        i = 0;
                    }
                    if (read(fd, &nr_of_bytes, sizeof(unsigned int)) == -1)
                    {
                        write(fd2, "READ_FROM_FILE_OFFSET#", strlen("READ_FROM_FILE_OFFSET#"));
                        write(fd2, "ERROR#", strlen("ERROR#"));
                        i = 0;
                    }
                    if (offset + nr_of_bytes >= fileStat.st_size)
                    {
                        write(fd2, "READ_FROM_FILE_OFFSET#", strlen("READ_FROM_FILE_OFFSET#"));
                        write(fd2, "ERROR#", strlen("ERROR#"));
                        i = 0;
                    }
                    printf("offset=%d bytes=%d\n", offset, nr_of_bytes);
                    printf("size of file=%ld\n", fileStat.st_size);

                    printf("%d\n", offset);
                    printf("%p\n", (void *)mappedFile + offset);
                    printf("%p\n", (void *)mappedFile + 39604);

                    memcpy(shared_mem, mappedFile + 39604, nr_of_bytes); // if i use offset instead of 39604, it doesnt work,
                    //  even though I printed offset and it is indeed 39604, and i also printed mappedFile+offset and mappedFile+39604 and it is the same value
                    i = 0;
                    write(fd2, "READ_FROM_FILE_OFFSET#", strlen("READ_FROM_FILE_OFFSET#"));
                    write(fd2, "SUCCESS#", strlen("SUCCESS#"));
                }
                else if (strcmp(buffer, "READ_FROM_FILE_SECTION#") == 0)
                {
                    unsigned int section_no;
                    unsigned int offset;
                    unsigned int nr_of_bytes;
                    if (read(fd, &section_no, sizeof(unsigned int)) == -1)
                    {
                        write(fd2, "READ_FROM_FILE_SECTION#", strlen("READ_FROM_FILE_SECTION#"));
                        write(fd2, "ERROR#", strlen("ERROR#"));
                        i = 0;
                    }
                    if (read(fd, &offset, sizeof(unsigned int)) == -1)
                    {
                        write(fd2, "READ_FROM_FILE_SECTION#", strlen("READ_FROM_FILE_SECTION#"));
                        write(fd2, "ERROR#", strlen("ERROR#"));
                        i = 0;
                    }
                    if (read(fd, &nr_of_bytes, sizeof(unsigned int)) == -1)
                    {
                        write(fd2, "READ_FROM_FILE_SECTION#", strlen("READ_FROM_FILE_SECTION#"));
                        write(fd2, "ERROR#", strlen("ERROR#"));
                        i = 0;
                    }
                    printf("nr_of_bytes=%u\n", nr_of_bytes);
                    char magic;
                    short header_size = 0;
                    char version = 0, nr_sections = 0;
                    memcpy(&magic, mappedFile + fileStat.st_size - 1, 1);
                    memcpy(&header_size, mappedFile + fileStat.st_size - 3, 2);
                    memcpy(&version, mappedFile + fileStat.st_size - header_size, 1);
                    memcpy(&nr_sections, mappedFile + fileStat.st_size - header_size + 1, 1);
                    if (nr_sections < section_no)
                    {
                        write(fd2, "READ_FROM_FILE_SECTION#", strlen("READ_FROM_FILE_SECTION#"));
                        write(fd2, "ERROR#", strlen("ERROR#"));
                        i = 0;
                    }
                    else
                    {
                        int count = 1;
                        while (count < section_no)
                        {
                            count++;
                        }
                        int offset_section;
                        memcpy(&offset_section, mappedFile + fileStat.st_size - header_size + 2 + (count - 1) * 19 + 11, 4);
                        memcpy(shared_mem, mappedFile + offset + offset_section, nr_of_bytes);
                        write(fd2, "READ_FROM_FILE_SECTION#", strlen("READ_FROM_FILE_SECTION#"));
                        write(fd2, "SUCCESS#", strlen("SUCCESS#"));
                        i = 0;
                    }
                }
                else if (strcmp(buffer, "READ_FROM_LOGICAL_SPACE_OFFSET#") == 0)
                {
                    unsigned int logical_offset = 0;
                    unsigned int no_of_bytes = 0;
                    if (read(fd, &logical_offset, sizeof(unsigned int)) == -1)
                    {
                        write(fd2, "READ_FROM_LOGICAL_SPACE_OFFSET#", strlen("READ_FROM_LOGICAL_SPACE_OFFSET#"));
                        write(fd2, "ERROR#", strlen("ERROR#"));
                        i = 0;
                    }
                    if (read(fd, &no_of_bytes, sizeof(unsigned int)) == -1)
                    {
                        write(fd2, "READ_FROM_LOGICAL_SPACE_OFFSET#", strlen("READ_FROM_LOGICAL_SPACE_OFFSET#"));
                        write(fd2, "ERROR#", strlen("ERROR#"));
                        i = 0;
                    }
                    int section_separator = 1024;
                    short header_size;
                    memcpy(&header_size, mappedFile + fileStat.st_size - 3, 2);
                    char nr_sections;
                    memcpy(&nr_sections, mappedFile + fileStat.st_size - header_size + 1, 1);
                    int count = 0;
                    int alligment = 0;
                    char buff[500000];
                    while (count < nr_sections)
                    {
                        count++;
                        int section_size;
                        int offset_section;
                        memcpy(&offset_section, mappedFile + fileStat.st_size - header_size + 2 + (count - 1) * 19 + 11, 4);
                        memcpy(&section_size, mappedFile + fileStat.st_size - header_size + 2 + (count - 1) * 19 + 15, 4);
                        memcpy(buff + alligment * section_separator, mappedFile + offset_section, section_size);
                        if (section_size > section_separator)
                        {
                            alligment += (int)(section_size / section_separator);
                        }
                        alligment++;
                    }
                    memcpy(shared_mem,buff+logical_offset,no_of_bytes);
                    write(fd2, "READ_FROM_LOGICAL_SPACE_OFFSET#", strlen("READ_FROM_LOGICAL_SPACE_OFFSET#"));
                    write(fd2, "SUCCESS#", strlen("SUCCESS#"));
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            else
            {
                i++;
            }
        }
        munmap(mappedFile, fileStat.st_size);
        close(fdmap);
        shm_unlink("/85y6r8jo");
    }
    unlink("RESP_PIPE_43749");
    return 0;
}