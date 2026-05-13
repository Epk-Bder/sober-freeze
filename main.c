#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

// write string into a file
static int write_text(const char *p, const char *t) {
    FILE *f = fopen(p, "w");
    if (!f) return 0;
    fprintf(f, "%s", t);
    fclose(f);
    return 1;
}

// find PID of process named "Main" (sober thing)
static pid_t find_sober() {
    DIR *d = opendir("/proc");
    if (!d) return -1;

    struct dirent *e;
    char path[128], name[256];

    while ((e = readdir(d))) {
        // skip non-PID entries
        if (e->d_name[0] < '0' || e->d_name[0] > '9') continue;

        // read process name
        snprintf(path, sizeof(path), "/proc/%s/comm", e->d_name);

        FILE *f = fopen(path, "r");
        if (!f) continue;

        if (fgets(name, sizeof(name), f)) {
            // remove newline
            name[strcspn(name, "\n")] = 0;

            // match process name
            if (strcmp(name, "Main") == 0) {
                fclose(f);
                closedir(d);
                return atoi(e->d_name);
            }
        }

        fclose(f);
    }

    closedir(d);
    return -1;
}

// get full cgroup v2 path of process
static int get_cgroup(pid_t pid, char *out, size_t sz) {
    char p[64];
    snprintf(p, sizeof(p), "/proc/%d/cgroup", pid);

    FILE *f = fopen(p, "r");
    if (!f) return 0;

    char line[1024];

    while (fgets(line, sizeof(line), f)) {
        // cgroup v2 format line
        if (strncmp(line, "0::", 3) == 0) {
            line[strcspn(line, "\n")] = 0;

            snprintf(out, sz, "/sys/fs/cgroup%s", line + 3);
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("freeze/thaw\n");
        return 1;
    }

    // decide mode
    int freeze = strcmp(argv[1], "freeze") == 0;

    // find sober process
    pid_t pid = find_sober();
    if (pid < 0) return 1;

    // get its cgroup path
    char cg[1024];
    if (!get_cgroup(pid, cg, sizeof(cg))) return 1;

    // store original cgroup so we can restore it later
    char state_file[1200];
    snprintf(state_file, sizeof(state_file), "/tmp/sober_cg_state");

    char child[1200];

    if (freeze) {
        // save original cgroup
        write_text(state_file, cg);

        // create child freeze cgroup
        snprintf(child, sizeof(child), "%s/hs_freeze", cg);

        mkdir(child, 0755);

        char procs[1400], frz[1400];

        // file that accepts PID moves
        snprintf(procs, sizeof(procs), "%s/cgroup.procs", child);

        // file that freezes cgroup
        snprintf(frz, sizeof(frz), "%s/cgroup.freeze", child);

        // write PID into child cgroup
        char pidtxt[32];
        snprintf(pidtxt, sizeof(pidtxt), "%d", pid);

        write_text(procs, pidtxt);

        // freeze the cgroup
        write_text(frz, "1");

        printf("sober frozen\n");
    } else {
        // load saved parent cgroup
        FILE *f = fopen(state_file, "r");
        if (!f) {
            printf("no state saved\n");
            return 1;
        }

        fgets(child, sizeof(child), f);
        fclose(f);

        // remove newline
        child[strcspn(child, "\n")] = 0;

        // rebuild freeze path
        char hs[1400];
        snprintf(hs, sizeof(hs), "%s/hs_freeze", child);

        char frz[1400];
        snprintf(frz, sizeof(frz), "%s/cgroup.freeze", hs);

        // unfreeze cgroup
        write_text(frz, "0");

        // move process back to parent cgroup
        char parent_procs[1400];
        snprintf(parent_procs, sizeof(parent_procs), "%s/cgroup.procs", child);

        char pidtxt[32];
        snprintf(pidtxt, sizeof(pidtxt), "%d", pid);

        write_text(parent_procs, pidtxt);

        // remove temp cgroup folder
        rmdir(hs);

        printf("sober thawed\n");
    }

    return 0;
}