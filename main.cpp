#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>

// Simple hash map implementation since we can't use std::map
template<typename K, typename V, int MAX_SIZE = 100000>
class HashMap {
private:
    struct Node {
        K key;
        V value;
        bool occupied;
        Node() : occupied(false) {}
    };
    Node data[MAX_SIZE];

    int hash(const K& key) const {
        unsigned long h = 0;
        const char* str = reinterpret_cast<const char*>(&key);
        for (size_t i = 0; i < sizeof(K); ++i) {
            h = h * 131 + str[i];
        }
        return h % MAX_SIZE;
    }

public:
    bool find(const K& key, V& value) const {
        int idx = hash(key);
        int start = idx;
        while (data[idx].occupied) {
            if (data[idx].key == key) {
                value = data[idx].value;
                return true;
            }
            idx = (idx + 1) % MAX_SIZE;
            if (idx == start) break;
        }
        return false;
    }

    bool insert(const K& key, const V& value) {
        int idx = hash(key);
        int start = idx;
        while (data[idx].occupied) {
            if (data[idx].key == key) {
                return false; // Already exists
            }
            idx = (idx + 1) % MAX_SIZE;
            if (idx == start) return false; // Full
        }
        data[idx].key = key;
        data[idx].value = value;
        data[idx].occupied = true;
        return true;
    }

    bool update(const K& key, const V& value) {
        int idx = hash(key);
        int start = idx;
        while (data[idx].occupied) {
            if (data[idx].key == key) {
                data[idx].value = value;
                return true;
            }
            idx = (idx + 1) % MAX_SIZE;
            if (idx == start) break;
        }
        return false;
    }

    void clear() {
        for (int i = 0; i < MAX_SIZE; ++i) {
            data[i].occupied = false;
        }
    }
};

// User structure
struct User {
    char username[25];
    char password[35];
    char name[35];
    char mailAddr[35];
    int privilege;
    bool logged_in;

    User() : privilege(0), logged_in(false) {
        username[0] = password[0] = name[0] = mailAddr[0] = 0;
    }
};

// Simple user management
class UserSystem {
private:
    HashMap<std::string, User> users;
    int user_count;

public:
    UserSystem() : user_count(0) {}

    bool add_user(const char* cur_username, const char* username, const char* password,
                  const char* name, const char* mailAddr, int privilege) {
        if (user_count == 0) {
            // First user
            User user;
            strcpy(user.username, username);
            strcpy(user.password, password);
            strcpy(user.name, name);
            strcpy(user.mailAddr, mailAddr);
            user.privilege = 10;
            user.logged_in = false;
            if (users.insert(std::string(username), user)) {
                user_count++;
                return true;
            }
            return false;
        }

        // Not first user - check permissions
        User cur_user;
        if (!users.find(std::string(cur_username), cur_user)) return false;
        if (!cur_user.logged_in) return false;
        if (privilege >= cur_user.privilege) return false;

        User user;
        strcpy(user.username, username);
        strcpy(user.password, password);
        strcpy(user.name, name);
        strcpy(user.mailAddr, mailAddr);
        user.privilege = privilege;
        user.logged_in = false;

        if (users.insert(std::string(username), user)) {
            user_count++;
            return true;
        }
        return false;
    }

    bool login(const char* username, const char* password) {
        User user;
        if (!users.find(std::string(username), user)) return false;
        if (user.logged_in) return false;
        if (strcmp(user.password, password) != 0) return false;

        user.logged_in = true;
        users.update(std::string(username), user);
        return true;
    }

    bool logout(const char* username) {
        User user;
        if (!users.find(std::string(username), user)) return false;
        if (!user.logged_in) return false;

        user.logged_in = false;
        users.update(std::string(username), user);
        return true;
    }

    bool query_profile(const char* cur_username, const char* username, User& result) {
        User cur_user, target_user;
        if (!users.find(std::string(cur_username), cur_user)) return false;
        if (!cur_user.logged_in) return false;
        if (!users.find(std::string(username), target_user)) return false;

        if (strcmp(cur_username, username) != 0 &&
            cur_user.privilege <= target_user.privilege) {
            return false;
        }

        result = target_user;
        return true;
    }

    bool modify_profile(const char* cur_username, const char* username,
                       const char* password, const char* name,
                       const char* mailAddr, int privilege, bool has_privilege,
                       User& result) {
        User cur_user, target_user;
        if (!users.find(std::string(cur_username), cur_user)) return false;
        if (!cur_user.logged_in) return false;
        if (!users.find(std::string(username), target_user)) return false;

        if (strcmp(cur_username, username) != 0 &&
            cur_user.privilege <= target_user.privilege) {
            return false;
        }

        if (has_privilege && privilege >= cur_user.privilege) {
            return false;
        }

        if (password[0]) strcpy(target_user.password, password);
        if (name[0]) strcpy(target_user.name, name);
        if (mailAddr[0]) strcpy(target_user.mailAddr, mailAddr);
        if (has_privilege) target_user.privilege = privilege;

        users.update(std::string(username), target_user);
        result = target_user;
        return true;
    }

    void clear() {
        users.clear();
        user_count = 0;
    }
};

UserSystem user_system;

void parse_command(const char* line) {
    char cmd[30] = {0};
    sscanf(line, "%s", cmd);

    if (strcmp(cmd, "add_user") == 0) {
        char cur_username[25] = {0}, username[25] = {0}, password[35] = {0};
        char name[35] = {0}, mailAddr[35] = {0};
        int privilege = 0;

        const char* p = line;
        while (*p) {
            if (*p == '-' && *(p+1)) {
                char key = *(p+1);
                p += 2;
                while (*p == ' ') p++;

                char value[100] = {0};
                int i = 0;
                while (*p && *p != ' ' && *p != '-') {
                    value[i++] = *p++;
                }
                value[i] = 0;

                switch(key) {
                    case 'c': strcpy(cur_username, value); break;
                    case 'u': strcpy(username, value); break;
                    case 'p': strcpy(password, value); break;
                    case 'n': strcpy(name, value); break;
                    case 'm': strcpy(mailAddr, value); break;
                    case 'g': privilege = atoi(value); break;
                }
            } else {
                p++;
            }
        }

        if (user_system.add_user(cur_username, username, password, name, mailAddr, privilege)) {
            printf("0\n");
        } else {
            printf("-1\n");
        }
    }
    else if (strcmp(cmd, "login") == 0) {
        char username[25] = {0}, password[35] = {0};

        const char* p = line;
        while (*p) {
            if (*p == '-' && *(p+1)) {
                char key = *(p+1);
                p += 2;
                while (*p == ' ') p++;

                char value[100] = {0};
                int i = 0;
                while (*p && *p != ' ' && *p != '-') {
                    value[i++] = *p++;
                }
                value[i] = 0;

                switch(key) {
                    case 'u': strcpy(username, value); break;
                    case 'p': strcpy(password, value); break;
                }
            } else {
                p++;
            }
        }

        if (user_system.login(username, password)) {
            printf("0\n");
        } else {
            printf("-1\n");
        }
    }
    else if (strcmp(cmd, "logout") == 0) {
        char username[25] = {0};

        const char* p = line;
        while (*p) {
            if (*p == '-' && *(p+1) == 'u') {
                p += 2;
                while (*p == ' ') p++;

                int i = 0;
                while (*p && *p != ' ' && *p != '-') {
                    username[i++] = *p++;
                }
                username[i] = 0;
                break;
            } else {
                p++;
            }
        }

        if (user_system.logout(username)) {
            printf("0\n");
        } else {
            printf("-1\n");
        }
    }
    else if (strcmp(cmd, "query_profile") == 0) {
        char cur_username[25] = {0}, username[25] = {0};

        const char* p = line;
        while (*p) {
            if (*p == '-' && *(p+1)) {
                char key = *(p+1);
                p += 2;
                while (*p == ' ') p++;

                char value[100] = {0};
                int i = 0;
                while (*p && *p != ' ' && *p != '-') {
                    value[i++] = *p++;
                }
                value[i] = 0;

                switch(key) {
                    case 'c': strcpy(cur_username, value); break;
                    case 'u': strcpy(username, value); break;
                }
            } else {
                p++;
            }
        }

        User result;
        if (user_system.query_profile(cur_username, username, result)) {
            printf("%s %s %s %d\n", result.username, result.name, result.mailAddr, result.privilege);
        } else {
            printf("-1\n");
        }
    }
    else if (strcmp(cmd, "modify_profile") == 0) {
        char cur_username[25] = {0}, username[25] = {0};
        char password[35] = {0}, name[35] = {0}, mailAddr[35] = {0};
        int privilege = -1;
        bool has_privilege = false;

        const char* p = line;
        while (*p) {
            if (*p == '-' && *(p+1)) {
                char key = *(p+1);
                p += 2;
                while (*p == ' ') p++;

                char value[100] = {0};
                int i = 0;
                while (*p && *p != ' ' && *p != '-') {
                    value[i++] = *p++;
                }
                value[i] = 0;

                switch(key) {
                    case 'c': strcpy(cur_username, value); break;
                    case 'u': strcpy(username, value); break;
                    case 'p': strcpy(password, value); break;
                    case 'n': strcpy(name, value); break;
                    case 'm': strcpy(mailAddr, value); break;
                    case 'g': privilege = atoi(value); has_privilege = true; break;
                }
            } else {
                p++;
            }
        }

        User result;
        if (user_system.modify_profile(cur_username, username, password, name, mailAddr, privilege, has_privilege, result)) {
            printf("%s %s %s %d\n", result.username, result.name, result.mailAddr, result.privilege);
        } else {
            printf("-1\n");
        }
    }
    else if (strcmp(cmd, "clean") == 0) {
        user_system.clear();
        printf("0\n");
    }
    else if (strcmp(cmd, "exit") == 0) {
        printf("bye\n");
    }
    else {
        // Commands not yet implemented - return -1 or default
        printf("-1\n");
    }
}

int main() {
    char line[10000];
    while (fgets(line, sizeof(line), stdin)) {
        // Remove newline
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = 0;
        }

        parse_command(line);

        // Check if exit command
        char cmd[30] = {0};
        sscanf(line, "%s", cmd);
        if (strcmp(cmd, "exit") == 0) {
            break;
        }
    }
    return 0;
}
