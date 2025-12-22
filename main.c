/*
 * PROFESSIONAL FILE INTEGRITY SUITE (Gemini Edition)
 * * Features:
 * - Dashboard with Drag-and-Drop styled file picker
 * - History Log (GtkTreeView) with timestamps
 * - Algorithm Selection (SHA256, SHA1, MD5)
 * - Modern CSS Styling
 * - GTK Stack Navigation
 */

#include <gtk/gtk.h>
#include <openssl/evp.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// --- ENUMS & CONSTANTS ---
enum {
    COL_TIME,
    COL_FILENAME,
    COL_ALGO,
    COL_HASH,
    COL_STATUS,
    NUM_COLS
};

// --- GLOBAL APP STATE ---
typedef struct {
    GtkWidget *window;
    GtkWidget *stack;           // Main content switcher
    GtkWidget *history_list;    // TreeView for logs
    GtkListStore *history_store;// Data model for logs
    
    // Dashboard Widgets
    GtkWidget *lbl_filename;
    GtkWidget *entry_hash;
    GtkWidget *progress_bar;
    GtkWidget *lbl_status_dash;
    GtkWidget *btn_save;
    GtkWidget *btn_verify;
    GtkWidget *combo_algo;      // Settings dropdown
    
    // State Data
    char current_hash[128];
    char current_file_path[1024];
    const EVP_MD *selected_digest;
    char selected_algo_name[16];
} AppWidgets;

AppWidgets app;

// --- EXTENSIVE CSS STYLING ---
const char *css_provider_data = 
    "* { font-family: 'Segoe UI', Sans; }"
    "window { background-color: #2b2b2b; color: #ffffff; }"
    
    /* Sidebar Styling */
    ".sidebar { background-color: #1e1e1e; border-right: 1px solid #333; }"
    ".sidebar-btn { "
    "   background-color: transparent; color: #aaa; border: none; "
    "   text-align: left; padding: 15px; font-size: 14px; font-weight: 600;"
    "   border-radius: 0px; margin: 2px 5px;"
    "}"
    ".sidebar-btn:checked { background-color: #3e3e3e; color: #fff; border-left: 4px solid #3498db; }"
    ".sidebar-btn:hover { background-color: #333; color: #fff; }"
    
    /* Dashboard Card */
    ".dash-card { background-color: #333; border-radius: 12px; padding: 30px; margin: 20px; box-shadow: 0 4px 10px rgba(0,0,0,0.3); }"
    ".big-label { font-size: 24px; font-weight: bold; color: #3498db; margin-bottom: 20px; }"
    ".file-area { background-color: #444; border: 2px dashed #666; border-radius: 8px; padding: 20px; }"
    ".hash-display { font-family: 'Consolas', monospace; font-size: 13px; background: #222; color: #00ffcc; padding: 10px; border-radius: 4px; border: 1px solid #444; }"
    
    /* Buttons */
    ".btn-primary { background-image: none; background-color: #3498db; color: white; border-radius: 6px; padding: 8px 16px; font-weight: bold; }"
    ".btn-primary:disabled { background-color: #555; color: #888; }"
    ".btn-verify { background-image: none; background-color: #27ae60; color: white; border-radius: 6px; padding: 8px 16px; font-weight: bold; }"
    
    /* Status Labels */
    ".status-box { padding: 15px; border-radius: 0 0 12px 12px; margin-top: 10px; }"
    ".status-idle { background-color: #555; color: #ccc; }"
    ".status-good { background-color: #2ecc71; color: #fff; font-weight: bold; }"
    ".status-bad { background-color: #e74c3c; color: #fff; font-weight: bold; }"

    /* History Table */
    "treeview { background-color: #2b2b2b; color: white; }"
    "treeview:selected { background-color: #3498db; color: white; }"
    "header { background-color: #1e1e1e; color: #aaa; font-weight: bold; }";


// --- UTILITY FUNCTIONS ---

void apply_css() {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, css_provider_data, -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

void get_timestamp(char *buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, 64, "%H:%M:%S", t);
}

void add_to_history(const char *filename, const char *status_msg) {
    GtkTreeIter iter;
    char time_str[64];
    get_timestamp(time_str);
    
    gtk_list_store_prepend(app.history_store, &iter);
    gtk_list_store_set(app.history_store, &iter,
                       COL_TIME, time_str,
                       COL_FILENAME, filename,
                       COL_ALGO, app.selected_algo_name,
                       COL_HASH, app.current_hash,
                       COL_STATUS, status_msg,
                       -1);
}

// --- CORE LOGIC: HASHING ---

int compute_hash(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) return -1;

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    const EVP_MD *md = app.selected_digest ? app.selected_digest : EVP_sha256();
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    unsigned char buffer[8192];
    size_t bytes_read;

    // Show Progress (Fake pulse for UI feedback)
    gtk_progress_bar_pulse(GTK_PROGRESS_BAR(app.progress_bar));

    EVP_DigestInit_ex(mdctx, md, NULL);
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) != 0) {
        EVP_DigestUpdate(mdctx, buffer, bytes_read);
        // In a real threaded app, we'd update progress % here
        if (bytes_read % 10 == 0) while (gtk_events_pending()) gtk_main_iteration();
    }
    EVP_DigestFinal_ex(mdctx, hash, &md_len);
    EVP_MD_CTX_free(mdctx);
    fclose(file);

    for(unsigned int i = 0; i < md_len; i++) {
        sprintf(app.current_hash + (i * 2), "%02x", hash[i]);
    }
    app.current_hash[md_len * 2] = '\0';
    
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app.progress_bar), 1.0);
    return 0;
}

// --- CALLBACKS ---

void on_algo_changed(GtkComboBoxText *combo, gpointer data) {
    const char *id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo));
    if (strcmp(id, "sha256") == 0) {
        app.selected_digest = EVP_sha256();
        strcpy(app.selected_algo_name, "SHA-256");
    } else if (strcmp(id, "sha1") == 0) {
        app.selected_digest = EVP_sha1();
        strcpy(app.selected_algo_name, "SHA-1");
    } else if (strcmp(id, "md5") == 0) {
        app.selected_digest = EVP_md5();
        strcpy(app.selected_algo_name, "MD5");
    }
    
    // If file is loaded, re-compute immediately
    if (strlen(app.current_file_path) > 0) {
        compute_hash(app.current_file_path);
        gtk_entry_set_text(GTK_ENTRY(app.entry_hash), app.current_hash);
    }
}

void on_file_set(GtkFileChooserButton *chooser, gpointer user_data) {
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
    if (filename) {
        strncpy(app.current_file_path, filename, 1023);
        
        char *basename = g_path_get_basename(filename);
        char display_text[256];
        snprintf(display_text, sizeof(display_text), "File: %s", basename);
        gtk_label_set_text(GTK_LABEL(app.lbl_filename), display_text);
        g_free(basename);

        if (compute_hash(filename) == 0) {
            gtk_entry_set_text(GTK_ENTRY(app.entry_hash), app.current_hash);
            gtk_widget_set_sensitive(app.btn_save, TRUE);
            gtk_widget_set_sensitive(app.btn_verify, TRUE);
            
            // Set Dashboard Status
            gtk_label_set_text(GTK_LABEL(app.lbl_status_dash), "Hash Computed Successfully.");
            GtkStyleContext *ctx = gtk_widget_get_style_context(app.lbl_status_dash);
            gtk_style_context_remove_class(ctx, "status-bad");
            gtk_style_context_add_class(ctx, "status-idle"); // Neutral
            
            add_to_history(filename, "Computed");
        }
        g_free(filename);
    }
}

void on_save_clicked(GtkWidget *btn, gpointer data) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save Hash", GTK_WINDOW(app.window),
                                                    GTK_FILE_CHOOSER_ACTION_SAVE,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Save", GTK_RESPONSE_ACCEPT, NULL);
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);
    gtk_file_chooser_set_current_name(chooser, "checksum.txt");

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(chooser);
        FILE *fp = fopen(filename, "w");
        if (fp) {
            fprintf(fp, "%s", app.current_hash);
            fclose(fp);
            add_to_history("Hash Saved", "Success");
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

void on_verify_clicked(GtkWidget *btn, gpointer data) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open Hash File", GTK_WINDOW(app.window),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Verify", GTK_RESPONSE_ACCEPT, NULL);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        FILE *fp = fopen(filename, "r");
        int match = 0;
        
        if (fp) {
            char stored_hash[128];
            if (fgets(stored_hash, 128, fp)) {
                stored_hash[strcspn(stored_hash, "\r\n")] = 0;
                // Case insensitive compare (some tools output uppercase)
                if (strcasecmp(app.current_hash, stored_hash) == 0) match = 1;
            }
            fclose(fp);
        }

        GtkStyleContext *ctx = gtk_widget_get_style_context(app.lbl_status_dash);
        gtk_style_context_remove_class(ctx, "status-idle");
        
        if (match) {
            gtk_label_set_text(GTK_LABEL(app.lbl_status_dash), "INTEGRITY CONFIRMED: MATCH");
            gtk_style_context_remove_class(ctx, "status-bad");
            gtk_style_context_add_class(ctx, "status-good");
            add_to_history(app.current_file_path, "VERIFIED: OK");
        } else {
            gtk_label_set_text(GTK_LABEL(app.lbl_status_dash), "WARNING: HASH MISMATCH");
            gtk_style_context_remove_class(ctx, "status-good");
            gtk_style_context_add_class(ctx, "status-bad");
            add_to_history(app.current_file_path, "VERIFIED: FAIL");
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

void on_sidebar_changed(GtkStackSwitcher *switcher, GObject *pspec, gpointer data) {
    // Optional: Add animation logic here if desired
}

// --- UI CONSTRUCTION HELPERS ---

GtkWidget* create_dashboard_page() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_valign(box, GTK_ALIGN_START);
    
    // Card Container
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_style_context_add_class(gtk_widget_get_style_context(card), "dash-card");
    gtk_box_pack_start(GTK_BOX(box), card, FALSE, FALSE, 0);

    // Title
    GtkWidget *lbl_title = gtk_label_new("Compute File Hash");
    gtk_widget_set_halign(lbl_title, GTK_ALIGN_START);
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl_title), "big-label");
    gtk_box_pack_start(GTK_BOX(card), lbl_title, FALSE, FALSE, 0);

    // File Area
    GtkWidget *file_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_style_context_add_class(gtk_widget_get_style_context(file_box), "file-area");
    
    app.lbl_filename = gtk_label_new("No file currently selected");
    gtk_box_pack_start(GTK_BOX(file_box), app.lbl_filename, FALSE, FALSE, 0);

    GtkWidget *file_btn = gtk_file_chooser_button_new("Select File", GTK_FILE_CHOOSER_ACTION_OPEN);
    g_signal_connect(file_btn, "file-set", G_CALLBACK(on_file_set), NULL);
    gtk_widget_set_hexpand(file_btn, TRUE);
    gtk_box_pack_start(GTK_BOX(file_box), file_btn, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(card), file_box, FALSE, FALSE, 0);

    // Hash Display
    app.entry_hash = gtk_entry_new();
    gtk_editable_set_editable(GTK_EDITABLE(app.entry_hash), FALSE);
    gtk_style_context_add_class(gtk_widget_get_style_context(app.entry_hash), "hash-display");
    gtk_entry_set_placeholder_text(GTK_ENTRY(app.entry_hash), "Hash string will appear here...");
    gtk_entry_set_alignment(GTK_ENTRY(app.entry_hash), 0.5);
    gtk_box_pack_start(GTK_BOX(card), app.entry_hash, FALSE, FALSE, 0);
    
    // Progress Bar
    app.progress_bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(card), app.progress_bar, FALSE, FALSE, 5);

    // Action Buttons
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);

    app.btn_save = gtk_button_new_with_label("Save Hash");
    g_signal_connect(app.btn_save, "clicked", G_CALLBACK(on_save_clicked), NULL);
    gtk_style_context_add_class(gtk_widget_get_style_context(app.btn_save), "btn-primary");
    gtk_widget_set_sensitive(app.btn_save, FALSE);

    app.btn_verify = gtk_button_new_with_label("Verify Integrity");
    g_signal_connect(app.btn_verify, "clicked", G_CALLBACK(on_verify_clicked), NULL);
    gtk_style_context_add_class(gtk_widget_get_style_context(app.btn_verify), "btn-verify");
    gtk_widget_set_sensitive(app.btn_verify, FALSE);

    gtk_grid_attach(GTK_GRID(grid), app.btn_save, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), app.btn_verify, 1, 0, 1, 1);
    gtk_box_pack_start(GTK_BOX(card), grid, FALSE, FALSE, 10);

    // Status Area
    app.lbl_status_dash = gtk_label_new("Ready");
    gtk_style_context_add_class(gtk_widget_get_style_context(app.lbl_status_dash), "status-box");
    gtk_style_context_add_class(gtk_widget_get_style_context(app.lbl_status_dash), "status-idle");
    gtk_box_pack_start(GTK_BOX(card), app.lbl_status_dash, FALSE, FALSE, 0);

    return box;
}

GtkWidget* create_history_page() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box), 20);

    GtkWidget *lbl = gtk_label_new("Session Activity Log");
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl), "big-label");
    gtk_box_pack_start(GTK_BOX(box), lbl, FALSE, FALSE, 0);

    // Scrolled Window for Table
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_box_pack_start(GTK_BOX(box), scrolled, TRUE, TRUE, 0);

    // Tree View Store
    app.history_store = gtk_list_store_new(NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    app.history_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(app.history_store));
    
    // Columns
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(app.history_list), -1, "Time", renderer, "text", COL_TIME, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(app.history_list), -1, "File Name", renderer, "text", COL_FILENAME, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(app.history_list), -1, "Algorithm", renderer, "text", COL_ALGO, NULL);
    
    // Hash Column (Monospace)
    GtkCellRenderer *mono_renderer = gtk_cell_renderer_text_new();
    g_object_set(mono_renderer, "family", "Consolas", NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(app.history_list), -1, "Hash Digest", mono_renderer, "text", COL_HASH, NULL);
    
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(app.history_list), -1, "Result", renderer, "text", COL_STATUS, NULL);

    gtk_container_add(GTK_CONTAINER(scrolled), app.history_list);
    
    return box;
}

GtkWidget* create_settings_page() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_set_border_width(GTK_CONTAINER(box), 30);
    
    GtkWidget *lbl = gtk_label_new("Configuration");
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl), "big-label");
    gtk_widget_set_halign(lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(box), lbl, FALSE, FALSE, 0);
    
    // Hashing Algorithm Selection
    GtkWidget *frame_algo = gtk_frame_new("Hashing Algorithm");
    GtkWidget *frame_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(frame_box), 15);
    gtk_container_add(GTK_CONTAINER(frame_algo), frame_box);
    
    app.combo_algo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app.combo_algo), "sha256", "SHA-256 (Recommended)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app.combo_algo), "sha1", "SHA-1 (Legacy)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app.combo_algo), "md5", "MD5 (Fast/Insecure)");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(app.combo_algo), "sha256");
    
    g_signal_connect(app.combo_algo, "changed", G_CALLBACK(on_algo_changed), NULL);
    
    gtk_box_pack_start(GTK_BOX(frame_box), gtk_label_new("Select the cryptographic digest method:"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(frame_box), app.combo_algo, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(box), frame_algo, FALSE, FALSE, 0);
    
    // About Section
    GtkWidget *frame_about = gtk_frame_new("About");
    GtkWidget *about_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(about_box), 15);
    gtk_container_add(GTK_CONTAINER(frame_about), about_box);
    
    gtk_box_pack_start(GTK_BOX(about_box), gtk_label_new("Integrity Suite v2.0"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(about_box), gtk_label_new("Built with C, GTK3 & OpenSSL"), FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(box), frame_about, FALSE, FALSE, 0);
    
    return box;
}

// --- MAIN UI ASSEMBLY ---

void activate(GtkApplication *app_ptr, gpointer user_data) {
    apply_css();

    // 1. Window Setup
    app.window = gtk_application_window_new(app_ptr);
    gtk_window_set_title(GTK_WINDOW(app.window), "Integrity Suite Professional");
    gtk_window_set_default_size(GTK_WINDOW(app.window), 900, 600);
    
    // 2. Main Horizontal Container (Sidebar | Content)
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(app.window), hbox);

    // 3. Stack (The Content Pages)
    app.stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(app.stack), GTK_STACK_TRANSITION_TYPE_SLIDE_UP_DOWN);

    // Add Pages to Stack
    GtkWidget *page_dash = create_dashboard_page();
    GtkWidget *page_hist = create_history_page();
    GtkWidget *page_sett = create_settings_page();
    
    gtk_stack_add_named(GTK_STACK(app.stack), page_dash, "dashboard");
    gtk_stack_add_named(GTK_STACK(app.stack), page_hist, "history");
    gtk_stack_add_named(GTK_STACK(app.stack), page_sett, "settings");

    // 4. Sidebar Construction
    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(sidebar), "sidebar");
    gtk_widget_set_size_request(sidebar, 200, -1);

    // Sidebar Header
    GtkWidget *header_lbl = gtk_label_new("INTEGRITY\nGUARD");
    gtk_style_context_add_class(gtk_widget_get_style_context(header_lbl), "big-label");
    gtk_widget_set_margin_top(header_lbl, 20);
    gtk_widget_set_margin_bottom(header_lbl, 20);
    gtk_box_pack_start(GTK_BOX(sidebar), header_lbl, FALSE, FALSE, 0);

    // Stack Switcher (Buttons for Sidebar)
    GtkWidget *stack_switcher = gtk_stack_switcher_new();
    gtk_stack_switcher_set_stack(GTK_STACK_SWITCHER(stack_switcher), GTK_STACK(app.stack));
    gtk_orientable_set_orientation(GTK_ORIENTABLE(stack_switcher), GTK_ORIENTATION_VERTICAL);
    
    // We manually add buttons to link to stack to style them vertically
    // Note: GTK StackSwitcher is usually automatic, but for custom sidebar styling 
    // we often use RadioButtons or standard Buttons. Here we let GTK handle it 
    // but rely on CSS to fix the look.
    gtk_box_pack_start(GTK_BOX(sidebar), stack_switcher, FALSE, FALSE, 0);

    // Pack Sidebar and Stack
    gtk_box_pack_start(GTK_BOX(hbox), sidebar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), app.stack, TRUE, TRUE, 0);

    // Initialize Defaults
    app.selected_digest = EVP_sha256();
    strcpy(app.selected_algo_name, "SHA-256");

    gtk_widget_show_all(app.window);
}

int main(int argc, char **argv) {
    GtkApplication *app_inst;
    int status;

    app_inst = gtk_application_new("com.gemini.integritysuite", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app_inst, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app_inst), argc, argv);
    g_object_unref(app_inst);

    return status;
}