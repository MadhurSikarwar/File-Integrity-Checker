/*
 * FILE INTEGRITY CHECKER - THE FINAL MEGA EDITION
 * -----------------------------------------------------
 * A Full-Stack System Security Tool written in C.
 * * [FEATURE LIST]
 * 1. CRYPTOGRAPHY: OpenSSL SHA-256 Hashing.
 * 2. MULTITHREADING: Background Directory Scanner (GThread).
 * 3. DATABASE: SQLite3 History Log (Persistent).
 * 4. VISUALS:
 * - Global Statistics Pie Chart (Cairo Graphics).
 * - Per-File History Bar Chart Popup (Double-click history row).
 * 5. FILE SYSTEM TOOLS:
 * - Recursive Directory Walking.
 * - Create New File / Create New Folder.
 * 6. UI/UX:
 * - Sortable Columns (Name, Extension, Hash).
 * - High-Contrast Dark Theme (CSS).
 * - Custom Iconography & Sidebar Navigation.
 */

#include <gtk/gtk.h>
#include <openssl/evp.h>
#include <sqlite3.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <cairo.h>
#include <math.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#endif

// ============================================================
// 1. CSS STYLING (High Contrast Dark Theme)
// ============================================================
const char *css_data = 
    /* Main Window */
    "window.background { background: linear-gradient(135deg, #1a1f2e 0%, #2c3e50 100%); color: #ecf0f1; font-family: 'Segoe UI', Roboto, Sans; }"
    
    /* Sidebar */
    ".sidebar { background-color: #17202a; border-right: 3px solid #8e44ad; }"
    
    /* Navigation Buttons */
    "stackswitcher button { color: #bdc3c7; background: transparent; border: none; padding: 15px; font-weight: bold; border-left: 5px solid transparent; }"
    "stackswitcher button:checked { background-color: #2c3e50; color: white; border-left-color: #f1c40f; }"
    "stackswitcher button:hover { background-color: #34495e; color: white; border-left-color: #3498db; }"
    
    /* Card Containers */
    ".card { background: linear-gradient(145deg, #2c3e50 0%, #34495e 100%); border-radius: 16px; padding: 25px; margin: 20px; border-top: 5px solid #3498db; box-shadow: 0 10px 30px rgba(0,0,0,0.6), 0 0 20px rgba(52,152,219,0.1); transition: transform 0.3s ease, box-shadow 0.3s ease; }"
    ".card:hover { transform: translateY(-2px); box-shadow: 0 15px 40px rgba(0,0,0,0.7), 0 0 30px rgba(52,152,219,0.2); }"
    ".big-label { font-size: 24px; font-weight: 800; color: #f1c40f; margin: 25px; }"
    ".card-title { font-size: 18px; font-weight: 700; color: #3498db; margin-bottom: 15px; }"
    
    /* Input Fields */
    ".hash-entry { font-family: 'Consolas'; font-size: 13px; background: #1a252f; color: #f39c12; border: 2px solid #8e44ad; padding: 8px; border-radius: 5px; }"
    
    /* Buttons */
    "button.btn-action { background-image: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; border-radius: 8px; font-weight: bold; padding: 10px 20px; border: none; box-shadow: 0 4px 15px rgba(102,126,234,0.4); transition: all 0.3s ease; }"
    "button.btn-verify { background-image: linear-gradient(135deg, #11998e 0%, #38ef7d 100%); color: white; border-radius: 8px; font-weight: bold; padding: 10px 20px; border: none; box-shadow: 0 4px 15px rgba(56,239,125,0.4); transition: all 0.3s ease; }"
    "button.btn-secondary { background-image: linear-gradient(135deg, #575757 0%, #3f3f3f 100%); color: white; border-radius: 8px; font-weight: bold; padding: 10px 20px; border: none; transition: all 0.3s ease; }"
    "button:hover { opacity: 1; transform: translateY(-2px) scale(1.03); box-shadow: 0 6px 20px rgba(0,0,0,0.3); }"
    
    /* Progress Bar */
    "progressbar progress { background-image: linear-gradient(90deg, #f093fb 0%, #f5576c 100%); border-radius: 10px; animation: pulse 2s ease-in-out infinite; }"
    "progressbar trough { background-color: #1a252f; border-radius: 10px; border: 1px solid #34495e; }"
    "@keyframes pulse { 0%, 100% { opacity: 1; } 50% { opacity: 0.8; } }"
    
    /* Tree Views (Tables) */
    "treeview { background-color: #2c3e50; color: #ecf0f1; }"
    "treeview:selected { background-color: #8e44ad; color: white; }"
    "treeview header button { background-color: #34495e; color: #f1c40f; font-weight: bold; }"
    
    /* FILE CHOOSER FIXES (Light Mode Force) */
    "filechooser { background-color: #212f3c; color: #ecf0f1; }"
    "filechooser .view { background-color: #2c3e50; color: #ecf0f1; }"
    "filechooser .view:selected { background-color: #8e44ad; color: #ffffff; }"
    "filechooser placessidebar { background-color: #17202a; color: #ecf0f1; }"
    "filechooser placessidebar row { color: #ecf0f1; }"
    "filechooser placessidebar row:selected { background-color: #8e44ad; color: #ffffff; }"
    "filechooser pathbar button { color: #ecf0f1; background-color: #34495e; margin: 2px; }"
    "filechooser button { color: #ecf0f1; background-image: linear-gradient(to right, #7f8c8d, #636e72); border: none; }"
    "filechooser button:hover { background-image: linear-gradient(to right, #95a5a6, #7f8c8d); }"
    "filechooser entry { color: #ecf0f1; background-color: #34495e; border: 1px solid #5d6d7e; }"
    "filechooser label { color: #ecf0f1; }";

// Light Mode Theme
const char *css_data_light = 
    /* Main Window */
    "window.background { background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%); color: #2c3e50; font-family: 'Segoe UI', Roboto, Sans; }"
    
    /* Sidebar */
    ".sidebar { background-color: #ecf0f1; border-right: 3px solid #3498db; }"
    
    /* Navigation Buttons */
    "stackswitcher button { color: #7f8c8d; background: transparent; border: none; padding: 15px; font-weight: bold; border-left: 5px solid transparent; }"
    "stackswitcher button:checked { background-color: #d5dbdb; color: #2c3e50; border-left-color: #f39c12; }"
    "stackswitcher button:hover { background-color: #bdc3c7; color: #2c3e50; border-left-color: #3498db; }"
    
    /* Card Containers */
    ".card { background: linear-gradient(145deg, #ffffff 0%, #f8f9fa 100%); border-radius: 16px; padding: 25px; margin: 20px; border-top: 5px solid #3498db; box-shadow: 0 10px 30px rgba(0,0,0,0.1), 0 0 20px rgba(52,152,219,0.05); transition: transform 0.3s ease, box-shadow 0.3s ease; }"
    ".card:hover { transform: translateY(-2px); box-shadow: 0 15px 40px rgba(0,0,0,0.15), 0 0 30px rgba(52,152,219,0.1); }"
    ".big-label { font-size: 24px; font-weight: 800; color: #e67e22; margin: 25px; }"
    ".card-title { font-size: 18px; font-weight: 700; color: #3498db; margin-bottom: 15px; }"
    
    /* Input Fields */
    ".hash-entry { font-family: 'Consolas'; font-size: 13px; background: #ecf0f1; color: #e67e22; border: 2px solid #3498db; padding: 8px; border-radius: 5px; }"
    
    /* Buttons */
    "button.btn-action { background-image: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; border-radius: 8px; font-weight: bold; padding: 10px 20px; border: none; box-shadow: 0 4px 15px rgba(102,126,234,0.4); transition: all 0.3s ease; }"
    "button.btn-verify { background-image: linear-gradient(135deg, #11998e 0%, #38ef7d 100%); color: white; border-radius: 8px; font-weight: bold; padding: 10px 20px; border: none; box-shadow: 0 4px 15px rgba(56,239,125,0.4); transition: all 0.3s ease; }"
    "button.btn-secondary { background-image: linear-gradient(135deg, #757575 0%, #616161 100%); color: white; border-radius: 8px; font-weight: bold; padding: 10px 20px; border: none; transition: all 0.3s ease; }"
    "button:hover { opacity: 1; transform: translateY(-2px) scale(1.03); box-shadow: 0 6px 20px rgba(0,0,0,0.2); }"
    
    /* Progress Bar */
    "progressbar progress { background-image: linear-gradient(90deg, #667eea 0%, #764ba2 100%); border-radius: 10px; }"
    "progressbar trough { background-color: #e0e0e0; border-radius: 10px; border: 1px solid #bdbdbd; }"
    
    /* Tree Views */
    "treeview { background-color: #ffffff; color: #2c3e50; }"
    "treeview:selected { background-color: #3498db; color: white; }"
    "treeview header button { background-color: #ecf0f1; color: #2c3e50; font-weight: bold; }"
    
    /* File Chooser */
    "filechooser { background-color: #f5f5f5; color: #2c3e50; }"
    "filechooser .view { background-color: #ffffff; color: #2c3e50; }"
    "filechooser .view:selected { background-color: #3498db; color: #ffffff; }"
    "filechooser placessidebar { background-color: #ecf0f1; color: #2c3e50; }"
    "filechooser button { color: #2c3e50; background-color: #e0e0e0; }"
    "filechooser entry { color: #2c3e50; background-color: #ffffff; border: 1px solid #bdbdbd; }";

// ============================================================
// 2. GLOBAL APPLICATION STATE
// ============================================================
typedef struct {
    GtkWidget *window;
    GtkWidget *stack;
    
    // -- Tab 1: Single File --
    GtkWidget *entry_single_hash;
    GtkWidget *lbl_single_status;
    GtkWidget *btn_save;
    GtkWidget *btn_verify;
    char single_file_path[1024];
    char single_hash[128];

    // -- Tab 2: Directory Scanner --
    GtkWidget *lbl_dir_path;
    GtkWidget *btn_scan_dir;
    GtkWidget *progress_bar;
    GtkListStore *dir_store; 
    gboolean is_scanning;
    char current_scan_dir[1024];

    // -- Tab 3: History --
    GtkListStore *history_store;
    GtkWidget *history_tree;

    // -- Tab 4: Global Stats --
    GtkWidget *drawing_area; // Global Pie Chart
    
    // Database Handle
    sqlite3 *db;
    
    // Feature 3: Multi-Hash
    int hash_algo; // 0=SHA256, 1=MD5, 2=SHA512
    gboolean filter_noise; // Feature 6: Filter
    gboolean is_monitoring; // Feature 2: Watchdog
    
    // Performance Metrics
    time_t scan_start_time;
    int files_scanned;
    long long bytes_scanned;
    GtkWidget *lbl_metrics; // Metrics display label
    
    // Theme
    int theme_mode; // 0=dark, 1=light
    GtkCssProvider *css_provider;
} AppState;

AppState app;

// Global var for popup graph target
char popup_target_filename[1024] = {0};

// ============================================================
// 3. HELPER FUNCTIONS
// ============================================================

// Get file extension for sorting
const char* get_extension(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

// Dialog to get text input (for new file/folder)
char* show_input_dialog(GtkWindow *parent, const char *title) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(title, parent, GTK_DIALOG_MODAL, "_Cancel", GTK_RESPONSE_CANCEL, "_Create", GTK_RESPONSE_ACCEPT, NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(content_area), entry);
    gtk_widget_show_all(dialog);
    
    char *result = NULL;
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        result = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
    }
    gtk_widget_destroy(dialog);
    return result;
}

// ============================================================
// 4. DATABASE LAYER (SQLite)
// ============================================================

void init_db() {
    sqlite3_open("integrity_history.db", &app.db);
    
    // 1. History Log Table
    const char *sql_hist = "CREATE TABLE IF NOT EXISTS history ("
                           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                           "timestamp TEXT, filename TEXT, hash TEXT, result TEXT);";
    sqlite3_exec(app.db, sql_hist, 0, 0, NULL);

    // 2. Snapshot Tables (Feature 1)
    const char *sql_snap = "CREATE TABLE IF NOT EXISTS snapshots ("
                           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                           "timestamp TEXT, description TEXT, root_dir TEXT);";
    sqlite3_exec(app.db, sql_snap, 0, 0, NULL);

    const char *sql_entries = "CREATE TABLE IF NOT EXISTS snapshot_entries ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                              "snapshot_id INTEGER, file_path TEXT, file_hash TEXT,"
                              "FOREIGN KEY(snapshot_id) REFERENCES snapshots(id));";
    sqlite3_exec(app.db, sql_entries, 0, 0, NULL);
}

void db_insert_log(const char *filename, const char *hash, const char *result) {
    char sql[2048];
    time_t now = time(NULL);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    snprintf(sql, sizeof(sql), "INSERT INTO history (timestamp, filename, hash, result) VALUES ('%s', '%s', '%s', '%s');",
             time_str, filename, hash, result);
    sqlite3_exec(app.db, sql, 0, 0, NULL);
}

// --- Snapshot DB Functions ---
int db_create_snapshot(const char *description, const char *root_dir) {
    char sql[2048];
    time_t now = time(NULL);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

    snprintf(sql, sizeof(sql), "INSERT INTO snapshots (timestamp, description, root_dir) VALUES ('%s', '%s', '%s');",
             time_str, description, root_dir);
    
    if (sqlite3_exec(app.db, sql, 0, 0, NULL) == SQLITE_OK) {
        return (int)sqlite3_last_insert_rowid(app.db);
    }
    return -1;
}

void db_add_snapshot_entry(int snapshot_id, const char *path, const char *hash) {
    // Use prepared statement for bulk inserts (Optimization)
    // For simplicity in this step, we use snprintf, but prepared statements would be better for distinct inserts.
    // Given the context, let's keep it simple first.
    char *sql = sqlite3_mprintf("INSERT INTO snapshot_entries (snapshot_id, file_path, file_hash) VALUES (%d, '%q', '%q');",
                                snapshot_id, path, hash);
    sqlite3_exec(app.db, sql, 0, 0, NULL);
    sqlite3_free(sql);
}

void db_load_history() {
    gtk_list_store_clear(app.history_store);
    sqlite3_stmt *stmt;
    const char *sql = "SELECT timestamp, filename, hash, result FROM history ORDER BY id DESC LIMIT 100;";
    
    if (sqlite3_prepare_v2(app.db, sql, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            GtkTreeIter iter;
            gtk_list_store_append(app.history_store, &iter);
            gtk_list_store_set(app.history_store, &iter,
                0, sqlite3_column_text(stmt, 0),
                1, sqlite3_column_text(stmt, 1),
                2, sqlite3_column_text(stmt, 2),
                3, sqlite3_column_text(stmt, 3),
                -1);
        }
    }
    sqlite3_finalize(stmt);
    
    // Trigger redraw of global stats if visible
    if (app.drawing_area) gtk_widget_queue_draw(app.drawing_area);
}

// ============================================================
// Configuration Persistence
// ============================================================

void save_config() {
    FILE *fp = fopen("integrity_checker.conf", "w");
    if (!fp) return;
    
    fprintf(fp, "[Settings]\n");
    fprintf(fp, "hash_algo=%d\n", app.hash_algo);
    fprintf(fp, "filter_noise=%d\n", app.filter_noise ? 1 : 0);
    fprintf(fp, "theme_mode=%d\n", app.theme_mode);
    fprintf(fp, "last_scan_dir=%s\n", app.current_scan_dir);
    fclose(fp);
}

void load_config() {
    FILE *fp = fopen("integrity_checker.conf", "r");
    if (!fp) return;
    
    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "hash_algo=")) {
            sscanf(line, "hash_algo=%d", &app.hash_algo);
        } else if (strstr(line, "filter_noise=")) {
            int val;
            sscanf(line, "filter_noise=%d", &val);
            app.filter_noise = val ? TRUE : FALSE;
        } else if (strstr(line, "theme_mode=")) {
            sscanf(line, "theme_mode=%d", &app.theme_mode);
        } else if (strstr(line, "last_scan_dir=")) {
            sscanf(line, "last_scan_dir=%1023[^\n]", app.current_scan_dir);
        }
    }
    fclose(fp);
}

// ============================================================
// 5. CRYPTOGRAPHY (SHA-256 / MD5 / SHA-512)
// ============================================================

int compute_hash(const char *filename, char *output_buffer) {
    FILE *file = fopen(filename, "rb");
    if (!file) return 0;

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    const EVP_MD *md;
    
    switch(app.hash_algo) {
        case 1: md = EVP_md5(); break;
        case 2: md = EVP_sha512(); break;
        default: md = EVP_sha256(); break;
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    unsigned char buffer[4096];
    size_t bytes_read;

    EVP_DigestInit_ex(mdctx, md, NULL);
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) != 0) {
        EVP_DigestUpdate(mdctx, buffer, bytes_read);
    }
    EVP_DigestFinal_ex(mdctx, hash, &md_len);
    EVP_MD_CTX_free(mdctx);
    fclose(file);

    for(unsigned int i = 0; i < md_len; i++) sprintf(output_buffer + (i * 2), "%02x", hash[i]);
    output_buffer[md_len * 2] = '\0';
    return 1;
}


// ============================================================
// 6. MULTITHREADED DIRECTORY SCANNER
// ============================================================

typedef struct { char *filename; char *hash; char *extension; double progress; } ScanResult;

// Called on Main Thread to update UI
gboolean on_scan_update(gpointer data) {
    ScanResult *res = (ScanResult*)data;
    if (res->filename && strcmp(res->filename, "DONE") != 0) {
        GtkTreeIter iter;
        gtk_list_store_prepend(app.dir_store, &iter);
        gtk_list_store_set(app.dir_store, &iter, 0, res->filename, 1, res->hash, 2, res->extension, -1);
        
        // Update metrics display in real-time
        if (app.lbl_metrics && app.scan_start_time > 0) {
            time_t elapsed = time(NULL) - app.scan_start_time;
            if (elapsed > 0) {
                double files_per_sec = (double)app.files_scanned / elapsed;
                double mb_per_sec = (double)app.bytes_scanned / (1024.0 * 1024.0 * elapsed);
                char metrics_text[512];
                snprintf(metrics_text, sizeof(metrics_text), 
                    "📊 <b>Metrics:</b> %d files | %.2f MB | ⚡ %.1f files/s | 💾 %.2f MB/s",
                    app.files_scanned, app.bytes_scanned / (1024.0 * 1024.0), files_per_sec, mb_per_sec);
                gtk_label_set_markup(GTK_LABEL(app.lbl_metrics), metrics_text);
            }
        }
    } else {
        gtk_label_set_text(GTK_LABEL(app.lbl_dir_path), g_strdup_printf("Scan Complete in: %s", app.current_scan_dir));
    }

    if (res->progress >= 0) gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app.progress_bar), res->progress);
    else gtk_progress_bar_pulse(GTK_PROGRESS_BAR(app.progress_bar));

    g_free(res->filename); g_free(res->hash); g_free(res->extension); g_free(res);
    return FALSE;
}

// Recursive Logic
void process_directory(const char *dir_path) {
    GDir *dir = g_dir_open(dir_path, 0, NULL);
    if (!dir) return;

    const char *name;
    while ((name = g_dir_read_name(dir)) != NULL) {
        // Feature 6: Filter Noise
        if (app.filter_noise) {
            const char *ext = get_extension(name);
            if (g_strcmp0(ext, "tmp") == 0 || g_strcmp0(ext, "log") == 0 || g_strcmp0(ext, "obj") == 0 || g_strcmp0(ext, "o") == 0) continue;
        }

        char full_path[2048];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, name);

        if (g_file_test(full_path, G_FILE_TEST_IS_DIR)) {
            process_directory(full_path);
        } else {
            char hash[129]; // Increased for SHA-512 (128 chars + null)
            if (compute_hash(full_path, hash)) {
                // Track file size for metrics
                struct stat st;
                long long file_size = 0;
                if (stat(full_path, &st) == 0) {
                    file_size = st.st_size;
                    app.bytes_scanned += file_size;
                }
                app.files_scanned++;
                
                ScanResult *res = g_malloc(sizeof(ScanResult));
                res->filename = g_strdup(full_path);
                res->hash = g_strdup(hash);
                res->extension = g_strdup(get_extension(name));
                res->progress = -1;
                g_idle_add(on_scan_update, res);
                db_insert_log(full_path, hash, "Auto-Scan");
            }
        }
    }
    g_dir_close(dir);
}

// Thread Entry Point
gpointer thread_scan_func(gpointer data) {
    char *path = (char*)data;
    strcpy(app.current_scan_dir, path);
    
    process_directory(path);
    
    ScanResult *res = g_malloc(sizeof(ScanResult));
    res->filename = g_strdup("DONE"); res->hash = NULL; res->extension = NULL; res->progress = 1.0;
    g_idle_add(on_scan_update, res);
    g_idle_add((GSourceFunc)db_load_history, NULL); // Refresh DB tab
    
    app.is_scanning = FALSE;
    g_free(path);
    return NULL;
}

// ============================================================
// 7. VISUALIZATIONS (GRAPHS)
// ============================================================

// --- A. Popup Bar Chart (Per-File) ---
gboolean draw_file_history_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
    if (strlen(popup_target_filename) == 0) return FALSE;

    guint width = gtk_widget_get_allocated_width(widget);
    guint height = gtk_widget_get_allocated_height(widget);
    
    // Dark Background
    cairo_set_source_rgb(cr, 0.13, 0.18, 0.24); 
    cairo_paint(cr);

    // Fetch data for specific file
    char sql[2048];
    snprintf(sql, sizeof(sql), "SELECT timestamp, result FROM history WHERE filename='%s' ORDER BY id ASC LIMIT 20;", popup_target_filename);
    
    sqlite3_stmt *stmt;
    int count = 0;
    struct { int status; char time[64]; } points[20];

    if (sqlite3_prepare_v2(app.db, sql, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *t = (const char*)sqlite3_column_text(stmt, 0);
            const char *r = (const char*)sqlite3_column_text(stmt, 1);
            strcpy(points[count].time, t);
            
            // Logic: Is this a "good" event or "bad"?
            if (strstr(r, "MATCH") || strstr(r, "Computed") || strstr(r, "Saved")) 
                points[count].status = 1; 
            else 
                points[count].status = 0; // Mismatch/Fail
            
            count++;
            if (count >= 20) break;
        }
    }
    sqlite3_finalize(stmt);

    if (count == 0) return FALSE;

    // Draw Bars
    double margin = 40.0;
    double bar_width = (width - 2 * margin) / count * 0.6;
    double step = (width - 2 * margin) / count;

    for (int i = 0; i < count; i++) {
        double x = margin + i * step;
        double bar_h = (height - 2 * margin) * 0.6;
        double y = height - margin - bar_h;

        // Color Logic
        if (points[i].status == 1) cairo_set_source_rgb(cr, 0.18, 0.8, 0.44); // Green
        else cairo_set_source_rgb(cr, 0.9, 0.3, 0.23); // Red

        cairo_rectangle(cr, x, y, bar_width, bar_h);
        cairo_fill(cr);

        // Labels
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_set_font_size(cr, 10);
        cairo_move_to(cr, x, y - 5);
        cairo_show_text(cr, points[i].status ? "OK" : "FAIL");
    }
    return FALSE;
}

// Function to trigger the popup
void show_file_history_popup(const char *filename) {
    strcpy(popup_target_filename, filename);
    GtkWidget *dialog = gtk_dialog_new_with_buttons("File History Analytics", GTK_WINDOW(app.window), GTK_DIALOG_MODAL, "_Close", GTK_RESPONSE_CLOSE, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 600, 400);
    
    GtkWidget *area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    // Header
    GtkWidget *lbl = gtk_label_new(NULL);
    char title[1024]; 
    snprintf(title, 1024, "<span size='large' weight='bold' foreground='#3498db'>Timeline:</span> %s", filename);
    gtk_label_set_markup(GTK_LABEL(lbl), title);
    gtk_container_add(GTK_CONTAINER(area), lbl);

    // Drawing Canvas
    GtkWidget *draw = gtk_drawing_area_new();
    gtk_widget_set_size_request(draw, 500, 300);
    g_signal_connect(draw, "draw", G_CALLBACK(draw_file_history_callback), NULL);
    gtk_container_add(GTK_CONTAINER(area), draw);

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// --- B. Global Pie Chart ---
gboolean draw_global_stats_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
    guint width = gtk_widget_get_allocated_width(widget);
    guint height = gtk_widget_get_allocated_height(widget);
    double center_x = width / 2.0;
    double center_y = height / 2.0;
    double radius = MIN(width, height) / 2.5;

    int match_count = 0, fail_count = 0;
    sqlite3_stmt *stmt;
    const char *sql = "SELECT result, COUNT(*) FROM history GROUP BY result;";
    
    if (sqlite3_prepare_v2(app.db, sql, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *res = (const char*)sqlite3_column_text(stmt, 0);
            int count = sqlite3_column_int(stmt, 1);
            if (strstr(res, "MATCH")) match_count += count;
            else if (strstr(res, "FAIL")) fail_count += count;
        }
    }
    sqlite3_finalize(stmt);

    int total = match_count + fail_count;
    if (total == 0) return FALSE;

    double match_angle = (double)match_count / total * 2 * M_PI;
    // Draw verification pie chart
    cairo_arc(cr, 200, 200, 100, 0, 2 * M_PI);
    cairo_set_source_rgb(cr, 0.2, 0.3, 0.4);
    cairo_fill_preserve(cr);
    cairo_stroke(cr);

    if (total > 0) {
        double match_angle = ((double)match_count / total) * 2 * M_PI;
        
        cairo_move_to(cr, 200, 200);
        cairo_arc(cr, 200, 200, 100, 0, match_angle);
        cairo_close_path(cr);
        cairo_set_source_rgb(cr, 0.18, 0.8, 0.44);
        cairo_fill(cr);
        
        cairo_move_to(cr, 200, 200);
        cairo_arc(cr, 200, 200, 100, match_angle, 2 * M_PI);
        cairo_close_path(cr);
        cairo_set_source_rgb(cr, 0.91, 0.3, 0.24);
        cairo_fill(cr);
    }
    
    // Labels
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 20);
    
    char label[64];
    snprintf(label, sizeof(label), "Match: %d", match_count);
    cairo_move_to(cr, 30, 350); cairo_show_text(cr, label);
    snprintf(label, sizeof(label), "Fail: %d", fail_count);
    cairo_move_to(cr, 30, 380); cairo_show_text(cr, label);
    
    // File Type Statistics (Right side)
    cairo_set_font_size(cr, 16);
    cairo_move_to(cr, 450, 30);
    cairo_show_text(cr, "File Types");
    
    // Count file types from history
    GHashTable *ext_counts = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    sqlite3_stmt *stmt2;
    if (sqlite3_prepare_v2(app.db, "SELECT filename FROM history;", -1, &stmt2, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt2) == SQLITE_ROW) {
            const char *fname = (const char*)sqlite3_column_text(stmt2, 0);
            const char *ext = get_extension(fname);
            if (strlen(ext) > 0) {
                int *count = (int*)g_hash_table_lookup(ext_counts, ext);
                if (count) {
                    (*count)++;
                } else {
                    int *new_count = g_malloc(sizeof(int));
                    *new_count = 1;
                    g_hash_table_insert(ext_counts, g_strdup(ext), new_count);
                }
            }
        }
    }
    sqlite3_finalize(stmt2);
    
    // Draw extension counts as small pie
    int total_files = 0;
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, ext_counts);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        total_files += *(int*)value;
    }
    
    if (total_files > 0) {
        double angle = 0;
        double colors[][3] = {{0.4,0.76,0.65}, {0.99,0.55,0.38}, {0.55,0.63,0.80}, {0.91,0.54,0.77}, {0.65,0.85,0.33}};
        int color_idx = 0;
        
        g_hash_table_iter_init(&iter, ext_counts);
        while (g_hash_table_iter_next(&iter, &key, &value)) {
            int count = *(int*)value;
            double sweep = (count / (double)total_files) * 2 * M_PI;
            
            cairo_move_to(cr, 550, 150);
            cairo_arc(cr, 550, 150, 80, angle, angle + sweep);
            cairo_close_path(cr);
            cairo_set_source_rgb(cr, colors[color_idx % 5][0], colors[color_idx % 5][1], colors[color_idx % 5][2]);
            cairo_fill(cr);
            
            angle += sweep;
            color_idx++;
        }
        
        // Legend
        int y_pos = 260;
        color_idx = 0;
        cairo_set_font_size(cr, 12);
        g_hash_table_iter_init(&iter, ext_counts);
        while (g_hash_table_iter_next(&iter, &key, &value)) {
            cairo_set_source_rgb(cr, colors[color_idx % 5][0], colors[color_idx % 5][1], colors[color_idx % 5][2]);
            cairo_rectangle(cr, 450, y_pos - 10, 15, 15);
            cairo_fill(cr);
            
            cairo_set_source_rgb(cr, 1, 1, 1);
            cairo_move_to(cr, 475, y_pos);
            snprintf(label, sizeof(label), ".%s: %d", (char*)key, *(int*)value);
            cairo_show_text(cr, label);
            
            y_pos += 20;
            color_idx++;
            if (color_idx >= 8) break; // Limit display
        }
    }
    
    g_hash_table_destroy(ext_counts);
    
    return FALSE;
}

// ============================================================
// 8. SIGNAL HANDLERS & CALLBACKS
// ============================================================

void on_history_row_activated(GtkTreeView *tree, GtkTreePath *path, GtkTreeViewColumn *col, gpointer data) {
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(tree);
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        char *filename;
        gtk_tree_model_get(model, &iter, 1, &filename, -1);
        show_file_history_popup(filename); // Trigger popup
        g_free(filename);
    }
}

void on_single_file_set(GtkFileChooserButton *chooser, gpointer user_data) {
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
    if (filename) {
        strcpy(app.single_file_path, filename);
        if (compute_hash(filename, app.single_hash)) {
            gtk_entry_set_text(GTK_ENTRY(app.entry_single_hash), app.single_hash);
            gtk_label_set_markup(GTK_LABEL(app.lbl_single_status), "<span color='#3498db'>Status: Hash Computed. Ready.</span>");
            gtk_widget_set_sensitive(app.btn_save, TRUE);
            gtk_widget_set_sensitive(app.btn_verify, TRUE);
            db_insert_log(filename, app.single_hash, "Computed"); 
            db_load_history();
        }
        g_free(filename);
    }
}

// Refactored Scan Logic
void start_scan(const char *path) {
    if (app.is_scanning) return;
    
    // Initialize metrics
    app.scan_start_time = time(NULL);
    app.files_scanned = 0;
    app.bytes_scanned = 0;
    
    gtk_label_set_text(GTK_LABEL(app.lbl_dir_path), g_strdup_printf("Scanning: %s", path));
    gtk_list_store_clear(app.dir_store);
    app.is_scanning = TRUE;
    GThread *t = g_thread_new("scanner", thread_scan_func, g_strdup(path));
    g_thread_unref(t);
}

void on_scan_dir_clicked(GtkWidget *btn, gpointer data) {
    if (app.is_scanning) return;
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Select Folder", GTK_WINDOW(app.window), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "_Cancel", GTK_RESPONSE_CANCEL, "_Select Folder", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        start_scan(path);
        g_free(path);
    } else { gtk_widget_destroy(dialog); }
}

void on_new_file_clicked(GtkWidget *btn, gpointer data) {
    if (strlen(app.current_scan_dir) == 0) {
        GtkWidget *msg = gtk_message_dialog_new(GTK_WINDOW(app.window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Scan a directory first.");
        gtk_dialog_run(GTK_DIALOG(msg)); gtk_widget_destroy(msg); return;
    }
    char *name = show_input_dialog(GTK_WINDOW(app.window), "New File Name");
    if (name) {
        char full_path[2048]; snprintf(full_path, sizeof(full_path), "%s/%s", app.current_scan_dir, name);
        FILE *fp = fopen(full_path, "w");
        if (fp) fclose(fp);
        g_free(name);
        // Rescan to show new file
        app.is_scanning = TRUE;
        GThread *t = g_thread_new("scanner", thread_scan_func, g_strdup(app.current_scan_dir));
        g_thread_unref(t);
    }
}

void on_new_folder_clicked(GtkWidget *btn, gpointer data) {
    if (strlen(app.current_scan_dir) == 0) {
        GtkWidget *msg = gtk_message_dialog_new(GTK_WINDOW(app.window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Scan a directory first.");
        gtk_dialog_run(GTK_DIALOG(msg)); gtk_widget_destroy(msg); return;
    }
    char *name = show_input_dialog(GTK_WINDOW(app.window), "New Folder Name");
    if (name) {
        char full_path[2048]; snprintf(full_path, sizeof(full_path), "%s/%s", app.current_scan_dir, name);
        mkdir(full_path, 0700);
        g_free(name);
        app.is_scanning = TRUE;
        GThread *t = g_thread_new("scanner", thread_scan_func, g_strdup(app.current_scan_dir));
        g_thread_unref(t);
    }
}

void on_save_clicked(GtkWidget *btn, gpointer data) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save Hash", GTK_WINDOW(app.window), GTK_FILE_CHOOSER_ACTION_SAVE, "_Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, NULL);
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(chooser);
        FILE *fp = fopen(filename, "w");
        if (fp) { fprintf(fp, "%s", app.single_hash); fclose(fp); db_insert_log(app.single_file_path, app.single_hash, "Saved Hash"); }
        g_free(filename);
    } gtk_widget_destroy(dialog);
}

void on_verify_clicked(GtkWidget *btn, gpointer data) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Load Hash", GTK_WINDOW(app.window), GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL, "_Check Integrity", GTK_RESPONSE_ACCEPT, NULL);
    GtkWidget *accept_btn = gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
    if(accept_btn) gtk_style_context_add_class(gtk_widget_get_style_context(accept_btn), "btn-verify");

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        FILE *fp = fopen(filename, "r");
        if (fp) {
            char stored[128];
            if (fgets(stored, 128, fp)) {
                stored[strcspn(stored, "\r\n")] = 0;
                if (strcmp(app.single_hash, stored) == 0) {
                    gtk_label_set_markup(GTK_LABEL(app.lbl_single_status), "<span foreground='#2ecc71' weight='bold' size='large'>✔ MATCH CONFIRMED</span>");
                    db_insert_log(app.single_file_path, app.single_hash, "Verified: MATCH");
                } else {
                    gtk_label_set_markup(GTK_LABEL(app.lbl_single_status), "<span foreground='#e74c3c' weight='bold' size='large'>✘ HASH MISMATCH</span>");
                    db_insert_log(app.single_file_path, app.single_hash, "Verified: FAIL");
                }
            } fclose(fp);
        } g_free(filename); db_load_history();
    } gtk_widget_destroy(dialog);
}

// ============================================================
// 9. UI LAYOUT CONSTRUCTION
// ============================================================

// --- Snapshot Features (Feature 1) ---

// Feature 4: VirusTotal
void on_check_virustotal_clicked(GtkWidget *btn, gpointer data) {
    if (strlen(app.single_hash) == 0) return;
    char url[512];
    snprintf(url, sizeof(url), "https://www.virustotal.com/gui/file/%s", app.single_hash);
    
    // Gtk 3 way to open link
    gtk_show_uri_on_window(GTK_WINDOW(app.window), url, GDK_CURRENT_TIME, NULL);
}

void on_create_snapshot_clicked(GtkWidget *btn, gpointer data) {
    if (strlen(app.current_scan_dir) == 0) {
        GtkWidget *msg = gtk_message_dialog_new(GTK_WINDOW(app.window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Scan a directory first.");
        gtk_dialog_run(GTK_DIALOG(msg)); gtk_widget_destroy(msg); return;
    }

    char *desc = show_input_dialog(GTK_WINDOW(app.window), "Snapshot Description");
    if (desc) {
        int snap_id = db_create_snapshot(desc, app.current_scan_dir);
        if (snap_id != -1) {
            GtkTreeIter iter;
            gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(app.dir_store), &iter);
            while (valid) {
                char *path, *hash;
                gtk_tree_model_get(GTK_TREE_MODEL(app.dir_store), &iter, 0, &path, 1, &hash, -1);
                db_add_snapshot_entry(snap_id, path, hash);
                g_free(path); g_free(hash);
                valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(app.dir_store), &iter);
            }
            GtkWidget *msg = gtk_message_dialog_new(GTK_WINDOW(app.window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Snapshot created successfully!");
            gtk_dialog_run(GTK_DIALOG(msg)); gtk_widget_destroy(msg);
        }
        g_free(desc);
    }
}

void show_comparison_results(GList *results) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Comparison Results: Current vs Baseline", GTK_WINDOW(app.window), GTK_DIALOG_MODAL, "_Close", GTK_RESPONSE_CLOSE, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 700, 500);
    
    GtkWidget *area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_widget_set_size_request(scrolled, -1, 400);
    
    GtkListStore *store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
    GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    
    GtkCellRenderer *rnd = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree), -1, "Status", rnd, "text", 0, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree), -1, "File Path", rnd, "text", 1, NULL);
    
    for (GList *l = results; l != NULL; l = l->next) {
        char *entry = (char*)l->data;
        char *sep = strchr(entry, '|');
        if (sep) {
            *sep = '\0';
            GtkTreeIter iter;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, entry, 1, sep + 1, -1);
            *sep = '|'; 
        }
    }
    
    gtk_container_add(GTK_CONTAINER(scrolled), tree);
    gtk_container_add(GTK_CONTAINER(area), scrolled);
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void on_compare_snapshot_clicked(GtkWidget *btn, gpointer data) {
    if (strlen(app.current_scan_dir) == 0) {
        GtkWidget *msg = gtk_message_dialog_new(GTK_WINDOW(app.window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Scan a directory first.");
        gtk_dialog_run(GTK_DIALOG(msg)); gtk_widget_destroy(msg); return;
    }

    int snap_id = -1;
    char sql[2048];
    snprintf(sql, sizeof(sql), "SELECT id FROM snapshots WHERE root_dir='%s' ORDER BY id DESC LIMIT 1;", app.current_scan_dir);
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(app.db, sql, -1, &stmt, 0) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) snap_id = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (snap_id == -1) {
        GtkWidget *msg = gtk_message_dialog_new(GTK_WINDOW(app.window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "No baseline snapshot found for this folder.");
        gtk_dialog_run(GTK_DIALOG(msg)); gtk_widget_destroy(msg); return;
    }

    GHashTable *snap_files = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    snprintf(sql, sizeof(sql), "SELECT file_path, file_hash FROM snapshot_entries WHERE snapshot_id=%d;", snap_id);
    if (sqlite3_prepare_v2(app.db, sql, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *p = (const char*)sqlite3_column_text(stmt, 0);
            const char *h = (const char*)sqlite3_column_text(stmt, 1);
            g_hash_table_insert(snap_files, g_strdup(p), g_strdup(h));
        }
    }
    sqlite3_finalize(stmt);

    GList *results = NULL;
    GtkTreeIter iter;
    gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(app.dir_store), &iter);
    while (valid) {
        char *path, *hash;
        gtk_tree_model_get(GTK_TREE_MODEL(app.dir_store), &iter, 0, &path, 1, &hash, -1);
        
        char *snap_hash = g_hash_table_lookup(snap_files, path);
        if (snap_hash) {
            if (strcmp(hash, snap_hash) != 0) {
                results = g_list_append(results, g_strdup_printf("MODIFIED|%s", path));
            }
            g_hash_table_remove(snap_files, path); 
        } else {
            results = g_list_append(results, g_strdup_printf("NEW|%s", path));
        }
        
        g_free(path); g_free(hash);
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(app.dir_store), &iter);
    }

    GHashTableIter h_iter;
    gpointer key, value;
    g_hash_table_iter_init(&h_iter, snap_files);
    while (g_hash_table_iter_next(&h_iter, &key, &value)) {
        results = g_list_append(results, g_strdup_printf("DELETED|%s", (char*)key));
    }

    show_comparison_results(results);
    g_hash_table_destroy(snap_files);
    g_list_free_full(results, g_free);
}

GtkWidget* create_single_page() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_style_context_add_class(gtk_widget_get_style_context(card), "card");
    gtk_box_pack_start(GTK_BOX(box), card, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(card), gtk_label_new("Single File Verification"), FALSE, FALSE, 0);

    GtkWidget *btn = gtk_file_chooser_button_new("Select File to Check", GTK_FILE_CHOOSER_ACTION_OPEN);
    g_signal_connect(btn, "file-set", G_CALLBACK(on_single_file_set), NULL);
    GList *children = gtk_container_get_children(GTK_CONTAINER(btn));
    if (children) gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(children->data)), "btn-secondary");
    gtk_box_pack_start(GTK_BOX(card), btn, FALSE, FALSE, 0);

    app.entry_single_hash = gtk_entry_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(app.entry_single_hash), "hash-entry");
    gtk_editable_set_editable(GTK_EDITABLE(app.entry_single_hash), FALSE);
    gtk_box_pack_start(GTK_BOX(card), app.entry_single_hash, FALSE, FALSE, 10);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10); gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    app.btn_save = gtk_button_new_with_label("Save Hash");
    app.btn_verify = gtk_button_new_with_label("Verify Integrity");
    g_signal_connect(app.btn_save, "clicked", G_CALLBACK(on_save_clicked), NULL);
    g_signal_connect(app.btn_verify, "clicked", G_CALLBACK(on_verify_clicked), NULL);
    gtk_style_context_add_class(gtk_widget_get_style_context(app.btn_save), "btn-action");
    gtk_style_context_add_class(gtk_widget_get_style_context(app.btn_verify), "btn-verify");
    gtk_widget_set_tooltip_text(app.btn_save, "Save the computed hash to a file");
    gtk_widget_set_tooltip_text(app.btn_verify, "Compare current hash against saved hash file");
    gtk_widget_set_sensitive(app.btn_save, FALSE); gtk_widget_set_sensitive(app.btn_verify, FALSE);
    gtk_grid_attach(GTK_GRID(grid), app.btn_save, 0, 0, 1, 1); gtk_grid_attach(GTK_GRID(grid), app.btn_verify, 1, 0, 1, 1);
    
    GtkWidget *btn_vt = gtk_button_new_with_label("Check VirusTotal");
    g_signal_connect(btn_vt, "clicked", G_CALLBACK(on_check_virustotal_clicked), NULL);
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_vt), "btn-action");
    gtk_widget_set_tooltip_text(btn_vt, "Check file hash reputation on VirusTotal");
    gtk_grid_attach(GTK_GRID(grid), btn_vt, 2, 0, 1, 1); // Add to grid
    
    gtk_box_pack_start(GTK_BOX(card), grid, FALSE, FALSE, 0);

    app.lbl_single_status = gtk_label_new("Waiting for file...");
    gtk_box_pack_start(GTK_BOX(card), app.lbl_single_status, FALSE, FALSE, 10);
    return box;
}

GtkWidget* create_dir_page() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_style_context_add_class(gtk_widget_get_style_context(card), "card");
    gtk_widget_set_vexpand(card, TRUE);
    gtk_box_pack_start(GTK_BOX(box), card, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(card), gtk_label_new("Recursive Directory Scanner"), FALSE, FALSE, 0);

    GtkWidget *hbox_ctrl = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    app.btn_scan_dir = gtk_button_new_with_label("Select Folder & Scan");
    g_signal_connect(app.btn_scan_dir, "clicked", G_CALLBACK(on_scan_dir_clicked), NULL);
    gtk_style_context_add_class(gtk_widget_get_style_context(app.btn_scan_dir), "btn-action");
    gtk_widget_set_tooltip_text(app.btn_scan_dir, "Select a directory to recursively scan all files (Ctrl+S)");
    gtk_box_pack_start(GTK_BOX(hbox_ctrl), app.btn_scan_dir, FALSE, FALSE, 0);

    // New File/Folder Buttons
    GtkWidget *btn_new_file = gtk_button_new_from_icon_name("document-new-symbolic", GTK_ICON_SIZE_BUTTON);
    g_signal_connect(btn_new_file, "clicked", G_CALLBACK(on_new_file_clicked), NULL);
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_new_file), "btn-secondary");
    gtk_box_pack_start(GTK_BOX(hbox_ctrl), btn_new_file, FALSE, FALSE, 0);

    GtkWidget *btn_new_folder = gtk_button_new_from_icon_name("folder-new-symbolic", GTK_ICON_SIZE_BUTTON);
    g_signal_connect(btn_new_folder, "clicked", G_CALLBACK(on_new_folder_clicked), NULL);
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_new_folder), "btn-secondary");
    gtk_box_pack_start(GTK_BOX(hbox_ctrl), btn_new_folder, FALSE, FALSE, 0);

    // Snapshot Buttons
    GtkWidget *btn_snap = gtk_button_new_with_label("Create Baseline");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_snap), "btn-action");
    g_signal_connect(btn_snap, "clicked", G_CALLBACK(on_create_snapshot_clicked), NULL);
    gtk_widget_set_tooltip_text(btn_snap, "Save current directory state as a baseline snapshot");
    gtk_box_pack_start(GTK_BOX(hbox_ctrl), btn_snap, FALSE, FALSE, 0);

    GtkWidget *btn_compare = gtk_button_new_with_label("Compare vs Baseline");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_compare), "btn-verify");
    g_signal_connect(btn_compare, "clicked", G_CALLBACK(on_compare_snapshot_clicked), NULL);
    gtk_widget_set_tooltip_text(btn_compare, "Compare current state against the most recent baseline");
    gtk_box_pack_start(GTK_BOX(hbox_ctrl), btn_compare, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(card), hbox_ctrl, FALSE, FALSE, 0);

    app.lbl_dir_path = gtk_label_new("No folder selected");
    gtk_box_pack_start(GTK_BOX(card), app.lbl_dir_path, FALSE, FALSE, 0);

    // Performance Metrics Display
    app.lbl_metrics = gtk_label_new("📊 Metrics: Ready");
    gtk_label_set_use_markup(GTK_LABEL(app.lbl_metrics), TRUE);
    gtk_style_context_add_class(gtk_widget_get_style_context(app.lbl_metrics), "card-title");
    gtk_box_pack_start(GTK_BOX(card), app.lbl_metrics, FALSE, FALSE, 0);

    app.progress_bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(card), app.progress_bar, FALSE, FALSE, 0);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scrolled, TRUE);
    
    // Sortable Tree View
    app.dir_store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(app.dir_store));
    
    GtkCellRenderer *rnd = gtk_cell_renderer_text_new();
    GtkCellRenderer *mono = gtk_cell_renderer_text_new(); g_object_set(mono, "family", "Consolas", NULL);

    GtkTreeViewColumn *col_file = gtk_tree_view_column_new_with_attributes("File", rnd, "text", 0, NULL);
    gtk_tree_view_column_set_sort_column_id(col_file, 0);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col_file);

    GtkTreeViewColumn *col_type = gtk_tree_view_column_new_with_attributes("Type", rnd, "text", 2, NULL);
    gtk_tree_view_column_set_sort_column_id(col_type, 2);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col_type);

    GtkTreeViewColumn *col_hash = gtk_tree_view_column_new_with_attributes("Hash", mono, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col_hash);

    gtk_container_add(GTK_CONTAINER(scrolled), tree);
    gtk_box_pack_start(GTK_BOX(card), scrolled, TRUE, TRUE, 0);
    return box;
}

// Feature 5: Export CSV
void on_export_clicked(GtkWidget *btn, gpointer data) {
    if(!app.db) return;
    
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Export History", GTK_WINDOW(app.window), GTK_FILE_CHOOSER_ACTION_SAVE, "_Cancel", GTK_RESPONSE_CANCEL, "_Export", GTK_RESPONSE_ACCEPT, NULL);
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);
    gtk_file_chooser_set_current_name(chooser, "integrity_log.csv");
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(chooser);
        FILE *fp = fopen(filename, "w");
        if (fp) {
            fprintf(fp, "ID,Timestamp,Filename,Hash,Result\n");
            sqlite3_stmt *stmt;
            const char *sql = "SELECT id, timestamp, filename, hash, result FROM history ORDER BY id DESC;";
            if (sqlite3_prepare_v2(app.db, sql, -1, &stmt, 0) == SQLITE_OK) {
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                     fprintf(fp, "%d,%s,%s,%s,%s\n",
                        sqlite3_column_int(stmt, 0),
                        (const char*)sqlite3_column_text(stmt, 1),
                        (const char*)sqlite3_column_text(stmt, 2),
                        (const char*)sqlite3_column_text(stmt, 3),
                        (const char*)sqlite3_column_text(stmt, 4));
                }
            }
            sqlite3_finalize(stmt);
            fclose(fp);
            GtkWidget *msg = gtk_message_dialog_new(GTK_WINDOW(app.window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Export Successful!");
            gtk_dialog_run(GTK_DIALOG(msg)); gtk_widget_destroy(msg);
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

// Feature 15: HTML Report Generation
void on_export_html_report(GtkWidget *btn, gpointer data) {
    if(!app.db) return;
    
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Export HTML Report", GTK_WINDOW(app.window), GTK_FILE_CHOOSER_ACTION_SAVE, "_Cancel", GTK_RESPONSE_CANCEL, "_Export", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "integrity_report.html");
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        FILE *fp = fopen(filename, "w");
        if (fp) {
            // HTML Header
            fprintf(fp, "<!DOCTYPE html><html><head><title>File Integrity Report</title>");
            fprintf(fp, "<style>body{font-family:Arial,sans-serif;background:linear-gradient(135deg,#667eea,#764ba2);color:#fff;padding:20px;}");
            fprintf(fp, ".container{max-width:1200px;margin:0 auto;background:rgba(0,0,0,0.3);padding:30px;border-radius:15px;}");
            fprintf(fp, "h1{text-align:center;font-size:2.5em;margin-bottom:10px;}h2{border-bottom:3px solid #f39c12;padding-bottom:10px;}");
            fprintf(fp, "table{width:100%%;border-collapse:collapse;margin:20px 0;background:rgba(255,255,255,0.1);}");
            fprintf(fp, "th,td{padding:12px;text-align:left;border-bottom:1px solid rgba(255,255,255,0.2);}");
            fprintf(fp, "th{background:rgba(0,0,0,0.5);font-weight:bold;}");
            fprintf(fp, "tr:hover{background:rgba(255,255,255,0.1);}");
            fprintf(fp, ".stats{display:flex;justify-content:space-around;margin:30px 0;}");
            fprintf(fp, ".stat-box{background:rgba(0,0,0,0.4);padding:20px;border-radius:10px;text-align:center;min-width:150px;}");
            fprintf(fp, ".stat-value{font-size:2em;font-weight:bold;color:#3498db;}");
            fprintf(fp, "</style></head><body><div class='container'>");
            
            fprintf(fp, "<h1>\U0001F512 File Integrity Report</h1>");
            fprintf(fp, "<p style='text-align:center;font-size:1.2em;opacity:0.8;'>Generated: %s</p>", __DATE__);
            
            // Statistics
            int total_scans = 0, match = 0, fail = 0;
            sqlite3_stmt *stmt;
            if (sqlite3_prepare_v2(app.db, "SELECT result, COUNT(*) FROM history GROUP BY result;", -1, &stmt, 0) == SQLITE_OK) {
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    const char *res = (const char*)sqlite3_column_text(stmt, 0);
                    int count = sqlite3_column_int(stmt, 1);
                    total_scans += count;
                    if (strstr(res, "MATCH")) match += count;
                    else if (strstr(res, "FAIL")) fail += count;
                }
            }
            sqlite3_finalize(stmt);
            
            fprintf(fp, "<div class='stats'>");
            fprintf(fp, "<div class='stat-box'><div class='stat-value'>%d</div><div>Total Scans</div></div>", total_scans);
            fprintf(fp, "<div class='stat-box'><div class='stat-value' style='color:#2ecc71;'>%d</div><div>Verified</div></div>", match);
            fprintf(fp, "<div class='stat-box'><div class='stat-value' style='color:#e74c3c;'>%d</div><div>Failed</div></div>", fail);
            fprintf(fp, "</div>");
            
            // Recent History
            fprintf(fp, "<h2>\U0001F4CB Recent Scan History</h2>");
            fprintf(fp, "<table><tr><th>Timestamp</th><th>File</th><th>Hash</th><th>Result</th></tr>");
            
            if (sqlite3_prepare_v2(app.db, "SELECT timestamp, filename, hash, result FROM history ORDER BY id DESC LIMIT 50;", -1, &stmt, 0) == SQLITE_OK) {
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    fprintf(fp, "<tr><td>%s</td><td>%s</td><td style='font-family:monospace;font-size:0.8em;'>%s</td><td>%s</td></tr>",
                        sqlite3_column_text(stmt, 0),
                        sqlite3_column_text(stmt, 1),
                        sqlite3_column_text(stmt, 2),
                        sqlite3_column_text(stmt, 3));
                }
            }
            sqlite3_finalize(stmt);
            
            fprintf(fp, "</table>");
            fprintf(fp, "</div></body></html>");
            fclose(fp);
            
            GtkWidget *msg = gtk_message_dialog_new(GTK_WINDOW(app.window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "HTML Report exported successfully!");
            gtk_dialog_run(GTK_DIALOG(msg)); gtk_widget_destroy(msg);
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

// History Search Filter Functions
static char* search_text_global = NULL;

gboolean history_filter_func(GtkTreeModel *model, GtkTreeIter *iter, gpointer data) {
    if (!search_text_global || strlen(search_text_global) == 0) return TRUE;
    
    char *filename, *hash;
    gtk_tree_model_get(model, iter, 1, &filename, 2, &hash, -1);
    
    gboolean visible = (strstr(filename, search_text_global) != NULL) || 
                       (strstr(hash, search_text_global) != NULL);
    g_free(filename); g_free(hash);
    return visible;
}

void on_history_search_changed(GtkEntry *entry, gpointer data) {
    GtkTreeView *tree = GTK_TREE_VIEW(data);
    GtkTreeModel *filter = gtk_tree_view_get_model(tree);
    const char *text = gtk_entry_get_text(entry);
    
    if (search_text_global) g_free(search_text_global);
    search_text_global = g_strdup(text);
    
    gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(filter));
}

GtkWidget* create_history_page() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_style_context_add_class(gtk_widget_get_style_context(card), "card");
    gtk_widget_set_vexpand(card, TRUE);
    gtk_box_pack_start(GTK_BOX(box), card, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(card), gtk_label_new("History Log (Double Click for Graph)"), FALSE, FALSE, 0);

    // Search Entry
    GtkWidget *search_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *search_label = gtk_label_new("Search:");
    GtkWidget *search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Filter by filename or hash...");
    gtk_box_pack_start(GTK_BOX(search_box), search_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(search_box), search_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(card), search_box, FALSE, FALSE, 0);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scrolled, TRUE);
    app.history_store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    
    // Use TreeModelFilter for search functionality
    GtkTreeModel *filter_model = gtk_tree_model_filter_new(GTK_TREE_MODEL(app.history_store), NULL);
    gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(filter_model), history_filter_func, NULL, NULL);
    app.history_tree = gtk_tree_view_new_with_model(filter_model);
    
    // Connect search callback
    g_signal_connect(search_entry, "changed", G_CALLBACK(on_history_search_changed), app.history_tree);
    
    GtkCellRenderer *rnd = gtk_cell_renderer_text_new();
    GtkCellRenderer *mono = gtk_cell_renderer_text_new(); g_object_set(mono, "family", "Consolas", NULL);
    
    gtk_tree_view_append_column(GTK_TREE_VIEW(app.history_tree), gtk_tree_view_column_new_with_attributes("Time", rnd, "text", 0, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(app.history_tree), gtk_tree_view_column_new_with_attributes("File", rnd, "text", 1, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(app.history_tree), gtk_tree_view_column_new_with_attributes("Hash", mono, "text", 2, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(app.history_tree), gtk_tree_view_column_new_with_attributes("Result", rnd, "text", 3, NULL));
    
    // Double click signal
    g_signal_connect(app.history_tree, "row-activated", G_CALLBACK(on_history_row_activated), NULL);

    gtk_container_add(GTK_CONTAINER(scrolled), app.history_tree);
    gtk_box_pack_start(GTK_BOX(card), scrolled, TRUE, TRUE, 0);

    GtkWidget *btn_refresh = gtk_button_new_with_label("Refresh Table");
    g_signal_connect(btn_refresh, "clicked", G_CALLBACK(db_load_history), NULL);
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_refresh), "btn-secondary");
    gtk_box_pack_start(GTK_BOX(card), btn_refresh, FALSE, FALSE, 0);

    GtkWidget *btn_export = gtk_button_new_with_label("Export to CSV");
    g_signal_connect(btn_export, "clicked", G_CALLBACK(on_export_clicked), NULL);
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_export), "btn-action");
    gtk_widget_set_tooltip_text(btn_export, "Export all history to CSV file (Ctrl+E)");
    gtk_box_pack_start(GTK_BOX(card), btn_export, FALSE, FALSE, 0);

    GtkWidget *btn_html = gtk_button_new_with_label("Export HTML Report");
    g_signal_connect(btn_html, "clicked", G_CALLBACK(on_export_html_report), NULL);
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_html), "btn-verify");
    gtk_widget_set_tooltip_text(btn_html, "Generate professional HTML report with statistics (Ctrl+H)");
    gtk_box_pack_start(GTK_BOX(card), btn_html, FALSE, FALSE, 0);
    
    return box;
}

// Duplicate File Detector
GtkWidget* create_duplicates_page() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_style_context_add_class(gtk_widget_get_style_context(card), "card");
    gtk_widget_set_vexpand(card, TRUE);
    gtk_box_pack_start(GTK_BOX(box), card, TRUE, TRUE, 0);
    
    GtkWidget *lbl_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_title), "<b>Duplicate File Detector</b>");
    gtk_box_pack_start(GTK_BOX(card), lbl_title, FALSE, FALSE, 0);
    
    GtkWidget *lbl_stats = gtk_label_new("Analyzing...");
    gtk_box_pack_start(GTK_BOX(card), lbl_stats, FALSE, FALSE, 0);
    
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scrolled, TRUE);
    
    GtkListStore *dup_store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING);
    
    sqlite3_stmt *stmt;
    const char *sql = "SELECT hash, COUNT(*) as cnt FROM history GROUP BY hash HAVING cnt > 1 ORDER BY cnt DESC;";
    
    int total_dupes = 0;
    
    if (sqlite3_prepare_v2(app.db, sql, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *hash = (const char*)sqlite3_column_text(stmt, 0);
            int count = sqlite3_column_int(stmt, 1);
            total_dupes += count - 1;
            
            sqlite3_stmt *stmt2;
            char sql2[512];
            snprintf(sql2, sizeof(sql2), "SELECT filename FROM history WHERE hash='%s' LIMIT 5;", hash);
            
            char files_list[1024] = "";
            if (sqlite3_prepare_v2(app.db, sql2, -1, &stmt2, 0) == SQLITE_OK) {
                int idx = 0;
                while (sqlite3_step(stmt2) == SQLITE_ROW && idx < 5) {
                    if (idx > 0) strcat(files_list, ", ");
                   strcat(files_list, (const char*)sqlite3_column_text(stmt2, 0));
                    idx++;
                }
                if (count > 5) strcat(files_list, "...");
            }
            sqlite3_finalize(stmt2);
            
            GtkTreeIter iter;
            gtk_list_store_append(dup_store, &iter);
            gtk_list_store_set(dup_store, &iter, 0, hash, 1, count, 2, files_list, -1);
        }
    }
    sqlite3_finalize(stmt);
    
    char stats_text[256];
    snprintf(stats_text, sizeof(stats_text), "<b>%d duplicate groups</b> found - %d duplicate files", 
        gtk_tree_model_iter_n_children(GTK_TREE_MODEL(dup_store), NULL), total_dupes);
    gtk_label_set_markup(GTK_LABEL(lbl_stats), stats_text);
    
    GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(dup_store));
    GtkCellRenderer *rnd = gtk_cell_renderer_text_new();
    GtkCellRenderer *mono = gtk_cell_renderer_text_new();
    g_object_set(mono, "family", "Consolas", NULL);
    
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree), -1, "Hash", mono, "text", 0, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree), -1, "Count", rnd, "text", 1, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree), -1, "Files", rnd, "text", 2, NULL);
    
    gtk_container_add(GTK_CONTAINER(scrolled), tree);
    gtk_box_pack_start(GTK_BOX(card), scrolled, TRUE, TRUE, 0);
    
    return box;
}

// Snapshot Management UI
GtkWidget* create_snapshots_page() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_style_context_add_class(gtk_widget_get_style_context(card), "card");
    gtk_widget_set_vexpand(card, TRUE);
    gtk_box_pack_start(GTK_BOX(box), card, TRUE, TRUE, 0);
    
    GtkWidget *lbl_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_title), "<b>Snapshot Management</b>");
    gtk_box_pack_start(GTK_BOX(card), lbl_title, FALSE, FALSE, 0);
    
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scrolled, TRUE);
    
    GtkListStore *snapshot_store = gtk_list_store_new(4, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    
    // Load snapshots from DB
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, timestamp, description, root_dir FROM snapshots ORDER BY id DESC;";
    if (sqlite3_prepare_v2(app.db, sql, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            GtkTreeIter iter;
            gtk_list_store_append(snapshot_store, &iter);
            gtk_list_store_set(snapshot_store, &iter,
                0, sqlite3_column_int(stmt, 0),
                1, sqlite3_column_text(stmt, 1),
                2, sqlite3_column_text(stmt, 2),
                3, sqlite3_column_text(stmt, 3),
                -1);
        }
    }
    sqlite3_finalize(stmt);
    
    GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(snapshot_store));
    GtkCellRenderer *rnd = gtk_cell_renderer_text_new();
    
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree), -1, "ID", rnd, "text", 0, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree), -1, "Timestamp", rnd, "text", 1, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree), -1, "Description", rnd, "text", 2, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree), -1, "Directory", rnd, "text", 3, NULL);
    
    gtk_container_add(GTK_CONTAINER(scrolled), tree);
    gtk_box_pack_start(GTK_BOX(card), scrolled, TRUE, TRUE, 0);
    
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(btn_box, GTK_ALIGN_CENTER);
    
    GtkWidget *btn_refresh = gtk_button_new_with_label("Refresh");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_refresh), "btn-secondary");
    gtk_box_pack_start(GTK_BOX(btn_box), btn_refresh, FALSE, FALSE, 0);
    
    GtkWidget *btn_delete = gtk_button_new_with_label("Delete Selected");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_delete), "btn-secondary");
    gtk_box_pack_start(GTK_BOX(btn_box), btn_delete, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(card), btn_box, FALSE, FALSE, 10);
    
    return box;
}

GtkWidget* create_stats_page() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_style_context_add_class(gtk_widget_get_style_context(card), "card");
    gtk_widget_set_vexpand(card, TRUE);
    gtk_box_pack_start(GTK_BOX(box), card, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(card), gtk_label_new("Global Integrity Statistics"), FALSE, FALSE, 0);

    app.drawing_area = gtk_drawing_area_new();
    gtk_widget_set_vexpand(app.drawing_area, TRUE);
    g_signal_connect(app.drawing_area, "draw", G_CALLBACK(draw_global_stats_callback), NULL);
    gtk_box_pack_start(GTK_BOX(card), app.drawing_area, TRUE, TRUE, 0);
    
    GtkWidget *legend_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_halign(legend_box, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(legend_box), gtk_label_new("■ MATCH (Green)"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(legend_box), gtk_label_new("■ FAIL (Red)"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(card), legend_box, FALSE, FALSE, 10);
    return box;
}

// Callback for Algo Selector
void on_algo_changed(GtkComboBox *w, gpointer d) { 
    app.hash_algo = gtk_combo_box_get_active(w); 
}
void on_filter_toggled(GtkToggleButton *b, gpointer d) {
    app.filter_noise = gtk_toggle_button_get_active(b);
}

void on_theme_toggle(GtkToggleButton *b, gpointer d) {
    app.theme_mode = gtk_toggle_button_get_active(b) ? 1 : 0;
    
    // Reload CSS
    if (app.css_provider) {
        gtk_style_context_remove_provider_for_screen(
            gdk_screen_get_default(), 
            GTK_STYLE_PROVIDER(app.css_provider)
        );
        g_object_unref(app.css_provider);
    }
    
    app.css_provider = gtk_css_provider_new();
    const char *css = app.theme_mode ? css_data_light : css_data;
    gtk_css_provider_load_from_data(app.css_provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(), 
        GTK_STYLE_PROVIDER(app.css_provider), 
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}

gboolean watchdog_timer(gpointer data) {
    if (!app.is_monitoring) return FALSE; // Stop timer
    if (strlen(app.current_scan_dir) > 0 && !app.is_scanning) {
        start_scan(app.current_scan_dir);
        // We might want to suppress UI clear/flash here for seamlessness?
        // But for "Ultimate Edition" simplicity, a fresh scan is honest.
    }
    return TRUE; // Continue
}

void on_watchdog_toggled(GtkToggleButton *b, gpointer d) {
    app.is_monitoring = gtk_toggle_button_get_active(b);
    if (app.is_monitoring) {
        g_timeout_add_seconds(15, watchdog_timer, NULL); // Scan every 15s
        gtk_label_set_text(GTK_LABEL(app.lbl_dir_path), "Watchdog Active: Auto-scanning...");
    }
}

// About Dialog with Complete OS Concepts Reference
void on_about_clicked(GtkWidget *btn, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "OS Concepts & Features",
        GTK_WINDOW(app.window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "Close", GTK_RESPONSE_CLOSE,
        NULL
    );
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 750, 700);
    
    // Create a scrolled window
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_widget_set_hexpand(scrolled, TRUE);
    
    // Content Box
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_box_pack_start(GTK_BOX(content_area), scrolled, TRUE, TRUE, 0);

    // Use GtkTextView instead of Label for better scrolling support
    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text_view), 20);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text_view), 20);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(text_view), 20);
    gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(text_view), 20);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    // Create tags for formatting
    gtk_text_buffer_create_tag(buffer, "heading", "weight", PANGO_WEIGHT_BOLD, "scale", 1.5, "foreground", "#2c3e50", NULL);
    gtk_text_buffer_create_tag(buffer, "subheading", "weight", PANGO_WEIGHT_BOLD, "scale", 1.2, "foreground", "#2980b9", "underline", PANGO_UNDERLINE_SINGLE, NULL);
    gtk_text_buffer_create_tag(buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
    gtk_text_buffer_create_tag(buffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);
    
    GtkTextIter iter;
    gtk_text_buffer_get_start_iter(buffer, &iter);

    // Insert Content
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "FILE INTEGRITY CHECKER - Ultimate Edition v2.0\n\n", -1, "heading", NULL);
    
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "📚 OPERATING SYSTEM CONCEPTS REFERENCE\n\n", -1, "heading", NULL);

    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "1. FILE SYSTEM MANAGEMENT\n", -1, "subheading", NULL);
    gtk_text_buffer_insert(buffer, &iter, "• File I/O: Uses fopen, fread, fwrite, fclose for buffered access.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "• Directory Traversal: Uses opendir, readdir to scan hierarchy.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "• Metadata: Uses stat() for size, timestamps, permissions.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "• Path Resolution: Handling absolute/relative paths.\n\n", -1);

    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "2. PROCESS MANAGEMENT\n", -1, "subheading", NULL);
    gtk_text_buffer_insert(buffer, &iter, "• Multi-threading: GThread separates UI from scanning logic.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "• Scheduling: Watchdog uses timer interrupts (15s interval).\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "• States: Transitions between Running (Scan) and Waiting (Idle).\n\n", -1);

    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "3. MEMORY MANAGEMENT\n", -1, "subheading", NULL);
    gtk_text_buffer_insert(buffer, &iter, "• Dynamic Allocation: malloc/calloc for file lists.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "• Deallocation: g_free() to prevent memory leaks.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "• Buffer Management: Fixed chunks for checking hash of large files.\n\n", -1);

    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "4. CONCURRENCY & SYNCHRONIZATION\n", -1, "subheading", NULL);
    gtk_text_buffer_insert(buffer, &iter, "• Mutual Exclusion: SQLite handles DB lock contention.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "• Thread Safety: g_idle_add() to push updates to Main Thread.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "• Race Conditions: Avoided by separating Logic and UI threads.\n\n", -1);

    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "5. I/O MANAGEMENT\n", -1, "subheading", NULL);
    gtk_text_buffer_insert(buffer, &iter, "• Buffered I/O: Standard library buffers to reduce syscalls.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "• Blocking vs Non-Blocking: Scan is blocking in worker thread.\n\n", -1);

    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "6. SECURITY\n", -1, "subheading", NULL);
    gtk_text_buffer_insert(buffer, &iter, "• Integrity: SHA-256/MD5/SHA-512 Hashing.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "• Access Control: Monitoring file permission bits.\n\n", -1);

    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "7. DATABASE (SYSTEMS)\n", -1, "subheading", NULL);
    gtk_text_buffer_insert(buffer, &iter, "• ACID: Atomicity and Durability via SQLite journaling.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "• Persistence: Saving state to disk between runs.\n\n", -1);

    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "8. INTER-PROCESS COMMUNICATION (IPC)\n", -1, "subheading", NULL);
    gtk_text_buffer_insert(buffer, &iter, "• Signals: GTK Signals (Observer pattern) for events.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "• Shared Memory: Global 'app' struct accessed by threads.\n\n", -1);

    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "✨ APPLICATION FEATURES (18 Total)\n", -1, "subheading", NULL);
    gtk_text_buffer_insert(buffer, &iter, "1. Baseline Snapshots: Capture directory state.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "2. Real-Time Watchdog: Auto-monitor changes.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "3. Multi-Hash: SHA256, MD5, SHA512 support.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "4. VirusTotal Integration: Online reputation check.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "5. CSV Export: Audit trails.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "6. Smart Filtering: Ignore .tmp/.log files.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "7. Performance Metrics: Live speed/progress.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "8. Config Persistence: Remembers settings.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "9. Snapshot UI: Manage saved baselines.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "10. Dark/Light Mode: Dynamic theme switching.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "11. Duplicate Detector: Hash-based finding.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "12. History Search: Instant log filtering.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "13. File Type Stats: Visual analytics.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "14. HTML Reports: Professional output.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "15. About Dialog: This reference guide.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "16. Keyboard Shortcuts: Ctrl+S/H/E/Q.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "17. Tooltips: Integrated API documentation.\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "18. Premium UI: Gradients & Animations.\n\n", -1);
    gtk_container_add(GTK_CONTAINER(scrolled), text_view);
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Keyboard Shortcuts Handler
gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    if (event->state & GDK_CONTROL_MASK) {
        switch (event->keyval) {
            case GDK_KEY_s:
            case GDK_KEY_S:
                if (app.btn_scan_dir) gtk_button_clicked(GTK_BUTTON(app.btn_scan_dir));
                return TRUE;
            case GDK_KEY_h:
            case GDK_KEY_H:
                on_export_html_report(NULL, NULL);
                return TRUE;
            case GDK_KEY_e:
            case GDK_KEY_E:
                on_export_clicked(NULL, NULL);
                return TRUE;
            case GDK_KEY_q:
            case GDK_KEY_Q:
                gtk_main_quit();
                return TRUE;
        }
    }
    return FALSE;
}

void activate(GtkApplication *app_inst, gpointer user_data) {
    app.css_provider = gtk_css_provider_new();
    const char *css = app.theme_mode ? css_data_light : css_data;
    gtk_css_provider_load_from_data(app.css_provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(app.css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    app.window = gtk_application_window_new(app_inst);
    gtk_window_set_title(GTK_WINDOW(app.window), "File Integrity Checker - Platinum");
    gtk_window_set_default_size(GTK_WINDOW(app.window), 1200, 800);
    gtk_style_context_add_class(gtk_widget_get_style_context(app.window), "background");

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(app.window), hbox);

    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(sidebar), "sidebar");
    gtk_widget_set_size_request(sidebar, 250, -1);
    
    GtkWidget *lbl_head = gtk_label_new("FILE\nINTEGRITY\nCHECKER");
    gtk_label_set_justify(GTK_LABEL(lbl_head), GTK_JUSTIFY_CENTER);
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl_head), "big-label");
    gtk_box_pack_start(GTK_BOX(sidebar), lbl_head, FALSE, FALSE, 30);

    // Hash Algorithm Selector
    GtkWidget *combo_algo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_algo), "SHA-256 (Default)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_algo), "MD5 (Fastest)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_algo), "SHA-512 (Strongest)");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_algo), 0);
    g_signal_connect(combo_algo, "changed", G_CALLBACK(on_algo_changed), NULL);
    gtk_box_pack_start(GTK_BOX(sidebar), combo_algo, FALSE, FALSE, 10);
    
    // Feature 6: Filter Switch
    GtkWidget *check_filter = gtk_check_button_new_with_label("Ignore Noise (.tmp, .log)");
    g_signal_connect(check_filter, "toggled", G_CALLBACK(on_filter_toggled), NULL); // Need to define this
    gtk_box_pack_start(GTK_BOX(sidebar), check_filter, FALSE, FALSE, 10);

    GtkWidget *check_watch = gtk_check_button_new_with_label("Real-time Watchdog");
    g_signal_connect(check_watch, "toggled", G_CALLBACK(on_watchdog_toggled), NULL);
    gtk_box_pack_start(GTK_BOX(sidebar), check_watch, FALSE, FALSE, 10);

    GtkWidget *check_theme = gtk_check_button_new_with_label("Light Mode");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_theme), app.theme_mode);
    g_signal_connect(check_theme, "toggled", G_CALLBACK(on_theme_toggle), NULL);
    gtk_box_pack_start(GTK_BOX(sidebar), check_theme, FALSE, FALSE, 10);
    
    // About Button
    GtkWidget *btn_about = gtk_button_new_with_label("About");
    g_signal_connect(btn_about, "clicked", G_CALLBACK(on_about_clicked), NULL);
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_about), "btn-secondary");
    gtk_box_pack_start(GTK_BOX(sidebar), btn_about, FALSE, FALSE, 10);
    
    // Tooltips
    gtk_widget_set_tooltip_text(combo_algo, "Choose hash algorithm: SHA-256 (balanced), MD5 (fast), SHA-512 (strongest)");
    gtk_widget_set_tooltip_text(check_filter, "Exclude temporary and log files from scans");
    gtk_widget_set_tooltip_text(check_watch, "Automatically re-scan every 15 seconds");
    gtk_widget_set_tooltip_text(check_theme, "Toggle between dark and light color themes");
    gtk_widget_set_tooltip_text(btn_about, "View app information and keyboard shortcuts");


    app.stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(app.stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    
    GtkWidget *switcher = gtk_stack_switcher_new();
    gtk_stack_switcher_set_stack(GTK_STACK_SWITCHER(switcher), GTK_STACK(app.stack));
    gtk_orientable_set_orientation(GTK_ORIENTABLE(switcher), GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start(GTK_BOX(sidebar), switcher, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), sidebar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), app.stack, TRUE, TRUE, 0);

    gtk_stack_add_titled(GTK_STACK(app.stack), create_single_page(), "single", "  Single Check");
    gtk_stack_add_titled(GTK_STACK(app.stack), create_dir_page(), "dir", "  Directory Scanner");
    gtk_stack_add_titled(GTK_STACK(app.stack), create_history_page(), "hist", "  History / Logs");
    gtk_stack_add_titled(GTK_STACK(app.stack), create_duplicates_page(), "dups", "  🔍 Duplicates");
    gtk_stack_add_titled(GTK_STACK(app.stack), create_snapshots_page(), "snaps", "  📸 Snapshots");
    gtk_stack_add_titled(GTK_STACK(app.stack), create_stats_page(), "stats", "  Global Stats");

    memset(app.current_scan_dir, 0, sizeof(app.current_scan_dir));
    
    load_config(); // Load saved preferences
    
    init_db(); db_load_history();
    
    // Connect keyboard shortcuts
    g_signal_connect(app.window, "key-press-event", G_CALLBACK(on_key_press), NULL);
    
    gtk_widget_show_all(app.window);
}

int main(int argc, char **argv) {
    GtkApplication *app_inst = gtk_application_new("com.gemini.mega", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app_inst, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app_inst), argc, argv);
    
    save_config(); // Save preferences on exit
    
    g_object_unref(app_inst);
    if (app.db) sqlite3_close(app.db);
    return status;
}