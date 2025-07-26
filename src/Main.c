#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <fcntl.h>

#define MAX_PATH 4096
#define BUFFER_SIZE 8192

int copy_file(const char *src, const char *dst) {
    int in_fd = open(src, O_RDONLY);
    if (in_fd < 0) {
        perror("open source");
        return -1;
    }

    int out_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd < 0) {
        perror("open dest");
        close(in_fd);
        return -1;
    }

    char buf[BUFFER_SIZE];
    ssize_t bytes;

    while ((bytes = read(in_fd, buf, BUFFER_SIZE)) > 0) {
        if (write(out_fd, buf, bytes) != bytes) {
            perror("write");
            close(in_fd);
            close(out_fd);
            return -1;
        }
    }

    if (bytes < 0) {
        perror("read");
    }

    close(in_fd);
    close(out_fd);

    return (bytes < 0) ? -1 : 0;
}
void run_commands_in_dir(const char *path) {
    printf("\n>> In directory: %s\n", path);

    char name1[MAX_PATH];
    snprintf(name1,sizeof(name1),"%s/LICENSE",path);
    copy_file("./LICENSE",name1);

    char name2[MAX_PATH];
    snprintf(name2,sizeof(name2),"%s/.gitignore",path);
    copy_file("./.gitignore",name2);

    char name3[MAX_PATH];
    snprintf(name3,sizeof(name3),"%s/README.md",path);
    copy_file("./README.md",name3);

    printf("\n>> Files added: %s\n", path);

    // char cmd[MAX_PATH];
    // snprintf(cmd,sizeof(cmd),"cd %s; pwd",path);
    // int ret = system(cmd);

    char cmd[MAX_PATH];
    snprintf(cmd,sizeof(cmd),"cd %s; git init; git add .; git commit -m \"init commit\"; gh repo create %s --public --source=. --remote=origin --push",path,basename(path));
    int ret = system(cmd);

    printf("\n>> Done: %s\n", path);
}
void walk_directory(const char *base_path) {
    struct dirent *entry;
    DIR *dir = opendir(base_path);

    if (!dir) {
        perror("opendir failed");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // "." und ".." ignorieren
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s%s", base_path, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) == -1) {
            perror("stat failed");
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            // Keine Rekursion hier
            run_commands_in_dir(full_path);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <folder>\n", argv[0]);
        return 1;
    }

    walk_directory(argv[1]);

    return 0;
}
