# File Integrity Checker - Ultimate Edition 🔐

![Version](https://img.shields.io/badge/version-2.0%20Ultimate-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)
![Features](https://img.shields.io/badge/features-18%20Complete-brightgreen.svg)

A **production-ready** security application featuring 18 comprehensive tools for file integrity monitoring, analysis, and management. Built with C, GTK3, SQLite, and OpenSSL.

---

## ✨ Complete Feature List

### 🔒 Core Security Features (1-6)

#### 1. **Baseline Snapshot Creation & Comparison**
- Creates cryptographic snapshots of directory states
- Stores file paths and hashes in SQLite database
- **Detects**: 🟢 New files, 🟡 Modified files, 🔴 Deleted files
- **Usage**: Click "Create Baseline" after scanning, then "Compare vs Baseline"

#### 2. **Real-Time Watchdog Monitoring**
- Automatic directory re-scanning every 15 seconds
- Background timer operation
- **Usage**: Toggle "Real-time Watchdog" checkbox in sidebar
- **Benefit**: Continuous monitoring without manual intervention

#### 3. **Multi-Hash Algorithm Support**
- Choose between 3 cryptographic hash functions:
  - **SHA-256** (Default, balanced security/speed)
  - **MD5** (Fastest, legacy compatibility)
  - **SHA-512** (Strongest, maximum security)
- **Usage**: Dropdown selector in sidebar
- Preference saved automatically

#### 4. **VirusTotal Integration**
- One-click hash reputation lookup
- Opens browser with automatic search
- **Usage**: Click "Check VirusTotal" after selecting file
- **Benefit**: Instant malware/threat detection

#### 5. **CSV Export**
- Export complete scan history to Excel-compatible CSV
- **Includes**: ID, Timestamp, Filename, Hash, Result
- **Usage**: Click "Export to CSV" in History tab
- Perfect for auditing and compliance

#### 6. **Smart Noise Filtering**
- Excludes temporary and log files from scans
- **Filters**: `.tmp`, `.log`, `.obj`, `.o` extensions
- **Usage**: Toggle "Ignore Noise" checkbox
- **Benefit**: Cleaner results, faster performance

---

### 📊 Advanced Analytics (7-14)

#### 7. **Performance Metrics Dashboard**
- Real-time scan statistics displayed during operation
- **Metrics**:
  - Files scanned count
  - Total MB processed
  - Files/second throughput
  - MB/second speed
- **Location**: Metrics label in Directory Scanner tab

#### 8. **Configuration Persistence**
- Automatic save/load of user preferences
- **Saved settings**:
  - Selected hash algorithm
  - Filter noise toggle state
  - Last scanned directory
  - Theme mode (dark/light)
- **File**: `integrity_checker.conf`
- Auto-loads on startup, auto-saves on exit

#### 9. **Snapshot Management UI**
- Dedicated tab for baseline management
- **Features**:
  - View all snapshots with metadata
  - Display: ID, Timestamp, Description, Directory
  - Refresh and Delete capabilities
- **Location**: "Snapshots" tab

#### 10. **Dark/Light Mode Toggle**
- Instant theme switching
- **Dark Mode**: Purple/blue gradients (#1a1f2e → #2c3e50)
- **Light Mode**: Clean white/gray (#f5f7fa → #c3cfe2)
- **Usage**: Toggle "Light Mode" checkbox in sidebar
- Full UI refresh with CSS reload

#### 11. **Duplicate File Detector**
- Hash-based duplicate file detection
- **Shows**:
  - Groups of files with identical content
  - Duplicate count per hash
  - List of duplicate files
  - Total duplicate statistics
- **Location**: "Duplicates" tab
- **Benefit**: Identify wasted space and file copies

#### 12. **History Search & Filtering**
- Real-time search through scan history
- **Search by**: Filename OR hash value
- Instant filtering as you type
- **Location**: Search box in History tab
- **Benefit**: Quickly find specific scans in large logs

#### 13. **File Type Statistics**
- Visual analytics with dual pie charts
- **Left Chart**: Verification results (Match/Fail ratio)
- **Right Chart**: File type distribution by extension
- Color-coded legend with counts
- **Location**: "Global Stats" tab

#### 14. **HTML Report Generation**
- Professional styled reports with embedded CSS
- **Includes**:
  - Statistics dashboard (Total/Verified/Failed)
  - Recent scan history table
  - Gradient styling and responsive layout
- **Usage**: Click "Export HTML Report" in History tab
- Perfect for presentations and documentation

---

### 🎨 Professional Polish (15-18)

#### 15. **About Dialog**
- Comprehensive application information
- **Displays**:
  - Version: 2.0 Ultimate Edition
  - All 18 features listed
  - Keyboard shortcuts reference
  - Credits and description
- **Usage**: Click "About" button in sidebar

#### 16. **Keyboard Shortcuts**
- Power-user hotkeys for faster workflow
- **Available shortcuts**:
  - `Ctrl+S` - Start directory scan
  - `Ctrl+H` - Export HTML report
  - `Ctrl+E` - Export CSV
  - `Ctrl+Q` - Quit application
- **Benefit**: Reduced clicking, improved productivity

#### 17. **Comprehensive Tooltips**
- Context-sensitive help on all controls
- Hover over any button/checkbox for explanation
- **Examples**:
  - Hash selector: "Choose hash algorithm: SHA-256 (balanced), MD5 (fast), SHA-512 (strongest)"
  - Scan button: "Select a directory to recursively scan all files (Ctrl+S)"

#### 18. **Premium UI/UX Design**
- Modern animated interface with professional styling
- **Features**:
  - Animated gradients on background and cards
  - Hover effects with card lifting
  - Button glows with colored shadows
  - Smooth 0.3s transitions on all interactions
  - Pulsing progress bars during scans
  - Glass-morphism effects

---

## 🚀 Installation & Setup (Windows - MSYS2)

### Prerequisites

You need **MSYS2** installed on Windows. If not installed:
1. Download from: https://www.msys2.org/
2. Run installer (default location: `C:\msys64`)
3. Open **"MSYS2 MinGW 64-bit"** terminal (important - not MSYS2 MSYS)

### Step 1: Update MSYS2

```bash
# Update package database
pacman -Syu

# If terminal closes, reopen "MSYS2 MinGW 64-bit" and run:
pacman -Su
```

### Step 2: Install Dependencies

Run this single command to install all required packages:

```bash
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-gtk3 mingw-w64-x86_64-sqlite3 mingw-w64-x86_64-openssl mingw-w64-x86_64-pkg-config
```

Press `Enter` to select all when prompted for toolchain.

**What gets installed:**
- `mingw-w64-x86_64-toolchain` - GCC compiler and build tools
- `mingw-w64-x86_64-gtk3` - GTK+ 3 GUI framework (~150 MB)
- `mingw-w64-x86_64-sqlite3` - SQLite database library
- `mingw-w64-x86_64-openssl` - OpenSSL cryptography library
- `mingw-w64-x86_64-pkg-config` - Package configuration tool

**Installation time**: 5-10 minutes (depending on internet speed)

### Step 3: Navigate to Project Directory

```bash
cd "/c/Users/YourUsername/OneDrive/Desktop/Projects/FileChecker"
# or wherever you cloned/downloaded the project
```

**Note**: MSYS2 uses Unix-style paths:
- Windows `C:\` becomes `/c/`
- Use forward slashes `/` instead of backslashes `\`

### Step 4: Compile

```bash
gcc FinalChecker.c -o FinalChecker.exe $(pkg-config --cflags --libs gtk+-3.0 sqlite3 openssl) -lm -mwindows
```

**What this does:**
- `gcc FinalChecker.c` - Compile the source file
- `-o FinalChecker.exe` - Output executable name
- `$(pkg-config ...)` - Auto-detect library paths and flags
- `-lm` - Link math library
- `-mwindows` - Create Windows GUI application (no console)

**Compile time**: 10-30 seconds

### Step 5: Run

```bash
./FinalChecker.exe
```

Or double-click `FinalChecker.exe` in Windows Explorer.

---

## 🐧 Installation (Linux)

### Ubuntu/Debian

```bash
# Install dependencies
sudo apt update
sudo apt install build-essential libgtk-3-dev libsqlite3-dev libssl-dev pkg-config

# Compile
gcc FinalChecker.c -o FinalChecker \
  `pkg-config --cflags --libs gtk+-3.0 sqlite3 libcrypto` \
  -lm -lpthread

# Run
./FinalChecker
```

### Fedora/RHEL

```bash
# Install dependencies
sudo dnf install gcc gtk3-devel sqlite-devel openssl-devel pkgconfig

# Compile (same as Ubuntu)
gcc FinalChecker.c -o FinalChecker \
  `pkg-config --cflags --libs gtk+-3.0 sqlite3 libcrypto` \
  -lm -lpthread
```

---

## 📖 Usage Guide

### Quick Start

1. **Launch the application**
2. **Select hash algorithm** from sidebar dropdown (SHA-256 recommended)
3. **Navigate to Directory Scanner tab**
4. **Click "Select Folder & Scan"** (or press `Ctrl+S`)
5. **Watch real-time metrics** during scanning
6. **Optional**: Click "Create Baseline" to save snapshot
7. **Optional**: Enable "Real-time Watchdog" for continuous monitoring

### Common Workflows

#### Detecting File Changes
1. Scan a directory
2. Click "Create Baseline"
3. Modify/add/delete some files
4. Scan again
5. Click "Compare vs Baseline"
6. Review popup showing: 🟢 New, 🟡 Modified, 🔴 Deleted files

#### Finding Duplicate Files
1. Scan directories
2. Go to "Duplicates" tab
3. View grouped files by identical hash
4. See statistics on duplicate count

#### Generating Reports
1. Go to "History / Logs" tab
2. Click "Export HTML Report" (or `Ctrl+H`)
3. Save to desired location
4. Open in browser for beautiful formatted report

#### Checking File Reputation
1. Go to "Single Check" tab
2. Select suspicious file
3. Hash automatically calculates
4. Click "Check VirusTotal"
5. Browser opens with results

---

## 🗂️ Tab Navigation

| Tab | Purpose |
|-----|---------|
| **Single Check** | Verify individual files against saved hashes |
| **Directory Scanner** | Recursive scanning with real-time metrics |
| **History / Logs** | Searchable audit trail with export options |
| **Duplicates** | Hash-based duplicate file detection |
| **Snapshots** | Baseline snapshot management |
| **Global Stats** | Pie charts showing verification & file type stats |

---

## ⚙️ Configuration

Settings are automatically saved to `integrity_checker.conf` in the same directory as the executable.

**Example config file:**
```ini
[Settings]
hash_algo=0          # 0=SHA256, 1=MD5, 2=SHA512
filter_noise=1       # 1=enabled, 0=disabled
theme_mode=0         # 0=dark, 1=light
last_scan_dir=C:\Users\...
```

---

## 🗄️ Database Schema

The application uses SQLite database `integrity_history.db` with 3 tables:

### Table: `history`
Stores all scan results and verification logs.
```sql
CREATE TABLE history (
    id INTEGER PRIMARY KEY,
    timestamp TEXT,
    filename TEXT,
    hash TEXT,
    result TEXT
);
```

### Table: `snapshots`
Stores baseline snapshot metadata.
```sql
CREATE TABLE snapshots (
    id INTEGER PRIMARY KEY,
    timestamp TEXT,
    description TEXT,
    root_dir TEXT
);
```

### Table: `snapshot_entries`
Stores individual file hashes for each snapshot.
```sql
CREATE TABLE snapshot_entries (
    id INTEGER PRIMARY KEY,
    snapshot_id INTEGER,
    filepath TEXT,
    hash TEXT
);
```

---

## 🛠️ Technical Architecture

### Technology Stack
```
Language:     C (C99 standard)
GUI:          GTK 3.x
Database:     SQLite3
Cryptography: OpenSSL (EVP API)
Graphics:     Cairo
Threading:    GThread
Platform:     Windows (MSYS2/MinGW) + Linux
```

### Code Structure
- **Lines 1-200**: Includes, CSS themes, global state
- **Lines 200-350**: Database operations (init, CRUD)
- **Lines 350-550**: Cryptography & recursive scanning
- **Lines 550-900**: UI callbacks and event handlers
- **Lines 900-1400**: Page constructors (tabs)
- **Lines 1400+**: Main application setup and entry point

### Key Functions
- `compute_hash()` - Multi-algorithm file hashing
- `process_directory()` - Recursive directory traversal
- `db_create_snapshot()` - Baseline creation and storage
- `draw_global_stats_callback()` - Cairo pie chart rendering
- `on_theme_toggle()` - Dynamic CSS theme switching

---

## 🎓 Learning Outcomes

This project demonstrates mastery of:
- ✅ GTK+ event-driven GUI programming
- ✅ SQLite database design and integration
- ✅ OpenSSL cryptographic APIs
- ✅ Multi-threaded application architecture
- ✅ Configuration file management
- ✅ Professional UI/UX principles
- ✅ Cross-platform C development
- ✅ Git version control

Perfect for a **2nd-year Computer Science project** or **portfolio showcase**!

---

## 📝 Project Files

```
FileChecker/
├── FinalChecker.c              # Main application source (1600+ lines)
├── README.md                   # This file
├── .gitignore                  # Git ignore rules
├── integrity_checker.conf      # Auto-generated config (ignored)
├── integrity_history.db        # Auto-generated database (ignored)
└── *.exe                       # Compiled binary (ignored)
```

---

## 🤝 Contributing

Contributions welcome! Potential enhancements:
- Additional hash algorithms (SHA-3, BLAKE2)
- Network directory scanning
- Custom scheduling UI
- Plugin system for custom checks
- Windows/Linux installers
- File permission tracking
- Email notifications

---

## 🏆 Credits

**Built with:**
- C Programming Language
- GTK 3 GUI Framework
- SQLite3 Database
- OpenSSL Cryptography
- Cairo Graphics

**Version:** 2.0 Ultimate Edition  
**Features:** 18 Complete Professional Tools  
**Platform:** Windows (MSYS2) + Linux  

---

## 📞 Support

### Common Issues

**Problem**: `fatal error: gtk/gtk.h: No such file or directory`  
**Solution**: Install GTK3 development libraries (see Installation section)

**Problem**: Program won't start / missing DLLs  
**Solution**: Run from MSYS2 MinGW terminal, not Windows CMD

**Problem**: Can't compile - "command not found"  
**Solution**: Make sure you're in "MSYS2 MinGW 64-bit" terminal, not regular MSYS2

### Getting Help
- Check `About` dialog (ℹ️ button) for feature reference
- Hover over controls for tooltips
- Review this README

---

## 🎯 Use Cases

### For Students
- **Demonstrate**: Professional security tool development
- **Showcase**: Full-stack C programming skills
- **Impress**: Premium UI/UX in academic projects

### For Security Professionals
- **Monitor**: Critical system directories
- **Detect**: Unauthorized file modifications
- **Audit**: File integrity compliance
- **Report**: Professional HTML/CSV exports

### For System Administrators
- **Track**: Configuration file changes
- **Verify**: Software installations
- **Baseline**: System state snapshots
- **Alert**: Real-time watchdog monitoring

---

**Perfect for demonstrating professional security tooling capabilities!** 🚀

---

## ⭐ Star this project if you find it useful!

**Happy file monitoring!** 🔐✨
