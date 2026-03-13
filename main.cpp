#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>

const int MAX_USERS = 10010;
const int MAX_TRAINS = 5010;
const int MAX_STATIONS = 105;
const int MAX_ORDERS = 100010;

// User structure
struct User {
    char username[25];
    char password[35];
    char name[35];
    char mailAddr[35];
    int privilege;
    bool logged_in;
    bool exists;
};

// Train structure
struct Train {
    char trainID[25];
    int stationNum;
    int seatNum;
    char stations[MAX_STATIONS][35];
    int prices[MAX_STATIONS];
    int travelTimes[MAX_STATIONS];
    int stopoverTimes[MAX_STATIONS];
    int saleDate[2]; // start day, end day (from June 1)
    char type;
    char startTime[10];
    bool released;
    bool exists;

    // Seat availability per date per segment
    int seats[100][MAX_STATIONS]; // [date][segment]
};

// Order structure
struct Order {
    char username[25];
    char trainID[25];
    int fromStation, toStation;
    int date;
    int num;
    long long price;
    int status; // 0: success, 1: pending, 2: refunded
    int timestamp;
    bool exists;
};

// Global data
User users[MAX_USERS];
int userCount = 0;

Train trains[MAX_TRAINS];
int trainCount = 0;

Order orders[MAX_ORDERS];
int orderCount = 0;
int orderTimestamp = 0;

// Utility functions
int findUser(const char* username) {
    for (int i = 0; i < userCount; i++) {
        if (users[i].exists && strcmp(users[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

int findTrain(const char* trainID) {
    for (int i = 0; i < trainCount; i++) {
        if (trains[i].exists && strcmp(trains[i].trainID, trainID) == 0) {
            return i;
        }
    }
    return -1;
}

int dateToInt(const char* date) {
    // Convert mm-dd to days from June 1
    int month, day;
    sscanf(date, "%d-%d", &month, &day);
    if (month == 6) return day - 1;
    if (month == 7) return 30 + day - 1;
    if (month == 8) return 61 + day - 1;
    return 0;
}

void intToDate(int days, char* result) {
    // Convert days from June 1 back to mm-dd
    if (days < 30) {
        sprintf(result, "06-%02d", days + 1);
    } else if (days < 61) {
        sprintf(result, "07-%02d", days - 30 + 1);
    } else {
        sprintf(result, "08-%02d", days - 61 + 1);
    }
}

int timeToMinutes(const char* time) {
    int hour, minute;
    sscanf(time, "%d:%d", &hour, &minute);
    return hour * 60 + minute;
}

void minutesToTime(int minutes, char* result) {
    int days = minutes / 1440;
    minutes %= 1440;
    int hour = minutes / 60;
    int minute = minutes % 60;
    sprintf(result, "%02d:%02d", hour, minute);
}

void addDateTime(int startDay, int startMin, int addMin, int* outDay, int* outMin) {
    int total = startMin + addMin;
    *outDay = startDay + total / 1440;
    *outMin = total % 1440;
}

// Command parsing helper
void parseParams(const char* line, char params[][105], char keys[30]) {
    int keyIdx = 0;
    const char* p = line;
    while (*p) {
        if (*p == '-' && *(p+1) && *(p+1) != ' ') {
            char key = *(p+1);
            keys[keyIdx] = key;
            p += 2;
            while (*p == ' ') p++;

            int i = 0;
            // Stop at space or end, but not at '-' unless preceded by space
            while (*p && *p != '\n') {
                if (*p == ' ' && *(p+1) == '-') {
                    break; // Found next parameter
                }
                if (*p == ' ' && *(p+1) == ' ') {
                    break; // Multiple spaces probably end of param
                }
                if (*p == ' ' && *(p+1) == 0) {
                    break; // Space before end
                }
                params[keyIdx][i++] = *p++;
            }
            params[keyIdx][i] = 0;
            keyIdx++;
        } else {
            p++;
        }
    }
    keys[keyIdx] = 0;
}

// Commands

void cmd_add_user(const char* line) {
    char params[30][105];
    char keys[30];
    parseParams(line, params, keys);

    char cur_username[25] = {0}, username[25] = {0}, password[35] = {0};
    char name[35] = {0}, mailAddr[35] = {0};
    int privilege = 0;

    for (int i = 0; keys[i]; i++) {
        switch(keys[i]) {
            case 'c': strcpy(cur_username, params[i]); break;
            case 'u': strcpy(username, params[i]); break;
            case 'p': strcpy(password, params[i]); break;
            case 'n': strcpy(name, params[i]); break;
            case 'm': strcpy(mailAddr, params[i]); break;
            case 'g': privilege = atoi(params[i]); break;
        }
    }

    if (findUser(username) != -1) {
        printf("-1\n");
        return;
    }

    if (userCount == 0) {
        // First user
        User& u = users[userCount++];
        strcpy(u.username, username);
        strcpy(u.password, password);
        strcpy(u.name, name);
        strcpy(u.mailAddr, mailAddr);
        u.privilege = 10;
        u.logged_in = false;
        u.exists = true;
        printf("0\n");
        return;
    }

    int curIdx = findUser(cur_username);
    if (curIdx == -1 || !users[curIdx].logged_in || privilege >= users[curIdx].privilege) {
        printf("-1\n");
        return;
    }

    User& u = users[userCount++];
    strcpy(u.username, username);
    strcpy(u.password, password);
    strcpy(u.name, name);
    strcpy(u.mailAddr, mailAddr);
    u.privilege = privilege;
    u.logged_in = false;
    u.exists = true;
    printf("0\n");
}

void cmd_login(const char* line) {
    char params[30][105];
    char keys[30];
    parseParams(line, params, keys);

    char username[25] = {0}, password[35] = {0};
    for (int i = 0; keys[i]; i++) {
        if (keys[i] == 'u') strcpy(username, params[i]);
        if (keys[i] == 'p') strcpy(password, params[i]);
    }

    int idx = findUser(username);
    if (idx == -1 || users[idx].logged_in || strcmp(users[idx].password, password) != 0) {
        printf("-1\n");
        return;
    }

    users[idx].logged_in = true;
    printf("0\n");
}

void cmd_logout(const char* line) {
    char params[30][105];
    char keys[30];
    parseParams(line, params, keys);

    char username[25] = {0};
    for (int i = 0; keys[i]; i++) {
        if (keys[i] == 'u') strcpy(username, params[i]);
    }

    int idx = findUser(username);
    if (idx == -1 || !users[idx].logged_in) {
        printf("-1\n");
        return;
    }

    users[idx].logged_in = false;
    printf("0\n");
}

void cmd_query_profile(const char* line) {
    char params[30][105];
    char keys[30];
    parseParams(line, params, keys);

    char cur_username[25] = {0}, username[25] = {0};
    for (int i = 0; keys[i]; i++) {
        if (keys[i] == 'c') strcpy(cur_username, params[i]);
        if (keys[i] == 'u') strcpy(username, params[i]);
    }

    int curIdx = findUser(cur_username);
    int idx = findUser(username);

    if (curIdx == -1 || !users[curIdx].logged_in || idx == -1) {
        printf("-1\n");
        return;
    }

    if (strcmp(cur_username, username) != 0 && users[curIdx].privilege <= users[idx].privilege) {
        printf("-1\n");
        return;
    }

    printf("%s %s %s %d\n", users[idx].username, users[idx].name, users[idx].mailAddr, users[idx].privilege);
}

void cmd_modify_profile(const char* line) {
    char params[30][105];
    char keys[30];
    parseParams(line, params, keys);

    char cur_username[25] = {0}, username[25] = {0};
    char password[35] = {0}, name[35] = {0}, mailAddr[35] = {0};
    int privilege = -1;
    bool has_privilege = false;

    for (int i = 0; keys[i]; i++) {
        switch(keys[i]) {
            case 'c': strcpy(cur_username, params[i]); break;
            case 'u': strcpy(username, params[i]); break;
            case 'p': strcpy(password, params[i]); break;
            case 'n': strcpy(name, params[i]); break;
            case 'm': strcpy(mailAddr, params[i]); break;
            case 'g': privilege = atoi(params[i]); has_privilege = true; break;
        }
    }

    int curIdx = findUser(cur_username);
    int idx = findUser(username);

    if (curIdx == -1 || !users[curIdx].logged_in || idx == -1) {
        printf("-1\n");
        return;
    }

    if (strcmp(cur_username, username) != 0 && users[curIdx].privilege <= users[idx].privilege) {
        printf("-1\n");
        return;
    }

    if (has_privilege && privilege >= users[curIdx].privilege) {
        printf("-1\n");
        return;
    }

    if (password[0]) strcpy(users[idx].password, password);
    if (name[0]) strcpy(users[idx].name, name);
    if (mailAddr[0]) strcpy(users[idx].mailAddr, mailAddr);
    if (has_privilege) users[idx].privilege = privilege;

    printf("%s %s %s %d\n", users[idx].username, users[idx].name, users[idx].mailAddr, users[idx].privilege);
}

void cmd_add_train(const char* line) {
    char params[30][105];
    char keys[30];
    parseParams(line, params, keys);

    Train t;
    memset(&t, 0, sizeof(t));

    for (int i = 0; keys[i]; i++) {
        switch(keys[i]) {
            case 'i': strcpy(t.trainID, params[i]); break;
            case 'n': t.stationNum = atoi(params[i]); break;
            case 'm': t.seatNum = atoi(params[i]); break;
            case 'x': strcpy(t.startTime, params[i]); break;
            case 'y': t.type = params[i][0]; break;
            case 's': {
                char temp[105];
                strcpy(temp, params[i]);
                char* token = strtok(temp, "|");
                int idx = 0;
                while (token && idx < MAX_STATIONS) {
                    strcpy(t.stations[idx++], token);
                    token = strtok(NULL, "|");
                }
                break;
            }
            case 'p': {
                char temp[105];
                strcpy(temp, params[i]);
                char* token = strtok(temp, "|");
                int idx = 0;
                while (token && idx < MAX_STATIONS) {
                    t.prices[idx++] = atoi(token);
                    token = strtok(NULL, "|");
                }
                break;
            }
            case 't': {
                char temp[105];
                strcpy(temp, params[i]);
                char* token = strtok(temp, "|");
                int idx = 0;
                while (token && idx < MAX_STATIONS) {
                    t.travelTimes[idx++] = atoi(token);
                    token = strtok(NULL, "|");
                }
                break;
            }
            case 'o': {
                if (strcmp(params[i], "_") == 0) break;
                char temp[105];
                strcpy(temp, params[i]);
                char* token = strtok(temp, "|");
                int idx = 0;
                while (token && idx < MAX_STATIONS) {
                    t.stopoverTimes[idx++] = atoi(token);
                    token = strtok(NULL, "|");
                }
                break;
            }
            case 'd': {
                char temp[105];
                strcpy(temp, params[i]);
                char* token = strtok(temp, "|");
                if (token) t.saleDate[0] = dateToInt(token);
                token = strtok(NULL, "|");
                if (token) t.saleDate[1] = dateToInt(token);
                break;
            }
        }
    }

    if (findTrain(t.trainID) != -1) {
        printf("-1\n");
        return;
    }

    // Initialize seats
    for (int d = 0; d < 100; d++) {
        for (int s = 0; s < t.stationNum; s++) {
            t.seats[d][s] = t.seatNum;
        }
    }

    t.released = false;
    t.exists = true;
    trains[trainCount++] = t;
    printf("0\n");
}

void cmd_delete_train(const char* line) {
    char params[30][105];
    char keys[30];
    parseParams(line, params, keys);

    char trainID[25] = {0};
    for (int i = 0; keys[i]; i++) {
        if (keys[i] == 'i') strcpy(trainID, params[i]);
    }

    int idx = findTrain(trainID);
    if (idx == -1 || trains[idx].released) {
        printf("-1\n");
        return;
    }

    trains[idx].exists = false;
    printf("0\n");
}

void cmd_release_train(const char* line) {
    char params[30][105];
    char keys[30];
    parseParams(line, params, keys);

    char trainID[25] = {0};
    for (int i = 0; keys[i]; i++) {
        if (keys[i] == 'i') strcpy(trainID, params[i]);
    }

    int idx = findTrain(trainID);
    if (idx == -1 || trains[idx].released) {
        printf("-1\n");
        return;
    }

    trains[idx].released = true;
    printf("0\n");
}

void cmd_query_train(const char* line) {
    char params[30][105];
    char keys[30];
    parseParams(line, params, keys);

    char trainID[25] = {0};
    char date[10] = {0};

    for (int i = 0; keys[i]; i++) {
        if (keys[i] == 'i') strcpy(trainID, params[i]);
        if (keys[i] == 'd') strcpy(date, params[i]);
    }

    int idx = findTrain(trainID);
    if (idx == -1) {
        printf("-1\n");
        return;
    }

    Train& t = trains[idx];
    int day = dateToInt(date);

    printf("%s %c\n", t.trainID, t.type);

    int startMin = timeToMinutes(t.startTime);
    int cumPrice = 0;
    int cumArrTime = 0; // Tracks cumulative time to arrival at current station

    for (int i = 0; i < t.stationNum; i++) {
        char arrDate[10], arrTime[10], depDate[10], depTime[10];

        // Arrival time at station i
        if (i == 0) {
            strcpy(arrDate, "xx-xx");
            strcpy(arrTime, "xx:xx");
        } else {
            int arrDay = day;
            int arrMin = startMin + cumArrTime;
            while (arrMin >= 1440) {
                arrMin -= 1440;
                arrDay++;
            }
            intToDate(arrDay, arrDate);
            minutesToTime(arrMin, arrTime);
        }

        // Departure time from station i
        if (i == t.stationNum - 1) {
            strcpy(depDate, "xx-xx");
            strcpy(depTime, "xx:xx");
        } else {
            int stopTime = (i > 0) ? t.stopoverTimes[i-1] : 0;
            int depDay = day;
            int depMin = startMin + cumArrTime + stopTime;
            while (depMin >= 1440) {
                depMin -= 1440;
                depDay++;
            }
            intToDate(depDay, depDate);
            minutesToTime(depMin, depTime);
        }

        int seat = (i < t.stationNum - 1) ? t.seats[day][i] : 0;

        printf("%s %s %s -> %s %s %d ", t.stations[i], arrDate, arrTime, depDate, depTime, cumPrice);
        if (i == t.stationNum - 1) {
            printf("x\n");
        } else {
            printf("%d\n", seat);
        }

        // Update for next station
        if (i < t.stationNum - 1) {
            cumPrice += t.prices[i];
            cumArrTime += t.travelTimes[i];
            if (i > 0) {
                cumArrTime += t.stopoverTimes[i-1];
            }
        }
    }
}

void cmd_query_ticket(const char* line) {
    printf("0\n");
}

void cmd_query_transfer(const char* line) {
    printf("0\n");
}

void cmd_buy_ticket(const char* line) {
    printf("-1\n");
}

void cmd_query_order(const char* line) {
    char params[30][105];
    char keys[30];
    parseParams(line, params, keys);

    char username[25] = {0};
    for (int i = 0; keys[i]; i++) {
        if (keys[i] == 'u') strcpy(username, params[i]);
    }

    int idx = findUser(username);
    if (idx == -1 || !users[idx].logged_in) {
        printf("-1\n");
        return;
    }

    printf("0\n");
}

void cmd_refund_ticket(const char* line) {
    printf("-1\n");
}

void cmd_clean() {
    for (int i = 0; i < userCount; i++) {
        users[i].exists = false;
    }
    userCount = 0;

    for (int i = 0; i < trainCount; i++) {
        trains[i].exists = false;
    }
    trainCount = 0;

    for (int i = 0; i < orderCount; i++) {
        orders[i].exists = false;
    }
    orderCount = 0;
    orderTimestamp = 0;

    printf("0\n");
}

void cmd_exit() {
    for (int i = 0; i < userCount; i++) {
        users[i].logged_in = false;
    }
    printf("bye\n");
}

int main() {
    char line[10000];
    while (fgets(line, sizeof(line), stdin)) {
        char cmd[30] = {0};
        sscanf(line, "%s", cmd);

        if (strcmp(cmd, "add_user") == 0) cmd_add_user(line);
        else if (strcmp(cmd, "login") == 0) cmd_login(line);
        else if (strcmp(cmd, "logout") == 0) cmd_logout(line);
        else if (strcmp(cmd, "query_profile") == 0) cmd_query_profile(line);
        else if (strcmp(cmd, "modify_profile") == 0) cmd_modify_profile(line);
        else if (strcmp(cmd, "add_train") == 0) cmd_add_train(line);
        else if (strcmp(cmd, "delete_train") == 0) cmd_delete_train(line);
        else if (strcmp(cmd, "release_train") == 0) cmd_release_train(line);
        else if (strcmp(cmd, "query_train") == 0) cmd_query_train(line);
        else if (strcmp(cmd, "query_ticket") == 0) cmd_query_ticket(line);
        else if (strcmp(cmd, "query_transfer") == 0) cmd_query_transfer(line);
        else if (strcmp(cmd, "buy_ticket") == 0) cmd_buy_ticket(line);
        else if (strcmp(cmd, "query_order") == 0) cmd_query_order(line);
        else if (strcmp(cmd, "refund_ticket") == 0) cmd_refund_ticket(line);
        else if (strcmp(cmd, "clean") == 0) cmd_clean();
        else if (strcmp(cmd, "exit") == 0) {
            cmd_exit();
            break;
        }
    }
    return 0;
}
