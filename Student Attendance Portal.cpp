#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cstring>
#include <ctime>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <limits> // For numeric_limits

#ifdef _WIN32
#define CLEAR_COMMAND "cls"
#else
#define CLEAR_COMMAND "clear"
#endif

using namespace std;

const int MAX_STUDENTS = 500;
const int MAX_TEACHERS = 100; // Total teacher entries 

// Global variables
int studentCount = 0;
int teacherCount = 0;

// Student arrays
int studentRolls[MAX_STUDENTS];
char studentNames[MAX_STUDENTS][50];
char studentDepartments[MAX_STUDENTS][30];
char studentSemesters[MAX_STUDENTS][8];
char studentCMS[MAX_STUDENTS][20];
char studentPasswords[MAX_STUDENTS][20];
int presentCounts[MAX_STUDENTS];
int absentCounts[MAX_STUDENTS];

// Teacher arrays 
char teacherNames[MAX_TEACHERS][50];
char teacherSubjects[MAX_TEACHERS][30];
char teacherDepartments[MAX_TEACHERS][30];
char teacherSemesters[MAX_TEACHERS][8];
char teacherCMS[MAX_TEACHERS][20];
char teacherPasswords[MAX_TEACHERS][20];

// Current logged in user info
char currentUserCMS[20] = "";
char currentUserName[50] = "";
char currentUserSubject[30] = ""; // NEW: Stores the subject selected by the teacher on login
int currentUserType = 0;         // 1=Admin, 2=Teacher, 3=Student

// History arrays
vector<string> historyDates[MAX_STUDENTS];
vector<char> historyStatus[MAX_STUDENTS];
vector<string> historySubjects[MAX_STUDENTS]; // To store the subject for each attendance entry

/* UI HELPERS*/
void cls() {
    // 
    system(CLEAR_COMMAND);
}

void line() {
    cout << "---------------------------------------------------------------------\n";
}

void title() {
    cout << "\n=====================================================================\n";
    cout << "                      STUDENT ATTENDANCE PORTAL\n";
    cout << "                      Developed by: Daniyal Saleem\n";
    cout << "=====================================================================\n";
}

void pauseForEnter() {
    cout << "\nPress Enter to continue...";
     // Clear any pending newlines - REMOVED/MOVED FOR CONSISTENCY
    cin.get();
}

    // DATE HELPERS 
string todayDateString() {
    time_t t = time(NULL);
    tm local_tm;
    tm* tmp = localtime(&t);
    if (tmp == NULL) {
        return string("1970-01-01");
    }
    local_tm = *tmp;
    char buf[11];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d",
             local_tm.tm_year + 1900,
             local_tm.tm_mon + 1,
             local_tm.tm_mday);
    return string(buf);
}

    // BASIC HELPERS
bool rollExists(int roll) {
    for (int i = 0; i < studentCount; ++i)
        if (studentRolls[i] == roll) return true;
    return false;
}

int findStudentIndex(int roll) {
    for (int i = 0; i < studentCount; ++i)
        if (studentRolls[i] == roll) return i;
    return -1;
}

// Finds the index of the first teacher entry with the given CMS
int findTeacherIndex(const char* cms) {
    for (int i = 0; i < teacherCount; ++i)
        if (strcmp(teacherCMS[i], cms) == 0) return i;
    return -1;
}

bool studentCMSExists(const char* cms) {
    for (int i = 0; i < studentCount; ++i)
        if (strcmp(studentCMS[i], cms) == 0) return true;
    return false;
}

// Checks if any teacher entry with the given CMS exists
bool teacherCMSExists(const char* cms) {
    for (int i = 0; i < teacherCount; ++i)
        if (strcmp(teacherCMS[i], cms) == 0) return true;
    return false;
}

     // ID GENERATION
void generateTeacherCMS(char* cms, int uniqueId) {
    int attempt = 0;
    do {
        // Use a base ID derived from the total unique teachers and a random component
        snprintf(cms, 20, "T%02d-25-%04d", uniqueId, rand() % 10000);
        attempt++;
    } while (teacherCMSExists(cms) && attempt < 1000);
}

void generateTeacherPassword(char* password) {
    const char* chars = "0123456789";
    for (int i = 0; i < 8; ++i) {
        password[i] = chars[rand() % 10];
    }
    password[8] = '\0';
}

void generateStudentCMS(char* cms) {
    int attempt = 0;
    do {
        snprintf(cms, 20, "S%03d-25-%04d", studentCount + 1, rand() % 10000);
        attempt++;
    } while (studentCMSExists(cms) && attempt < 1000);
}

void generateStudentPassword(char* password) {
    const char* chars = "0123456789";
    for (int i = 0; i < 8; ++i) {
        password[i] = chars[rand() % 10];
    }
    password[8] = '\0';
}

    //SAVE / LOAD 
void saveToFullFile() {
    ofstream fout("attendance_full.txt");
    if (!fout) {
        cout << "Error writing attendance_full.txt\n";
        return;
    }
    
    // Save teachers
    fout << teacherCount << "\n";
    for (int i = 0; i < teacherCount; ++i) {
        fout << teacherNames[i] << "\n"
             << teacherSubjects[i] << "\n"
             << teacherDepartments[i] << "\n"
             << teacherSemesters[i] << "\n"
             << teacherCMS[i] << "\n"
             << teacherPasswords[i] << "\n";
    }
    
    // Save students
    fout << studentCount << "\n";
    for (int i = 0; i < studentCount; ++i) {
        fout << studentRolls[i] << " "
             << studentNames[i] << " "
             << studentDepartments[i] << " "
             << studentSemesters[i] << " "
             << studentCMS[i] << " "
             << studentPasswords[i] << " "
             << presentCounts[i] << " "
             << absentCounts[i] << " "
             << historyDates[i].size() << "\n";
        for (size_t h = 0; h < historyDates[i].size(); ++h) {
            fout << historyDates[i][h] << " " << historyStatus[i][h] << " " << historySubjects[i][h] << "\n";
        }
    }
    fout.close();
}

void loadFromFullFile() {
    ifstream fin("attendance_full.txt");
    if (!fin) {
        cout << "No saved data found.\n";
        return;
    }
    
    // Load teachers
    int tcnt = 0;
    fin >> tcnt;
    fin.ignore(); // Consume the newline after tcnt
    if (tcnt < 0 || tcnt > MAX_TEACHERS) {
        fin.close();
        cout << "Teacher data corrupted. Starting fresh.\n";
        return;
    }
    
    teacherCount = 0;
    for (int i = 0; i < tcnt; ++i) {
        // Using getline for names to fix the missing first letter issue
        fin.getline(teacherNames[i], 50); 
        fin.getline(teacherSubjects[i], 30);
        fin.getline(teacherDepartments[i], 30);
        fin.getline(teacherSemesters[i], 10);
        fin.getline(teacherCMS[i], 20);
        fin.getline(teacherPasswords[i], 20);
        teacherCount++;
    }
    
    // Load students
    int scnt = 0;
    fin >> scnt;
    // fin.ignore(); // Not needed here as next read is number
    if (scnt < 0 || scnt > MAX_STUDENTS) {
        fin.close();
        cout << "Student data corrupted. Starting fresh.\n";
        return;
    }
    
    studentCount = 0;
    for (int i = 0; i < scnt; ++i) {
        int historyCount = 0;
        fin >> studentRolls[i] >> studentNames[i] >> studentDepartments[i] >> studentSemesters[i] 
            >> studentCMS[i] >> studentPasswords[i] >> presentCounts[i] >> absentCounts[i] >> historyCount;
        
        historyDates[i].clear();
        historyStatus[i].clear();
        historySubjects[i].clear(); // NEW: Clear history subjects
        
        for (int h = 0; h < historyCount; ++h) {
            string datestr, subjectstr;
            char st;
            fin >> datestr >> st >> subjectstr; // Load subject
            historyDates[i].push_back(datestr);
            historyStatus[i].push_back(st);
            historySubjects[i].push_back(subjectstr); // Store subject
        }
        studentCount++;
    }
    fin.close();
    cout << teacherCount << " teacher entries and " << studentCount << " students loaded.\n";
}

     //ADMIN FUNCTIONS
bool adminLogin(const string& cms, const string& password) {
    if (cms == "admin.login" && password == "A37-25-0000") {
        cout << "\nAdmin login successful!\n";
        strcpy(currentUserCMS, "admin.login");
        strcpy(currentUserName, "Administrator");
        currentUserType = 1;
        pauseForEnter();
        return true;
    } else {
        cout << "\nInvalid credentials!\n";
        pauseForEnter();
        return false;
    }
}

    // Helper to add  teachers record 
void addTeacherRecord(int& uniqueTeacherId) {
    if (teacherCount >= MAX_TEACHERS) {
        cout << "Maximum teacher limit reached!\n";
        return;
    }
    
    cls(); title();
    cout << "Adding Teacher Record " << (teacherCount + 1) << "\n";
    
    // Ensure buffer is cleared before the first getline
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
    
    // 1. Get Teacher Info (Name, CMS, Password)
    char name[50], cms[20], password[20];
    
    cout << "Enter Teacher Name: ";
    cin.getline(name, 50);
    
    cout << "Enter CMS ID (or leave blank to generate new): ";
    cin.getline(cms, 20);
    
    int existingTeacherIdx = findTeacherIndex(cms);
    if (strlen(cms) > 0 && existingTeacherIdx != -1) {
        // Teacher exists, use existing CMS and Password
        strcpy(teacherNames[teacherCount], name);
        strcpy(teacherCMS[teacherCount], teacherCMS[existingTeacherIdx]);
        strcpy(teacherPasswords[teacherCount], teacherPasswords[existingTeacherIdx]);
        cout << "Existing Teacher found. Using CMS ID: " << teacherCMS[teacherCount] << "\n";
    } else {
        // New Teacher or blank CMS entered, generate new CMS/Password
        strcpy(teacherNames[teacherCount], name);
        if (strlen(cms) == 0) {
            generateTeacherCMS(teacherCMS[teacherCount], uniqueTeacherId++);
        } else {
            // Assume the user wants a specific, non-duplicate CMS (rare for this simple system)
            strcpy(teacherCMS[teacherCount], cms);
        }
        generateTeacherPassword(teacherPasswords[teacherCount]);
        cout << "New CMS ID: " << teacherCMS[teacherCount] << "\n";
        cout << "New Password: " << teacherPasswords[teacherCount] << "\n";
    }
    
    // 2. Get Assignment Info (Subject, Dept, Semester)
    cout << "Enter Subject you currently teaching : ";
    cin.getline(teacherSubjects[teacherCount], 30);
    
    cout << "Enter Department : ";
    cin.getline(teacherDepartments[teacherCount], 30);
    
    cout << "Enter Semester : ";
    cin.getline(teacherSemesters[teacherCount], 10);
    
    cout << "\nTeacher Assignment added successfully!\n";
    teacherCount++;
}

void addMultipleTeachers() {
    int n;
    cout << "How many teacher do you want to add? ";
    cin >> n;

    if (!cin || n <= 0) {
        cout << "Invalid number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    if (teacherCount + n > MAX_TEACHERS) {
        cout << "Cannot add: max capacity reached.\n";
        return;
    }

    // Finding ID for new teachers
    int uniqueTeacherId = 1;
    for (int i = 0; i < teacherCount; ++i) {
        // Simple logic to find max ID based on the first two digits of CMS
        if (teacherCMS[i][0] == 'T') {
            int id = atoi(&teacherCMS[i][1]);
            if (id >= uniqueTeacherId) {
                uniqueTeacherId = id + 1;
            }
        }
    }


    for (int i = 0; i < n; ++i) {
        addTeacherRecord(uniqueTeacherId);
        if (i < n - 1) {
            cout << "\n";
            cout << "Adding next teacher info...\n";
            pauseForEnter();
        }
    }
}

void viewTeachers() {
    cls(); title();
    if (teacherCount == 0) {
        cout << "No teachers informations to display.\n";
        return;
    }

    cout << "REGISTERED TEACHERS \n";
    line();
    cout << left << setw(20) << "Name"
         << setw(15) << "Subject"
         << setw(15) << "Department"
         << setw(10) << "Semester"
         << setw(15) << "CMS ID"
         << setw(12) << "Password\n";
    line();

    for (int i = 0; i < teacherCount; ++i) {
        cout << left << setw(20) << teacherNames[i]
             << setw(15) << teacherSubjects[i]
             << setw(15) << teacherDepartments[i]
             << setw(10) << teacherSemesters[i]
             << setw(15) << teacherCMS[i]
             << setw(12) << teacherPasswords[i] << "\n";
    }
}

void addStudents() {
    int n;
    cout << "How many students do you want to add? ";
    cin >> n;

    if (!cin || n <= 0) {
        cout << "Invalid number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    if (studentCount + n > MAX_STUDENTS) {
        cout << "Cannot add: max capacity reached.\n";
        return;
    }

    for (int i = 0; i < n; ++i) {
        cls(); title();
        cout << "Adding Student " << (studentCount + 1) << "\n";

        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear buffer for getline
        
        cout << "Enter Name: ";
        cin.getline(studentNames[studentCount], 50);
        
        int roll;
        while (true) {
            cout << "Enter Roll Number: ";
            if (!(cin >> roll)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input. Please enter a number.\n";
                continue;
            }
            if (roll <= 0 || rollExists(roll)) {
                cout << "Invalid or duplicate roll. no. Try again.\n";
                // Only ignore if we successfully read a number (which is then invalid/duplicate)
                // If it failed to read a number, the previous continue handled the cin.ignore
                if (roll > 0) cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
                continue;
            }
            break;
        }
        
        studentRolls[studentCount] = roll;
        cout << "Enter Department: ";
        cin >> studentDepartments[studentCount];
        cout << "Enter Semester: ";
        cin >> studentSemesters[studentCount];

        // Generate CMS ID and password
        generateStudentCMS(studentCMS[studentCount]);
        generateStudentPassword(studentPasswords[studentCount]);

        presentCounts[studentCount] = 0;
        absentCounts[studentCount] = 0;
        historyDates[studentCount].clear();
        historyStatus[studentCount].clear();
        historySubjects[studentCount].clear();

        cout << "Student added successfully!\n";
        cout << "CMS ID: " << studentCMS[studentCount] << "\n";
        cout << "Password: " << studentPasswords[studentCount] << "\n";
        
        studentCount++;
        pauseForEnter(); // Pause after adding each student
    }
}

void adminMenu() {
    while (true) {
        cls(); title();
        cout << "ADMIN MENU\n";
        cout << "==========\n";
        cout << "1. Add Teachers \n"; // Option 1 is now Add Multiple
        cout << "2. View Teachers\n";
        cout << "3. Add Students\n";
        cout << "4. View Students Details\n";
        cout << "5. Save Data\n";
        cout << "6. Logout\n";
        cout << "0. Exit Program\n";

        cout << "Enter choice: ";
        int ch; 
        if (!(cin >> ch)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input! Please enter a number.\n";
            pauseForEnter();
            continue;
        }
        // Consume the newline after reading the number
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cls(); title();
        switch (ch) {
            case 1: addMultipleTeachers(); break; // Was 2
            case 2: viewTeachers(); break;      // Was 3
            case 3: addStudents(); break;       // Was 4
            case 4: 
                if (studentCount == 0) {
                    cout << "No students to display.\n";
                } else {
                    cout << "STUDENTS Details\n";
                    line();
                    cout << left << setw(8) << "Roll" << setw(20) << "Name" 
                         << setw(15) << "Department" << setw(10) << "Semester"
                         << setw(15) << "CMS ID" << setw(12) << "Password\n";
                    line();
                    for (int i = 0; i < studentCount; ++i) {
                        cout << left << setw(8) << studentRolls[i] << setw(20) << studentNames[i]
                             << setw(15) << studentDepartments[i] << setw(10) << studentSemesters[i]
                             << setw(15) << studentCMS[i] << setw(12) << studentPasswords[i] << "\n";
                    }
                }
                break; // Was 5
            case 5:
                saveToFullFile();
                cout << "Data Saved Successfully!\n";
                break; // Was 6
            case 6:
                saveToFullFile();
                cout << "Logged out successfully!\n";
                return; // Was 7
            case 0:
                saveToFullFile();
                cout << "Goodbye!\n";
                exit(0);
            default: cout << "Invalid choice!\n";
        }

        pauseForEnter();
    }
}

    //TEACHER FUNCTIONS
bool teacherLogin(const char* cms, const char* password) {
    //  Find all entries for this CMS ID
    vector<int> matchingIndices;
    for (int i = 0; i < teacherCount; ++i) {
        if (strcmp(teacherCMS[i], cms) == 0 && strcmp(teacherPasswords[i], password) == 0) {
            matchingIndices.push_back(i);
        }
    }

    if (matchingIndices.empty()) {
        cout << "\nInvalid credentials!\n";
        return false;
    }

    // Login is successful, 
    strcpy(currentUserCMS, cms);
    strcpy(currentUserName, teacherNames[matchingIndices[0]]);
    currentUserType = 2;
    
    // 2. If multiple assignments, ask the teacher to select one
    if (matchingIndices.size() > 1) {
        cls(); title();
        cout << "Welcome Sir " << currentUserName << "! Please select the subject/assignment for this session:\n";
        line();
        for (size_t i = 0; i < matchingIndices.size(); ++i) {
            int idx = matchingIndices[i];
            cout << i + 1 << ". " << teacherSubjects[idx] 
                 << " (" << teacherDepartments[idx] << " - " << teacherSemesters[idx] << ")\n";
        }
        line();
        
        int choice;
        cout << "Enter choice number: ";
        if (!(cin >> choice) || choice < 1 || choice > (int)matchingIndices.size()) {
            cout << "\nInvalid choice. Defaulting to the first subject.\n";
            choice = 1;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        int selectedIndex = matchingIndices[choice - 1];
        strcpy(currentUserSubject, teacherSubjects[selectedIndex]);
        cout << "\nLogged in for Subject: " << currentUserSubject << "\n";
    } else {
        // Only one assignment, auto-select
        strcpy(currentUserSubject, teacherSubjects[matchingIndices[0]]);
        cout << "\nLogged in for Subject: " << currentUserSubject << "\n";
    }

    pauseForEnter();
    return true;
}

void viewStudentsSummary() {
    cls(); title();
    if (studentCount == 0) {
        cout << "No students to display.\n";
        return;
    }

    // Find current teacher's matching assignment details (Dept/Sem)
    char currentDept[30];
    char currentSem[8];
    bool found = false;
    for (int i = 0; i < teacherCount; ++i) {
        if (strcmp(teacherCMS[i], currentUserCMS) == 0 && strcmp(teacherSubjects[i], currentUserSubject) == 0) {
            strcpy(currentDept, teacherDepartments[i]);
            strcpy(currentSem, teacherSemesters[i]);
            found = true;
            break;
        }
    }
    
    if (!found) {
        cout << "Error: Teacher info. not found in records.\n";
        return;
    }

    cout << "Attendance Summary for " << currentDept << ", " << currentSem << " (Subject: " << currentUserSubject << ")\n";
    line();
    cout << left << setw(8) << "Roll No."
         << setw(18) << "Name"
         << setw(10) << "P (" << currentUserSubject << ")"
         << setw(10) << "A (" << currentUserSubject << ")"
         << setw(10) << "%\n";
    line();

    for (int i = 0; i < studentCount; ++i) {
        // Filter students by current teacher's assignment 
        if (strcmp(studentDepartments[i], currentDept) == 0 && strcmp(studentSemesters[i], currentSem) == 0) {
            // Calculate P/A counts specific to this teacher's subject
            int subjectP = 0;
            int subjectA = 0;
            for(size_t j=0; j < historyDates[i].size(); ++j) {
                if (historySubjects[i][j] == currentUserSubject) {
                    if (historyStatus[i][j] == 'P') subjectP++;
                    else subjectA++;
                }
            }
            
            int total = subjectP + subjectA;
            double perc = total == 0 ? 0.0 : (subjectP * 100.0 / total);
            
            cout << left << setw(8) << studentRolls[i]
                 << setw(18) << studentNames[i]
                 << setw(10) << subjectP
                 << setw(10) << subjectA
                 << setw(9) << fixed << setprecision(2) << perc << "%\n";
        }
    }
}

void viewStudentDetails() {
    cout << "Enter roll no. to view details: ";
    int r;
    if (!(cin >> r)) {
        cout << "Invalid input!\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    
    int idx = findStudentIndex(r);
    if (idx == -1) {
        cout << "Student not found.\n";
        return;
    }
    
    // Calculate P/A counts specific to this teacher's subject
    int subjectP = 0;
    int subjectA = 0;
    for(size_t j=0; j < historyDates[idx].size(); ++j) {
        if (historySubjects[idx][j] == currentUserSubject) {
            if (historyStatus[idx][j] == 'P') subjectP++;
            else subjectA++;
        }
    }
    
    cout << "Student found:\n";
    cout << "Roll No.: " << studentRolls[idx] << "\n";
    cout << "Name: " << studentNames[idx] << "\n";
    cout << "Department: " << studentDepartments[idx] << "\n";
    cout << "Semester: " << studentSemesters[idx] << "\n";
    line();
    cout << "Attendance for Subject: " << currentUserSubject << "\n";
    cout << "Total Present: " << subjectP << "\n";
    cout << "Total Absent:  " << subjectA << "\n";
    int total = subjectP + subjectA;
    double perc = total == 0 ? 0.0 : (subjectP * 100.0 / total);
    cout << "Percentage:    " << fixed << setprecision(2) << perc << "%\n";
    line();
    cout << "History for " << currentUserSubject << ":\n";
    
    int historyCount = 0;
    for (int i = (int)historyDates[idx].size()-1; i >= 0; --i) {
        if (historySubjects[idx][i] == currentUserSubject) {
            cout << historyDates[idx][i] << " : " << (historyStatus[idx][i] == 'P' ? "Present" : "Absent") << "\n";
            historyCount++;
        }
    }
    
    if (historyCount == 0) {
        cout << "No history available for this subject.\n";
    }
}

void markAttendanceFiltered() {
    if (studentCount == 0) {
        cout << "No students available.\n";
        return;
    }
    
    // Find current teacher's matching assignment details
    char currentDept[30];
    char currentSem[8];
    int teacherIdx = -1; 
    for (int i = 0; i < teacherCount; ++i) {
        if (strcmp(teacherCMS[i], currentUserCMS) == 0 && strcmp(teacherSubjects[i], currentUserSubject) == 0) {
            strcpy(currentDept, teacherDepartments[i]);
            strcpy(currentSem, teacherSemesters[i]);
            teacherIdx = i;
            break;
        }
    }
    
    if (teacherIdx == -1) {
        cout << "Error: Teacher not found in system.\n";
        return;
    }
    
    // Use today's date automatically
    string dateInput = todayDateString();
    
    cls(); title();
    cout << "Mark Attendance for " << currentDept 
         << " Department, " << currentSem << " Semester (Subject: " << currentUserSubject << ")\n";
    cout << "Date: " << dateInput << "\n";
    line();
    
    // Check for repeated attendance on the same date/subject for the *first matching student*
    for (int i = 0; i < studentCount; ++i) {
        if (strcmp(studentDepartments[i], currentDept) == 0 &&
            strcmp(studentSemesters[i], currentSem) == 0) {
            
            for(size_t j=0; j < historyDates[i].size(); ++j) {
                if (historyDates[i][j] == dateInput && historySubjects[i][j] == currentUserSubject) {
                    cout << "Attendance for " << currentUserSubject << " on " << dateInput << " is already marked.\n";
                    cout << "Roll " << studentRolls[i] << " was marked: " << historyStatus[i][j] << "\n";
                    pauseForEnter();
                    return; 
                }
            }
        }
    }
    
    // mark attendance
    int matchedStudents = 0;
    int markedCount = 0;
    
    cout << "Enter status (P/A) for each student:\n";
    for (int i = 0; i < studentCount; ++i) {
        if (strcmp(studentDepartments[i], currentDept) == 0 &&
            strcmp(studentSemesters[i], currentSem) == 0) {
            matchedStudents++;
            cout << left << setw(8) << studentRolls[i] << setw(20) << studentNames[i] << " : ";
            char ch; 
            if (!(cin >> ch)) {
                 ch = 'A'; // Default to Absent on invalid input
                 cin.clear();
                 cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            
            ch = toupper(ch);

            // Record attendance
            historyDates[i].push_back(dateInput);
            historyStatus[i].push_back((ch=='P') ? 'P' : 'A');
            historySubjects[i].push_back(currentUserSubject);
            
            // Update global P/A counts 
            if (ch=='P') presentCounts[i]++;
            else absentCounts[i]++;
            
            markedCount++;
            // This is crucial for the loop to continue reading the next input correctly
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
    
    if (matchedStudents == 0) {
        cout << "No students found matching your department and semester.\n";
    } else {
        cout << "\nAttendance recorded for " << markedCount << " students on " << dateInput << " for subject " << currentUserSubject << ".\n";
    }
}

void exportCSV() {
    ofstream fout("attendance_export.csv");
    if (!fout) { cout << "Cannot create CSV.\n"; return; }
    
    // Find current teacher's matching assignment details
    char currentDept[30];
    char currentSem[8];
    bool found = false;
    for (int i = 0; i < teacherCount; ++i) {
        if (strcmp(teacherCMS[i], currentUserCMS) == 0 && strcmp(teacherSubjects[i], currentUserSubject) == 0) {
            strcpy(currentDept, teacherDepartments[i]);
            strcpy(currentSem, teacherSemesters[i]);
            found = true;
            break;
        }
    }
    if (!found) { fout.close(); cout << "Error: Teacher record not found in system.\n"; return; }

    
    fout << "Subject," << currentUserSubject << "\n";
    fout << "Department," << currentDept << "\n";
    fout << "Semester," << currentSem << "\n";
    fout << "Roll,Name,Department,Semester,Present,Absent,Percentage\n";
    
    for (int i = 0; i < studentCount; ++i) {
        // Filter students by current teacher's assignment Dept/Sem
        if (strcmp(studentDepartments[i], currentDept) == 0 && strcmp(studentSemesters[i], currentSem) == 0) {
            // Calculate P/A counts specific to this teacher's subject
            int subjectP = 0;
            int subjectA = 0;
            for(size_t j=0; j < historyDates[i].size(); ++j) {
                if (historySubjects[i][j] == currentUserSubject) {
                    if (historyStatus[i][j] == 'P') subjectP++;
                    else subjectA++;
                }
            }
            
            int total = subjectP + subjectA;
            double perc = total == 0 ? 0.0 : (subjectP * 100.0 / total);
            
            fout << studentRolls[i] << "," << studentNames[i] << "," << studentDepartments[i] << "," << studentSemesters[i] << ","
                 << subjectP << "," << subjectA << "," << fixed << setprecision(2) << perc << "\n";
        }
    }
    fout.close();
    cout << "CSV exported  " << currentUserSubject << ")\n";
}

void teacherMenu() {
    while (true) {
        cls(); title();
        cout << "TEACHER MENU - Welcome Sir " << currentUserName << " (Subject: " << currentUserSubject << ")\n";
        cout << "==========================================\n";
        cout << "1. View Students Summary \n";
        cout << "2. Search Particular Student \n";
        cout << "3. Mark Attendance \n";
        cout << "4. Export CSV \n";
        cout << "5. Save Data \n";
        cout << "6. Logout\n";
        cout << "0. Exit Program\n";

        cout << "Enter choice: ";
        int ch; 
        if (!(cin >> ch)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input!\n";
            pauseForEnter();
            continue;
        }
        // Consume the newline after reading the number
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cls(); title();
        switch (ch) {
            case 1: viewStudentsSummary(); break;
            case 2: viewStudentDetails(); break;
            case 3: markAttendanceFiltered(); break;
            case 4: exportCSV(); break;
            case 5:
                saveToFullFile();
                cout << "Data Saved Successfully!\n";
                break;
            case 6:
                saveToFullFile();
                cout << "Logged out successfully!\n";
                return;
            case 0:
                saveToFullFile();
                cout << "Goodbye!\n";
                exit(0);
            default: cout << "Invalid choice!\n";
        }

        pauseForEnter();
    }
}

         // STUDENT FUNCTIONS
bool studentLogin(const char* cms, const char* password) {
    for (int i = 0; i < studentCount; ++i) {
        if (strcmp(studentCMS[i], cms) == 0 && strcmp(studentPasswords[i], password) == 0) {
            strcpy(currentUserCMS, cms);
            strcpy(currentUserName, studentNames[i]);
            currentUserType = 3;
            
            cout << "\nLogin successful! Welcome " << studentNames[i] << "!\n";
            pauseForEnter();
            return true;
        }
    }
    
    cout << "\nInvalid credentials!\n";
    pauseForEnter();
    return false;
}

void viewStudentPersonalInfo() {
    int idx = -1;
    for (int i = 0; i < studentCount; ++i) {
        if (strcmp(studentCMS[i], currentUserCMS) == 0) {
            idx = i;
            break;
        }
    }
    
    if (idx == -1) {
        cout << "Student not found!\n";
        return;
    }
    
    cls(); title();
    cout << "PERSONAL INFORMATION\n";
    line();
    cout << "Roll No.: " << studentRolls[idx] << "\n";
    cout << "Name: " << studentNames[idx] << "\n";
    cout << "Department: " << studentDepartments[idx] << "\n";
    cout << "Semester: " << studentSemesters[idx] << "\n";
    cout << "CMS ID: " << studentCMS[idx] << "\n";
    
    cout << "Assigned Subjects:\n";
    bool foundSubjects = false;
    for (int i = 0; i < teacherCount; ++i) {
        if (strcmp(teacherDepartments[i], studentDepartments[idx]) == 0 &&
            strcmp(teacherSemesters[i], studentSemesters[idx]) == 0) {
            cout << "* " << teacherSubjects[i] << " (Taught by: " << teacherNames[i] << ")\n";
            foundSubjects = true;
        }
    }
    if (!foundSubjects) {
        cout << "No subjects assigned yet";
    }
    cout << "\n";
    line();
}

void viewStudentAttendanceRecord() {
    int idx = -1;
    for (int i = 0; i < studentCount; ++i) {
        if (strcmp(studentCMS[i], currentUserCMS) == 0) {
            idx = i;
            break;
        }
    }
    
    if (idx == -1) {
        cout << "Student not found!\n";
        return;
    }
    
    cls(); title();
    cout << "ATTENDANCE RECORD - " << studentNames[idx] << "\n";
    line();
    
    // Summary by Subject
    cout << "SUMMARY BY SUBJECT:\n";
    vector<string> uniqueSubjects;
    for (size_t j = 0; j < historySubjects[idx].size(); ++j) {
        if (find(uniqueSubjects.begin(), uniqueSubjects.end(), historySubjects[idx][j]) == uniqueSubjects.end()) {
            uniqueSubjects.push_back(historySubjects[idx][j]);
        }
    }

    int totalP = 0;
    int totalA = 0;

    for(const string& subject : uniqueSubjects) {
        int subjectP = 0;
        int subjectA = 0;
        for(size_t j=0; j < historySubjects[idx].size(); ++j) {
            if (historySubjects[idx][j] == subject) {
                if (historyStatus[idx][j] == 'P') subjectP++;
                else subjectA++;
            }
        }
        totalP += subjectP;
        totalA += subjectA;

        int subTotal = subjectP + subjectA;
        double subPerc = subTotal == 0 ? 0.0 : (subjectP * 100.0 / subTotal);
        
        cout << left << setw(20) << subject << ": " 
             << "P=" << subjectP << ", A=" << subjectA 
             << " (" << fixed << setprecision(2) << subPerc << "%)\n";
    }

    line();
    cout << "OVERALL TOTAL:\n";
    cout << "Total Present: " << totalP << "\n";
    cout << "Total Absent:  " << totalA << "\n";
    int totalOverall = totalP + totalA;
    double percOverall = totalOverall == 0 ? 0.0 : (totalP * 100.0 / totalOverall);
    cout << "Percentage:    " << fixed << setprecision(2) << percOverall << "%\n";
    line();
    
    cout << "COMPLETE HISTORY:\n";
    if (historyDates[idx].empty()) {
        cout << "No history available.\n";
    } else {
        // Display history with subject name
        cout << left << setw(12) << "Date" << setw(10) << "Status" << "Subject\n";
        line();
        for (int j = (int)historyDates[idx].size()-1; j >= 0; --j) {
            cout << left << setw(12) << historyDates[idx][j] 
                 << setw(10) << (historyStatus[idx][j] == 'P' ? "Present" : "Absent")
                 << historySubjects[idx][j] << "\n";
        }
    }
}

void exportStudentCSV() {
    int idx = -1;
    for (int i = 0; i < studentCount; ++i) {
        if (strcmp(studentCMS[i], currentUserCMS) == 0) {
            idx = i;
            break;
        }
    }
    
    if (idx == -1) {
        cout << "Student not found!\n";
        return;
    }
    
    string filename = string(studentNames[idx]) + "attendance.csv";
    ofstream fout(filename.c_str());
    if (!fout) { cout << "Cannot create CSV.\n"; return; }
    
    fout << "Name:," << studentNames[idx] << "\n";
    fout << "Roll No.:," << studentRolls[idx] << "\n";
    fout << "Department:," << studentDepartments[idx] << "\n";
    fout << "Semester:," << studentSemesters[idx] << "\n";
    fout << "CMS ID:," << studentCMS[idx] << "\n\n";
    
    // Write detailed history
    fout << "Date,Subject,Status\n";
    for (size_t j = 0; j < historyDates[idx].size(); ++j) {
        fout << historyDates[idx][j] << "," << historySubjects[idx][j] << "," << (historyStatus[idx][j] == 'P' ? "Present" : "Absent") << "\n";
    }
    
    // Write summary by subject
    vector<string> uniqueSubjects;
    for (size_t j = 0; j < historySubjects[idx].size(); ++j) {
        if (find(uniqueSubjects.begin(), uniqueSubjects.end(), historySubjects[idx][j]) == uniqueSubjects.end()) {
            uniqueSubjects.push_back(historySubjects[idx][j]);
        }
    }

    fout << "\nSummary By Subject\n";
    fout << "Subject,Total Present,Total Absent,Percentage\n";
    
    int totalP = 0;
    int totalA = 0;

    for(const string& subject : uniqueSubjects) {
        int subjectP = 0;
        int subjectA = 0;
        for(size_t j=0; j < historySubjects[idx].size(); ++j) {
            if (historySubjects[idx][j] == subject) {
                if (historyStatus[idx][j] == 'P') subjectP++;
                else subjectA++;
            }
        }
        totalP += subjectP;
        totalA += subjectA;

        int subTotal = subjectP + subjectA;
        double subPerc = subTotal == 0 ? 0.0 : (subjectP * 100.0 / subTotal);
        fout << subject << "," << subjectP << "," << subjectA << "," << fixed << setprecision(2) << subPerc << "%\n";
    }
    
    // Write overall summary
    fout << "\nOverall Summary\n";
    fout << "Total Present," << totalP << "\n";
    fout << "Total Absent," << totalA << "\n";
    int totalOverall = totalP + totalA;
    double percOverall = totalOverall == 0 ? 0.0 : (totalP * 100.0 / totalOverall);
    fout << "Percentage," << fixed << setprecision(2) << percOverall << "%\n";
    
    fout.close();
    cout << "CSV exported as " << filename << "\n";
}

void studentMenu() {
    while (true) {
        cls(); title();
        cout << "STUDENT MENU - Welcome " << currentUserName << "\n";
        cout << "==================================\n";
        cout << "1. Personal Information\n";
        cout << "2. Attendance Record\n";
        cout << "3. Export CSV\n";
        cout << "4. Logout\n";
        cout << "0. Exit Program\n";

        cout << "Enter choice: ";
        int ch; 
        if (!(cin >> ch)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input!\n";
            pauseForEnter();
            continue;
        }
        // Consume the newline after reading the number
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cls(); title();
        switch (ch) {
            case 1: viewStudentPersonalInfo(); break;
            case 2: viewStudentAttendanceRecord(); break;
            case 3: exportStudentCSV(); break;
            case 4:
                cout << "Logged out successfully!\n";
                return;
            case 0:
                cout << "Goodbye!\n";
                exit(0);
            default: cout << "Invalid choice!\n";
        }

        pauseForEnter();
    }
}

  // USER TYPE DETECTION 
int detectUserType(const char* cms, const char* password) {
    // Check if admin
    if (strcmp(cms, "admin.login") == 0 && strcmp(password, "A37-25-0000") == 0) {
        return 1; // Admin
    }
    
    // Check if teacher (by finding any matching entry)
    for (int i = 0; i < teacherCount; ++i) {
        if (strcmp(teacherCMS[i], cms) == 0 && strcmp(teacherPasswords[i], password) == 0) {
            return 2; // Teacher
        }
    }
    
    // Check if student
    for (int i = 0; i < studentCount; ++i) {
        if (strcmp(studentCMS[i], cms) == 0 && strcmp(studentPasswords[i], password) == 0) {
            return 3; // Student
        }
    }
    
    return 0; // Invalid
}

     // MAIN Function---------- */
int main() {
    // Ensureing that terminal is clear 
    cls();
    
    srand(time(0)); // Seed for random number generation
    loadFromFullFile();

    while (true) {
        cls(); title();
        cout << "LOGIN PORTAL\n";
        cout << "============\n";
        
        char cms[20], password[20];
        cout << "Enter CMS ID: ";
        cin >> cms;
        cout << "Enter Password: ";
        cin >> password;
        
        int userType = detectUserType(cms, password);
 
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
		        switch (userType) {
            case 1: // Admin
                adminLogin(cms, password);
                adminMenu();
                break;
            case 2: // Teacher
                if (teacherLogin(cms, password)) {
                    teacherMenu();
                }
                break;
            case 3: // Student
                studentLogin(cms, password);
                studentMenu();
                break;
            case 0: // Invalid
                cout << "\nInvalid CMS ID or password!\n";
                pauseForEnter();
                break;
        }
    }

    return 0;
}
