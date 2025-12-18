#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static float absf(float x) { return (x < 0.0f) ? -x : x; }

#define MAX_INPUT_STUDENTS 5
#define MAX_STUDENTS 100

typedef struct {
    int id;
    char name[64];
    int age;
    float gpa;
} Student;

static void trim_newline(char *s) {
    size_t n = strlen(s);
    if (n > 0 && s[n - 1] == '\n') {
        s[n - 1] = '\0';
    }
}

static void read_text(const char *prompt, char *out, size_t out_size) {
    for (;;) {
        printf("%s", prompt);
        fflush(stdout);
        if (!fgets(out, (int)out_size, stdin)) {
            out[0] = '\0';
            return;
        }
        trim_newline(out);
        if (out[0] != '\0') return;
        puts("Khong duoc de trong. Vui long nhap lai.");
    }
}

/* N?p nhanh 10 sinh viên gi? l?p d? ki?m th? các ch?c nang (in/sort/tìm/thêm/xóa/luu/d?c). */
static void loadFakeStudents10(Student *students, int *n) {
    Student tmp[10] = {
        {9001, "Fake SV 01", 18, 2.50f},
        {9002, "Fake SV 02", 19, 3.00f},
        {9003, "Fake SV 03", 20, 3.25f},
        {9004, "Fake SV 04", 21, 3.75f},
        {9005, "Fake SV 05", 18, 4.00f},
        {9006, "Fake SV 06", 19, 1.80f},
        {9007, "Fake SV 07", 20, 2.95f},
        {9008, "Fake SV 08", 21, 3.10f},
        {9009, "Fake SV 09", 22, 3.60f},
        {9010, "Fake SV 10", 18, 2.20f}
    };
    memcpy(students, tmp, sizeof(tmp));
    *n = 10;
}

static int read_int_range(const char *prompt, int min_value, int max_value) {
    char line[128];
    for (;;) {
        printf("%s", prompt);
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) return min_value;

        char *end = NULL;
        long v = strtol(line, &end, 10);
        while (end && (*end == ' ' || *end == '\t')) end++;

        if (end == line || (end && *end != '\0' && *end != '\n')) {
            puts("Gia tri khong hop le. Vui long nhap so nguyen.");
            continue;
        }
        if (v < min_value || v > max_value) {
            printf("Gia tri phai nam trong [%d..%d].\n", min_value, max_value);
            continue;
        }
        return (int)v;
    }
}

static float read_float_range(const char *prompt, float min_value, float max_value) {
    char line[128];
    for (;;) {
        printf("%s", prompt);
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) return min_value;

        char *end = NULL;
        float v = strtof(line, &end);
        while (end && (*end == ' ' || *end == '\t')) end++;

        if (end == line || (end && *end != '\0' && *end != '\n')) {
            puts("Gia tri khong hop le. Vui long nhap so thuc (vi du: 3.75).");
            continue;
        }
        if (v < min_value || v > max_value) {
            printf("Gia tri phai nam trong [%.2f..%.2f].\n", min_value, max_value);
            continue;
        }
        return v;
    }
}

static void print_table(const Student *students, int n) {
    puts("+------+------------------------------+-----+------+\n"
         "| ID   | Name                         | Age | GPA  |\n"
         "+------+------------------------------+-----+------+");

    for (int i = 0; i < n; i++) {
        printf("| %-4d | %-28s | %-3d | %-4.2f |\n",
               students[i].id,
               students[i].name,
               students[i].age,
               students[i].gpa);
    }

    puts("+------+------------------------------+-----+------+");
}

static int saveToFile(const char *filename, const Student *students, int n) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Khong mo duoc file");
        return 0;
    }

    /* Tab-separated for easy viewing and to keep names with spaces intact */
    fprintf(fp, "id\tname\tage\tgpa\n");
    for (int i = 0; i < n; i++) {
        fprintf(fp, "%d\t%s\t%d\t%.2f\n",
                students[i].id,
                students[i].name,
                students[i].age,
                students[i].gpa);
    }

    if (fclose(fp) != 0) {
        perror("Khong dong duoc file");
        return 0;
    }

    return 1;
}

static int readFromFile(const char *filename, Student *students, int max_students) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Khong mo duoc file de doc");
        return 0;
    }

    char line[256];
    int count = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (line[0] == '\0') continue;

        /* Skip header line like: id\tname\tage\tgpa */
        if (count == 0 && strncmp(line, "id\t", 3) == 0) {
            continue;
        }

        if (count >= max_students) break;

        Student s;
        char name[64];
        float gpa = 0.0f;

        /* Format written by saveToFile: id \t name \t age \t gpa */
        int matched = sscanf(line, "%d\t%63[^\t]\t%d\t%f", &s.id, name, &s.age, &gpa);
        if (matched == 4) {
            strncpy(s.name, name, sizeof(s.name));
            s.name[sizeof(s.name) - 1] = '\0';
            s.gpa = gpa;
            students[count++] = s;
        }
    }

    fclose(fp);
    return count;
}

/**
 * Calculate average GPA of a student list.
 *
 * Rules:
 * - If the list is empty (`n <= 0`), returns 0.0f.
 * - `students` may be NULL only when `n <= 0` (safe for empty-list case).
 *
 * @param students Pointer to an array of Student.
 * @param n Number of students in the array.
 * @return float Average GPA, or 0.0f when the list is empty.
 */
static float calcAverageGpa(const Student *students, int n) {
    if (n <= 0) return 0.0f;

    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += students[i].gpa;
    }
    return sum / (float)n;
}

/**
 * Sort students by GPA using Bubble Sort (stable).
 *
 * Bubble Sort repeatedly compares adjacent elements and swaps them if they are
 * in the wrong order. This implementation is stable: students with equal GPA
 * keep their original relative order (we do not swap when GPA is equal).
 *
 * @param students Pointer to array of Student (will be modified in-place).
 * @param n Number of students in the array.
 * @param ascending 1 = sort GPA low->high, 0 = sort GPA high->low.
 */
static void sortStudentsByGpa(Student *students, int n, int ascending) {
    if (n <= 1) return;

    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - i; j++) {
            int should_swap = ascending
                                  ? (students[j].gpa > students[j + 1].gpa)
                                  : (students[j].gpa < students[j + 1].gpa);
            if (should_swap) {
                Student tmp = students[j];
                students[j] = students[j + 1];
                students[j + 1] = tmp;
                swapped = 1;
            }
        }
        if (!swapped) break; /* already sorted */
    }
}

/**
 * Find a student by their ID.
 *
 * Rules:
 * - If `id` is negative, the input is considered invalid and the function returns -2.
 * - If a matching student is found, returns the index (0-based) in the array.
 * - If no match is found, returns -1.
 *
 * @param students Pointer to an array of Student.
 * @param n Number of students in the array.
 * @param id Student ID to search for.
 * @return int Index if found; -1 if not found; -2 if id is negative/invalid.
 */
static int findStudentById(const Student *students, int n, int id) {
    if (id < 0) return -2;
    for (int i = 0; i < n; i++) {
        if (students[i].id == id) return i;
    }
    return -1;
}

/**
 * Add a new student to the list (unique by ID).
 *
 * Rules:
 * - If `new_student.id` is negative => invalid input, return -2.
 * - If the list is full (`*n >= max_students`) => return 0.
 * - If `new_student.id` already exists in the list => return -1.
 * - Otherwise, append the student to the end of the array, increment `*n` and return 1.
 *
 * @param students Pointer to array of Student (will be modified).
 * @param n In/out: current number of students; will be incremented on success.
 * @param max_students Capacity of the array.
 * @param new_student Student record to add.
 * @return int 1=success, 0=full, -1=duplicate ID, -2=invalid (negative ID).
 */
static int addStudent(Student *students, int *n, int max_students, Student new_student) {
    if (new_student.id < 0) return -2;
    if (*n >= max_students) return 0;

    if (findStudentById(students, *n, new_student.id) >= 0) {
        return -1;
    }

    students[*n] = new_student;
    (*n)++;
    return 1;
}

/**
 * Delete a student by ID (keeps array contiguous).
 *
 * Rules:
 * - If `id` is negative => invalid input, return -2.
 * - If `id` is not found => return 0.
 * - If found => remove it by shifting elements left, decrement `*n`, return 1.
 *
 * @param students Pointer to array of Student (will be modified).
 * @param n In/out: current number of students; will be decremented on success.
 * @param id Student ID to delete.
 * @return int 1=deleted, 0=not found, -2=invalid (negative ID).
 */
static int deleteStudentById(Student *students, int *n, int id) {
    int idx = findStudentById(students, *n, id);
    if (idx == -2) return -2;
    if (idx < 0) return 0;

    for (int i = idx; i < *n - 1; i++) {
        students[i] = students[i + 1];
    }
    (*n)--;
    return 1;
}

#ifdef RUN_TESTS
static void run_findStudentById_tests(void) {
    Student s[3] = {
        {1001, "A", 19, 3.50f},
        {1002, "B", 20, 3.85f},
        {1003, "C", 18, 3.20f}
    };

    /* Test case 1: ID t?n t?i */
    assert(findStudentById(s, 3, 1002) == 1);

    /* Test case 2: ID không t?n t?i */
    assert(findStudentById(s, 3, 9999) == -1);

    /* Test case 3: ID âm */
    assert(findStudentById(s, 3, -5) == -2);

    puts("All findStudentById() test cases PASSED.");
}

static void run_add_delete_tests(void) {
    Student s[5] = {
        {1, "A", 18, 3.00f},
        {2, "B", 19, 3.50f}
    };
    int n = 2;

    /* Test case: thêm trùng ID */
    Student dup = {2, "B2", 20, 3.90f};
    assert(addStudent(s, &n, 5, dup) == -1);
    assert(n == 2);

    /* (extra) Test: thêm m?i OK */
    Student ok = {3, "C", 20, 2.80f};
    assert(addStudent(s, &n, 5, ok) == 1);
    assert(n == 3);
    assert(s[2].id == 3);

    /* Test case: xóa không t?n t?i */
    assert(deleteStudentById(s, &n, 9999) == 0);
    assert(n == 3);

    /* (extra) Test: xóa t?n t?i OK */
    assert(deleteStudentById(s, &n, 2) == 1);
    assert(n == 2);
    assert(findStudentById(s, n, 2) == -1);

    puts("All addStudent()/deleteStudentById() test cases PASSED.");
}

static void run_calcAverageGpa_tests(void) {
    Student s10[10] = {
        {2001, "SV 01", 18, 2.50f},
        {2002, "SV 02", 19, 3.00f},
        {2003, "SV 03", 20, 3.25f},
        {2004, "SV 04", 21, 3.75f},
        {2005, "SV 05", 18, 4.00f},
        {2006, "SV 06", 19, 1.80f},
        {2007, "SV 07", 20, 2.95f},
        {2008, "SV 08", 21, 3.10f},
        {2009, "SV 09", 22, 3.60f},
        {2010, "SV 10", 18, 2.20f}
    };

    /* Test: average of 10 fake students */
    float avg = calcAverageGpa(s10, 10);
    assert(absf(avg - 3.015f) < 0.0001f);

    /* Test case: danh sách r?ng */
    assert(calcAverageGpa(s10, 0) == 0.0f);
    assert(calcAverageGpa(NULL, 0) == 0.0f);

    puts("All calcAverageGpa() test cases PASSED.");
}

static void run_sortStudentsByGpa_tests(void) {
    /* Test case 1: GPA b?ng nhau (stable - gi? nguyên th? t?) */
    Student same[3] = {
        {1, "S1", 18, 3.50f},
        {2, "S2", 19, 3.50f},
        {3, "S3", 20, 3.50f}
    };
    sortStudentsByGpa(same, 3, 1);
    assert(same[0].id == 1 && same[1].id == 2 && same[2].id == 3);
    sortStudentsByGpa(same, 3, 0);
    assert(same[0].id == 1 && same[1].id == 2 && same[2].id == 3);

    /* Test case 2: GPA tang d?n s?n */
    Student asc[5] = {
        {10, "A", 18, 1.00f},
        {11, "B", 18, 2.00f},
        {12, "C", 18, 3.00f},
        {13, "D", 18, 3.50f},
        {14, "E", 18, 4.00f}
    };
    sortStudentsByGpa(asc, 5, 1);
    for (int i = 0; i < 4; i++) assert(asc[i].gpa <= asc[i + 1].gpa);

    /* Test case 3: GPA ng?u nhiên */
    Student rnd[6] = {
        {21, "R1", 18, 2.80f},
        {22, "R2", 18, 3.90f},
        {23, "R3", 18, 1.20f},
        {24, "R4", 18, 3.10f},
        {25, "R5", 18, 3.10f}, /* duplicate to check stability for equals */
        {26, "R6", 18, 2.00f}
    };
    sortStudentsByGpa(rnd, 6, 1);
    for (int i = 0; i < 5; i++) assert(rnd[i].gpa <= rnd[i + 1].gpa);
    /* For equal GPA=3.10, original order 24 then 25 should be preserved */
    int pos24 = -1, pos25 = -1;
    for (int i = 0; i < 6; i++) {
        if (rnd[i].id == 24) pos24 = i;
        if (rnd[i].id == 25) pos25 = i;
    }
    assert(pos24 != -1 && pos25 != -1 && pos24 < pos25);

    sortStudentsByGpa(rnd, 6, 0);
    for (int i = 0; i < 5; i++) assert(rnd[i].gpa >= rnd[i + 1].gpa);

    puts("All sortStudentsByGpa() test cases PASSED.");
}

static void run_all_tests(void) {
    run_findStudentById_tests();
    run_add_delete_tests();
    run_calcAverageGpa_tests();
    run_sortStudentsByGpa_tests();
    puts("ALL TESTS PASSED.");
}
#endif

int main(void) {
#ifdef RUN_TESTS
    run_all_tests();
    return 0;
#endif

    Student students[MAX_STUDENTS];
    int n = 0;

    /* Menu l?p l?i d?n khi ngu?i dùng ch?n 0 (thoát). */
    for (;;) {
        puts("\n===== STUDENT MANAGER =====");
        printf("Hien tai: %d sinh vien trong bo nho\n", n);
        puts("1) Nhap danh sach tu ban phim (toi da 5)");
        puts("2) Doc danh sach tu file (vd: classA.txt)");
        puts("3) In danh sach ra man hinh");
        puts("4) Sap xep theo GPA (tang/giam)");
        puts("5) Tim kiem sinh vien theo ID");
        puts("6) Them sinh vien moi (chan trung ID)");
        puts("7) Xoa sinh vien theo ID");
        puts("8) Luu danh sach ra file");
        puts("9) Tinh GPA trung binh");
        puts("10) Nap 10 sinh vien gia lap de test");
        puts("0) Thoat");

        int choice = read_int_range("Chon: ", 0, 10);
        if (choice == 0) break; /* Thoát chuong trình */

        switch (choice) {
            case 1: { /* Nh?p danh sách t? bàn phím (t?i da 5 SV) */
                n = read_int_range("Nhap so sinh vien (1..5): ", 1, MAX_INPUT_STUDENTS);
                for (int i = 0; i < n; i++) {
                    printf("\n-- Nhap sinh vien thu %d --\n", i + 1);
                    students[i].id = read_int_range("ID: ", 0, 1000000000);
                    read_text("Name: ", students[i].name, sizeof(students[i].name));
                    students[i].age = read_int_range("Age: ", 1, 150);
                    students[i].gpa = read_float_range("GPA (0.00..4.00): ", 0.0f, 4.0f);
                }
                puts("Da nhap xong danh sach.");
                break;
            }
            case 2: { /* Ð?c danh sách t? file (classA.txt/classB.txt/...) */
                char filename[128];
                read_text("Nhap ten file can doc (vd: classA.txt): ", filename, sizeof(filename));
                int m = readFromFile(filename, students, MAX_STUDENTS);
                if (m <= 0) {
                    puts("Khong doc duoc (file rong/khong ton tai/format sai).");
                } else {
                    n = m;
                    printf("Da doc %d sinh vien tu file %s\n", n, filename);
                }
                break;
            }
            case 3: { /* In danh sách ra màn hình (d?ng b?ng) */
                if (n <= 0) {
                    puts("Danh sach rong.");
                } else {
                    puts("\nDANH SACH SINH VIEN:");
                    print_table(students, n);
                }
                break;
            }
            case 4: { /* S?p x?p theo GPA */
                if (n <= 0) {
                    puts("Danh sach rong. Khong the sap xep.");
                    break;
                }
                int sort_choice = read_int_range("Chon kieu sap xep (1=Tang, 2=Giam): ", 1, 2);
                if (sort_choice == 1) sortStudentsByGpa(students, n, 1);
                if (sort_choice == 2) sortStudentsByGpa(students, n, 0);
                puts("Da sap xep xong.");
                break;
            }
            case 5: { /* Tìm ki?m theo ID */
                if (n <= 0) {
                    puts("Danh sach rong. Khong the tim kiem.");
                    break;
                }
                char line[128];
                printf("Nhap ID can tim: ");
                fflush(stdout);
                if (fgets(line, sizeof(line), stdin)) {
                    int id = (int)strtol(line, NULL, 10);
                    int idx = findStudentById(students, n, id);
                    if (idx >= 0) {
                        puts("Tim thay sinh vien:");
                        print_table(&students[idx], 1);
                    } else if (idx == -2) {
                        puts("ID am (khong hop le).");
                    } else {
                        puts("Khong tim thay sinh vien voi ID da nhap.");
                    }
                }
                break;
            }
            case 6: { /* Thêm sinh viên m?i (ch?n trùng ID) */
                if (n >= MAX_STUDENTS) {
                    puts("Danh sach day. Khong the them.");
                    break;
                }
                Student ns;
                puts("\n-- Them sinh vien moi --");
                ns.id = read_int_range("ID: ", -1000000000, 1000000000);
                read_text("Name: ", ns.name, sizeof(ns.name));
                ns.age = read_int_range("Age: ", 1, 150);
                ns.gpa = read_float_range("GPA (0.00..4.00): ", 0.0f, 4.0f);

                int rc = addStudent(students, &n, MAX_STUDENTS, ns);
                if (rc == 1) puts("Da them sinh vien.");
                else if (rc == -1) puts("Khong them duoc: ID bi trung.");
                else if (rc == -2) puts("Khong them duoc: ID am (khong hop le).");
                else puts("Khong them duoc: danh sach day.");
                break;
            }
            case 7: { /* Xóa sinh viên theo ID */
                if (n <= 0) {
                    puts("Danh sach rong. Khong the xoa.");
                    break;
                }
                char line[128];
                printf("Nhap ID can xoa: ");
                fflush(stdout);
                if (fgets(line, sizeof(line), stdin)) {
                    int id = (int)strtol(line, NULL, 10);
                    int rc = deleteStudentById(students, &n, id);
                    if (rc == 1) puts("Da xoa sinh vien.");
                    else if (rc == -2) puts("ID am (khong hop le).");
                    else puts("Khong tim thay sinh vien de xoa.");
                }
                break;
            }
            case 8: { /* Luu danh sách ra file */
                if (n <= 0) {
                    puts("Danh sach rong. Khong co gi de luu.");
                    break;
                }
                char out_file[128];
                read_text("Nhap ten file luu (vd: classA.txt): ", out_file, sizeof(out_file));
                if (saveToFile(out_file, students, n)) {
                    printf("Da luu danh sach vao file %s\n", out_file);
                } else {
                    puts("Luu file that bai.");
                }
                break;
            }
            case 9: { /* Tính GPA trung bình */
                if (n <= 0) {
                    puts("Danh sach rong. GPA trung binh = 0.00");
                } else {
                    printf("GPA trung binh: %.2f\n", calcAverageGpa(students, n));
                }
                break;
            }
            case 10: { /* N?p 10 sinh viên gi? l?p d? test */
                loadFakeStudents10(students, &n);
                puts("Da nap 10 sinh vien gia lap.");
                break;
            }
            default:
                puts("Lua chon khong hop le.");
                break;
        }
    }

    return 0;
}



