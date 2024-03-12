#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

struct section
{

    int sect_size;
    int sect_offset;
    char sect_name[11];
    char sect_type;
};
void list_rec(bool rec, bool name, bool perm, char *path, char *permString, char *nameStarts)
{
    struct stat s;
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        char fullpath[1024] = {0};
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
        stat(fullpath, &s);
        if (perm)
        {
            mode_t perm_bits = s.st_mode & 0777;
            mode_t bits = 0;
            for (int i = 0; i < 9; i += 3) // this will check for each group of 3 characters and shift them according to user/group/others
            {
                if (permString[i] == 'r')
                {
                    bits |= 0400 >> i / 3 * 3;
                }
                if (permString[i + 1] == 'w')
                {
                    bits |= 0200 >> i / 3 * 3;
                }
                if (permString[i + 2] == 'x')
                {
                    bits |= 0100 >> i / 3 * 3;
                }
            }
            if (perm_bits == bits)
            {
                // printf("da");
                if (S_ISDIR(s.st_mode))
                {
                    printf("%s\n", fullpath);
                    if (rec)
                        list_rec(rec, name, perm, fullpath, permString, nameStarts);
                }
                else
                    printf("%s\n", fullpath);
            }
        }
        else if (name)
        {
            if (strncmp(entry->d_name, nameStarts, strlen(nameStarts)) == 0)
            { // this checks for the name, and comparing only the first strlen characters to be sure that it starts ( not contains ) the certain string
                if (S_ISDIR(s.st_mode))
                {
                    printf("%s\n", fullpath);
                    if (rec)
                        list_rec(rec, name, perm, fullpath, permString, nameStarts);
                }
                else
                    printf("%s\n", fullpath);
            }
        }
        else if (S_ISDIR(s.st_mode)) // default case ( no filters )
        {
            printf("%s\n", fullpath);
            if (rec) // only if recursive was specified we do recursion
                list_rec(rec, name, perm, fullpath, permString, nameStarts);
        }
        else
            printf("%s\n", fullpath);
        // free(fullpath);
    }
    closedir(dir);
}
bool validateFile(char *path, struct section *mySection)
{
    char magic;
    short header_size;
    unsigned char version;
    char no_sections;
    int validTypes[6] = {21, 56, 37, 70, 80, 40};
    int fd = 0;

    fd = open(path, O_RDONLY, 0777);
    lseek(fd, -1, SEEK_END);
    read(fd, &magic, 1);
    if (magic != '6')
    {
        free(mySection);
        return false;
    }
    lseek(fd, -3, SEEK_END);
    read(fd, &header_size, 2);
    lseek(fd, -header_size, SEEK_END);
    read(fd, &version, 1);
    if (version < 95 || version > 135)
    {
        free(mySection);
        return false;
    }
    read(fd, &no_sections, 1);
    if (no_sections < 2 || no_sections > 19)
    {
        free(mySection);
        return false;
    }
    mySection = malloc(sizeof(struct section) * no_sections);
    bool ok=false;
    for (int i = 0; i < no_sections; i++)
    {
        read(fd, &mySection[i].sect_name, 10);
        mySection->sect_name[10] = 0;
        read(fd, &mySection[i].sect_type, 1);
        for (int j = 0; j < 6; j++)
        {
            if (mySection[i].sect_type == validTypes[j])
            {
                ok = true;
                break;
            }
        }
        if (!ok)
        {
            free(mySection);
            return false;
        }
        ok = false;
        read(fd, &mySection[i].sect_offset, 4);
        read(fd, &mySection[i].sect_size, 4);
        if (mySection[i].sect_size > 1329)
        {
            if (strcmp(path, "test_root/6e51QCUocU.MP9") == 0)
                printf("\nsize=%d\n", mySection[i].sect_size);
            free(mySection);
            return false;
        }
    }

    free(mySection);
    return true;
}
void find_all(char *path)
{
    struct stat s;
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        char fullpath[1024] = {0};
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
        stat(fullpath, &s);

        if (S_ISDIR(s.st_mode)) // default case ( no filters )
        {
            // printf("%s\n", fullpath);
            find_all(fullpath);
        }
        else if(S_ISREG(s.st_mode))
        {
            struct section *mySection = NULL;
            bool valid = false;
            valid = validateFile(fullpath, mySection);
            if (valid == true)
            {

                printf("%s\n", fullpath);
            }
        }
        // free(fullpath);
    }
    closedir(dir);
}
void parse(char *path, struct section *mySection)
{
    char magic;
    short header_size;
    unsigned char version;
    char no_sections;
    int fd = 0;
    int validTypes[6] = {21, 56, 37, 70, 80, 40};
    fd = open(path, O_RDONLY, 0777);
    lseek(fd, -1, SEEK_END);
    read(fd, &magic, 1);
    if (magic != '6')
    {
        printf("ERROR\n");
        printf("wrong magic");
        free(mySection);
        return;
    }
    lseek(fd, -3, SEEK_END);
    read(fd, &header_size, 2);
    lseek(fd, -header_size, SEEK_END);
    read(fd, &version, 1);
    if (version < 95 || version > 135)
    {
        printf("ERROR\n");
        printf("wrong version\n");
        //printf("version=%d\n",version);
        free(mySection);
        return;
    }
    read(fd, &no_sections, 1);
    if (no_sections < 2 || no_sections > 19)
    {
        printf("ERROR\n");
        printf("wrong sect_nr");
        free(mySection);
        return;
    }
    mySection = malloc(sizeof(struct section) * no_sections);
    bool ok = false;
    for (int i = 0; i < no_sections; i++)
    {
        read(fd, &mySection[i].sect_name, 10);
        mySection->sect_name[10] = 0;
        read(fd, &mySection[i].sect_type, 1);
        for (int j = 0; j < 6; j++)
        {
            if (mySection[i].sect_type == validTypes[j])
            {
                ok = true;
                break;
            }
        }
        if (!ok)
        {
            printf("ERROR\n");
            printf("wrong sect_types");
            free(mySection);
            return;
        }
        ok = false;
        read(fd, &mySection[i].sect_offset, 4);
        read(fd, &mySection[i].sect_size, 4);
    }
    printf("SUCCESS\n");
    printf("version=%d\n", version);
    printf("nr_sections=%d\n", no_sections);
    for (int i = 0; i < no_sections; i++)
    {
        printf("section%d: %s %d %d\n", i + 1, mySection[i].sect_name, mySection[i].sect_type, mySection[i].sect_size);
    }
    free(mySection);
}
void extract(char *path, struct section *mySection, int section_number, int line_number)
{
    char magic;
    short header_size;
    unsigned char version;
    char no_sections;
    int fd = 0;
    int validTypes[6] = {21, 56, 37, 70, 80, 40};
    if ((fd = open(path, O_RDONLY, 0777)) < 0)
    {
        return;
    }
    lseek(fd, -1, SEEK_END);
    if (read(fd, &magic, 1) < 0)
    {
        return;
    }
    if (magic != '6')
    {
        printf("ERROR\ninvalid file");
        return;
    }
    lseek(fd, -3, SEEK_END);
    if (read(fd, &header_size, 2) < 0)
    {
        return;
    }
    lseek(fd, -header_size, SEEK_END);
    if (read(fd, &version, 1) < 0)
    {
        return;
    }
    if (version < 95 || version > 135)
    {
        printf("ERROR\ninvalid file");
        return;
    }
    if (read(fd, &no_sections, 1) < 0)
    {
        return;
    }
    if (no_sections < 2 || no_sections > 19)
    {
        printf("ERROR\ninvalid file");
        return;
    }
    mySection = malloc(sizeof(struct section) * no_sections);
    bool ok = false;
    if (no_sections < section_number)
    {
        printf("ERROR\ninvalid section");
        free(mySection);
        return;
    }
    for (int i = 0; i < no_sections; i++)
    {
        read(fd, &mySection[i].sect_name, 10);
        mySection->sect_name[10] = 0;
        read(fd, &mySection[i].sect_type, 1);
        read(fd, &mySection[i].sect_offset, 4);
        read(fd, &mySection[i].sect_size, 4);
    }

    for (int j = 0; j < 6; j++)
    {
        if (mySection[section_number - 1].sect_type == validTypes[j])
        {
            ok = true;
            break;
        }
    }
    if (!ok)
    {
        printf("ERROR\ninvalid section");
        free(mySection);
        return;
    }
    ok = false;

    char *buffer;
    buffer = (char *)malloc(sizeof(char) * (mySection[section_number - 1].sect_size) + 1);

    lseek(fd, mySection[section_number - 1].sect_offset, SEEK_SET);

    read(fd, buffer, mySection[section_number - 1].sect_size);

    char *token;
    char *bufferCopy;
    bufferCopy = (char *)malloc(sizeof(char) * (strlen(buffer) + 1));
    strcpy(bufferCopy, buffer);

    char delim[] = {0x0A, 0};

    token = strtok(buffer, delim);
    int line = 0;
    while (token != NULL)
    {
        line++;
        // Get the next line
        // printf("line=%s\n",token);
        token = strtok(NULL, delim);
    }
    // printf("linenr=%d\n", line);
    token = strtok(bufferCopy, delim);
    char *lineBuffer; // HEX EDITOR
    ok = false;
    int line2 = 0;
    line -= line_number;
    // printf("lineNew=%d\n", line);
    while (token != NULL)
    {
        line2++;
        if (line2 == line + 1)
        {
            lineBuffer = (char *)malloc(sizeof(char) * (strlen(token) + 1));
            strcpy(lineBuffer, token);
            ok = true;
            break;
        }
        // Get the next line
        token = strtok(NULL, delim);
    }

    if (!ok)
    {
        printf("ERROR\ninvalid line");
        free(buffer);
        free(bufferCopy);
        free(mySection);
        return;
    }
    // printf("%s",lineBuffer);
    printf("SUCCESS\n");
    for (int i = strlen(lineBuffer) - 1; i >= 0; i--)
        printf("%c", lineBuffer[i]);
    free(buffer);
    free(bufferCopy);
    free(mySection);
    free(lineBuffer);
}

int main(int argc, char **argv)
{
    bool rec = false, name = false, perm = false;
    if (argc == 2)
    {
        if (strcmp(argv[1], "variant") == 0)
        {
            printf("43749\n");
        }
    }
    else if (argc < 2)
    {
        printf("ERROR");
    }
    else
    {
        if (strcmp(argv[1], "list") == 0)
        {
            char path[1024] = {0};
            char nameStart[1024] = {0};
            char permString[1024] = {0};
            char *nameUsed;
            char *permsUsed;
            char *equalsgn;
            for (int i = 2; i < argc; i++)
            {
                if (strncmp(argv[i], "path=", 5) == 0)
                {
                    // path = (char *)malloc(sizeof(char) * strlen(argv[i]) + 1);
                    if (sscanf(argv[i], "%s", path) != 1) // get path=...
                    {
                        return -1;
                    }
                    // printf("path=%s\n",path);
                    equalsgn = strchr(path, '=');
                    equalsgn++;
                } // get the useful path}
                if (strcmp(argv[i], "recursive") == 0)
                    rec = true;
                if (strncmp(argv[i], "name_starts_with=", 17) == 0)
                {
                    name = true;
                    // nameStart = (char *)malloc(sizeof(char) * strlen(argv[i] + 1));
                    if (sscanf(argv[i], "%s", nameStart) != 1) // get the name_start_with=,,,
                    {
                        return -1;
                    }
                    nameUsed = strchr(nameStart, '=');
                    nameUsed++; // get the useful name
                }
                if (strncmp(argv[i], "permissions=", 12) == 0)
                {
                    perm = true;
                    // permString = (char *)malloc(sizeof(char) * strlen(argv[i] + 1)); // get the permission=...
                    if (sscanf(argv[i], "%s", permString) != 1)
                    {
                        return -1;
                    }
                    permsUsed = strchr(permString, '=');
                    permsUsed++; // get the useful permissions
                }
            }
            printf("SUCCESS\n");
            list_rec(rec, name, perm, equalsgn, permsUsed, nameUsed);
            // free(nameStart);
            // free(permString);
            // free(path);
            rec = false;
            name = false;
            perm = false;
        }
        // magic=6 95<=version<=130  2<=nrSections<=19 type=[21, 56. 37, 70, 80, 40, 70]
        if (strcmp(argv[1], "parse") == 0)
        {
            char *path;
            char *equalsgn;
            path = (char *)malloc(sizeof(char) * strlen(argv[2]) + 1);
            if (sscanf(argv[2], "%s", path) != 1) // get path=...
            {
                free(path);
                return -1;
            }
            equalsgn = strchr(path, '=');
            equalsgn++;
            struct section *mySection = NULL;
            // mySection = malloc(sizeof(struct section));
            parse(equalsgn, mySection);
            free(path);
        }
        if (strcmp(argv[1], "extract") == 0)
        {
            char *path;
            char *equalsgn;
            int section_number = 0;
            int line_number = 0;
            char *section_string;
            char *section_string_number;
            char *line_string;
            char *line_string_number;
            for (int i = 2; i < argc; i++)
            {
                if (strncmp(argv[i], "path=", 5) == 0)
                {
                    path = (char *)malloc(sizeof(char) * strlen(argv[i]) + 1);
                    if (sscanf(argv[i], "%s", path) != 1) // get path=...
                    {
                        free(path);
                        return -1;
                    }
                    equalsgn = strchr(path, '=');
                    equalsgn++;
                }
                if (strncmp(argv[i], "section=", 8) == 0)
                {
                    section_string = (char *)malloc(sizeof(char) * strlen(argv[i]) + 1);
                    if (sscanf(argv[i], "%s", section_string) != 1) // get section_nr=...
                    {
                        free(section_string);
                        return -1;
                    }
                    section_string_number = strchr(section_string, '=');
                    section_string_number++;
                    section_number = atoi(section_string_number);
                }
                if (strncmp(argv[i], "line=", 5) == 0)
                {
                    line_string = (char *)malloc(sizeof(char) * strlen(argv[i]) + 1);
                    if (sscanf(argv[i], "%s", line_string) != 1) // get section_nr=...
                    {
                        free(line_string);
                        return -1;
                    }
                    line_string_number = strchr(line_string, '=');
                    line_string_number++;
                    line_number = atoi(line_string_number);
                }
            }
            struct section *mySection = NULL;
            extract(equalsgn, mySection, section_number, line_number);
            free(section_string);
            free(line_string);
            free(path);
        }
        if (strcmp(argv[1], "findall") == 0)
        {
            char path[1024] = {0};
            char *equalsgn;
            if (strncmp(argv[2], "path=", 5) == 0)
            {
                // path = (char *)malloc(sizeof(char) * strlen(argv[i]) + 1);
                if (sscanf(argv[2], "%s", path) != 1) // get path=...
                {
                    return -1;
                }
                // printf("path=%s\n",path);
                equalsgn = strchr(path, '=');
                equalsgn++;
            } // get the useful path}

            printf("SUCCESS\n");
            find_all(equalsgn);
        }
    }
    return 0;
}
