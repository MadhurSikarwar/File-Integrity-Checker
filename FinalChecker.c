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
    "window.background { background-color: #212f3c; color: #ecf0f1; font-family: 'Segoe UI', Roboto, Sans; }"
    
    /* Sidebar */
    ".sidebar { background-color: #17202a; border-right: 3px solid #8e44ad; }"
    
    /* Navigation Buttons */
    "stackswitcher button { color: #bdc3c7; background: transparent; border: none; padding: 15px; font-weight: bold; border-left: 5px solid transparent; }"
    "stackswitcher button:checked { background-color: #2c3e50; color: white; border-left-color: #f1c40f; }"
    "stackswitcher button:hover { background-color: #34495e; color: white; border-left-color: #3498db; }"
    
    /* Card Containers */
    ".card { background-color: #2c3e50; border-radius: 12px; padding: 25px; margin: 20px; border-top: 4px solid #3498db; box-shadow: 0 5px 15px rgba(0,0,0,0.5); }"
    ".big-label { font-size: 24px; font-weight: 800; color: #f1c40f; margin: 25px; }"
    ".card-title { font-size: 18px; font-weight: 700; color: #3498db; margin-bottom: 15px; }"
    
    /* Input Fields */
    ".hash-entry { font-family: 'Consolas'; font-size: 13px; background: #1a252f; color: #f39c12; border: 2px solid #8e44ad; padding: 8px; border-radius: 5px; }"
    
    /* Buttons */
    "button.btn-action { background-image: linear-gradient(to right, #2980b9, #8e44ad); color: white; border-radius: 5px; font-weight: bold; }"
    "button.btn-verify { background-image: linear-gradient(to right, #27ae60, #16a085); color: white; border-radius: 5px; font-weight: bold; }"
    "button.btn-secondary { background-image: linear-gradient(to right, #7f8c8d, #636e72); color: white; border-radius: 5px; font-weight: bold; }"
    "button:hover { opacity: 0.9; transform: scale(1.02); }"
    
    /* Progress Bar */
    "progressbar progress { background-image: linear-gradient(to right, #f1c40f, #e67e22); border-radius: 5px; }"
    "progressbar trough { background-color: #34495e; border-radius: 5px; }"
    
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
    const char *sql = "CREATE TABLE IF NOT EXISTS history ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "timestamp TEXT, filename TEXT, hash TEXT, result TEXT);";
    sqlite3_exec(app.db, sql, 0, 0, NULL);
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
// 5. CRYPTOGRAPHY (SHA-256)
// ============================================================

int compute_sha256(const char *filename, char *output_buffer) {
    FILE *file = fopen(filename, "rb");
    if (!file) return 0;

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    const EVP_MD *md = EVP_sha256();
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
    output_buffer[64] = '\0';
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
        char full_path[2048];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, name);

        if (g_file_test(full_path, G_FILE_TEST_IS_DIR)) {
            process_directory(full_path);
        } else {
            char hash[65];
            if (compute_sha256(full_path, hash)) {
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
    double fail_angle = (double)fail_count / total * 2 * M_PI;
    double current_angle = -M_PI / 2;

    // Green Slice
    cairo_set_source_rgb(cr, 0.18, 0.8, 0.44); 
    cairo_move_to(cr, center_x, center_y);
    cairo_arc(cr, center_x, center_y, radius, current_angle, current_angle + match_angle);
    cairo_close_path(cr); cairo_fill(cr);
    current_angle += match_angle;

    // Red Slice
    cairo_set_source_rgb(cr, 0.9, 0.3, 0.23); 
    cairo_move_to(cr, center_x, center_y);
    cairo_arc(cr, center_x, center_y, radius, current_angle, current_angle + fail_angle);
    cairo_close_path(cr); cairo_fill(cr);

    // Center Hole (Donut)
    cairo_set_source_rgb(cr, 0.17, 0.24, 0.31); 
    cairo_arc(cr, center_x, center_y, radius * 0.6, 0, 2 * M_PI);
    cairo_fill(cr);

    // Text
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_select_font_face(cr, "Segoe UI", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 20);
    char txt[64]; snprintf(txt, 64, "Total: %d", total);
    cairo_text_extents_t ext;
    cairo_text_extents(cr, txt, &ext);
    cairo_move_to(cr, center_x - ext.width/2, center_y + ext.height/2);
    cairo_show_text(cr, txt);

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
        if (compute_sha256(filename, app.single_hash)) {
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

void on_scan_dir_clicked(GtkWidget *btn, gpointer data) {
    if (app.is_scanning) return;
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Select Folder", GTK_WINDOW(app.window), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "_Cancel", GTK_RESPONSE_CANCEL, "_Select Folder", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_label_set_text(GTK_LABEL(app.lbl_dir_path), g_strdup_printf("Scanning: %s", path));
        gtk_list_store_clear(app.dir_store);
        app.is_scanning = TRUE;
        GThread *t = g_thread_new("scanner", thread_scan_func, g_strdup(path));
        g_thread_unref(t);
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
    gtk_widget_set_sensitive(app.btn_save, FALSE); gtk_widget_set_sensitive(app.btn_verify, FALSE);
    gtk_grid_attach(GTK_GRID(grid), app.btn_save, 0, 0, 1, 1); gtk_grid_attach(GTK_GRID(grid), app.btn_verify, 1, 0, 1, 1);
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
    
    gtk_box_pack_start(GTK_BOX(card), hbox_ctrl, FALSE, FALSE, 0);

    app.lbl_dir_path = gtk_label_new("No folder selected");
    gtk_box_pack_start(GTK_BOX(card), app.lbl_dir_path, FALSE, FALSE, 0);

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

GtkWidget* create_history_page() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_style_context_add_class(gtk_widget_get_style_context(card), "card");
    gtk_widget_set_vexpand(card, TRUE);
    gtk_box_pack_start(GTK_BOX(box), card, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(card), gtk_label_new("History Log (Double Click for Graph)"), FALSE, FALSE, 0);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scrolled, TRUE);
    app.history_store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    app.history_tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(app.history_store));
    
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

void activate(GtkApplication *app_inst, gpointer user_data) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, css_data, -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

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
    gtk_stack_add_titled(GTK_STACK(app.stack), create_stats_page(), "stats", "  Global Stats");

    memset(app.current_scan_dir, 0, sizeof(app.current_scan_dir));
    init_db(); db_load_history();
    gtk_widget_show_all(app.window);
}

int main(int argc, char **argv) {
    GtkApplication *app_inst = gtk_application_new("com.gemini.mega", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app_inst, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app_inst), argc, argv);
    g_object_unref(app_inst);
    if (app.db) sqlite3_close(app.db);
    return status;
}